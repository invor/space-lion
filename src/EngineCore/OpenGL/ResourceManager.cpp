#include "ResourceManager.hpp"

#include "../ResourceLoading.hpp"
//#include "GraphicsBackend.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        namespace OpenGL
        {
            void ResourceManager::clearAllResources()
            {
                m_name_to_shader_program_idx.clear();
                m_name_to_mesh_idx.clear();
                m_name_to_textures_2d_idx.clear();
                m_name_to_textureArray_idx.clear();
                m_name_to_textureCubemapArray_idx.clear();
                m_name_to_textures_3d_idx.clear();
                m_name_to_FBO_idx.clear();
                m_name_to_buffer_idx.clear();

                m_id_to_shader_program_idx.clear();
                m_id_to_mesh_idx.clear();
                m_id_to_textures_2d_idx.clear();
                m_id_to_textureArray_idx.clear();
                m_id_to_textureCubemapArray_idx.clear();
                m_id_to_textures_3d_idx.clear();
                m_id_to_FBO_idx.clear();
                m_id_to_buffer_idx.clear();

                m_shader_programs.clear();
                m_meshes.clear();
                m_textures_2d.clear();
                m_textureArrays.clear();
                m_textureCubemapArrays.clear();
                m_textures_3d.clear();
                m_FBOs.clear();
                m_buffers.clear();

                m_resource_cnt = 0;
            }

            ResourceID ResourceManager::allocateMeshAsync(
                std::string const & name,
                size_t vertex_cnt,
                size_t index_cnt,
                std::shared_ptr<GenericVertexLayout> const & vertex_layout,
                GLenum const index_type,
                GLenum const mesh_type)
            {
                size_t idx = m_meshes.size();
                ResourceID rsrc_id = generateResourceID();

                {
                    std::unique_lock<std::shared_mutex> lock(m_meshes_mutex);
                    m_meshes.push_back(Resource<glowl::Mesh>(rsrc_id));
                    m_id_to_mesh_idx.insert(std::pair<unsigned int, size_t>(m_meshes.back().id.value(), idx));
                }

                m_renderThread_tasks.push([this, idx, vertex_cnt, index_cnt, vertex_layout, index_type, mesh_type]() {

                    std::shared_lock<std::shared_mutex> lock(m_meshes_mutex);

                    VertexLayout ogl_vertex_layout;
                    ogl_vertex_layout.byte_size = vertex_layout->byte_size;
                    for (auto& attr : vertex_layout->attributes) {
                        //VertexLayout::Attribute ogl_attr(attr.size,attr.type,attr.normalized,attr.offset);
                        ogl_vertex_layout.attributes.push_back({ attr.size,attr.type,attr.normalized, static_cast<GLsizei>(attr.offset) });
                    }

                    // TODO get number of buffer required for vertex layout and compute byte sizes
                    std::vector<void*> vertex_data_ptrs(ogl_vertex_layout.attributes.size(), nullptr);
                    std::vector<size_t> vertex_data_buffer_byte_sizes(ogl_vertex_layout.attributes.size());

                    for (size_t attrib_idx = 0; attrib_idx < ogl_vertex_layout.attributes.size(); ++attrib_idx) {
                        vertex_data_buffer_byte_sizes[attrib_idx] = computeAttributeByteSize(ogl_vertex_layout.attributes[attrib_idx]) * vertex_cnt;
                    }

                    size_t index_data_byte_size = 4 * index_cnt; //TODO support different index formats

                    this->m_meshes[idx].resource = std::make_unique<glowl::Mesh>(
                        vertex_data_ptrs, //TODO THIS CALLS THE WRONG CONSTRUCTOR
                        vertex_data_buffer_byte_sizes,
                        nullptr,
                        index_data_byte_size,
                        ogl_vertex_layout,
                        index_type,
                        GL_DYNAMIC_DRAW,
                        mesh_type);

                    this->m_meshes[idx].state = READY;
                });

                return m_meshes.back().id;
            }

            WeakResource<glowl::GLSLProgram> ResourceManager::createShaderProgram(
                std::string const& program_name,
                std::vector<ShaderFilename> const& shader_filenames,
                std::string const& additional_cs_defines)
            {
                {
                    std::shared_lock<std::shared_mutex> lock(m_shader_programs_mutex);
                    auto search = m_name_to_shader_program_idx.find(program_name);
                    if (search != m_name_to_shader_program_idx.end())
                        return WeakResource<glowl::GLSLProgram>(
                            m_shader_programs[search->second].id,
                            m_shader_programs[search->second].resource.get(),
                            m_shader_programs[search->second].state);
                }

                size_t idx = m_shader_programs.size();
                ResourceID rsrc_id = generateResourceID();

                std::unique_lock<std::shared_mutex> lock(m_shader_programs_mutex);
                m_shader_programs.push_back(Resource<glowl::GLSLProgram>(rsrc_id));
                m_id_to_shader_program_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_shader_program_idx.insert(std::pair<std::string, size_t>(program_name, idx));

                m_shader_programs[idx].resource = std::make_unique<glowl::GLSLProgram>();
                m_shader_programs[idx].resource->setId(program_name);
                m_shader_programs[idx].resource->init();

                std::string vertex_src;
                std::string tessellationControl_src;
                std::string tessellationEvaluation_src;
                std::string geometry_src;
                std::string fragment_src;
                std::string compute_src;

                size_t cs_define_insertion;

                for (auto& shader_filename : shader_filenames)
                {
                    auto shader_src = ResourceLoading::readShaderFile(shader_filename.first.c_str());

                    switch (shader_filename.second)
                    {
                    case glowl::GLSLProgram::VertexShader:
                        vertex_src = shader_src;

                        {
                            //TODO Scan vertex shader for input parameters
                            unsigned int param_idx = 0;
                            std::string line;
                            std::istringstream tokenStream(shader_src);
                            while (std::getline(tokenStream, line, '\n'))
                            {
                                std::stringstream ss(line);
                                std::string token;

                                ss >> token;

                                if (std::strcmp("in", token.c_str()) == 0)
                                {
                                    ss >> token; // this should be the data type
                                    ss >> token; // this should be the variable name

                                    token.erase(token.end() - 1);
                                    m_shader_programs[idx].resource->bindAttribLocation(param_idx++, token.c_str());

                                    //std::cout<<"Input parameter name: "<<buffer<<std::endl;
                                }
                            }
                        }

                        break;
                    case glowl::GLSLProgram::FragmentShader:
                        fragment_src = shader_src;

                        {
                            // And scan fragment shader for output parameters
                            unsigned int param_idx = 0;
                            std::string line;
                            std::istringstream tokenStream(shader_src);
                            while (std::getline(tokenStream, line, '\n'))
                            {
                                std::stringstream ss(line);
                                std::string token;

                                ss >> token;

                                if (std::strcmp("in", token.c_str()) == 0)
                                {
                                    ss >> token; // this should be the data type
                                    ss >> token; // this should be the variable name

                                    token.erase(token.end() - 1);
                                    m_shader_programs[idx].resource->bindFragDataLocation(param_idx++, token.c_str());

                                    //std::cout<<"Input parameter name: "<<buffer<<std::endl;
                                }
                            }
                        }

                        break;
                    case glowl::GLSLProgram::GeometryShader:
                        geometry_src = shader_src;
                        break;
                    case glowl::GLSLProgram::TessellationControl:
                        tessellationControl_src = shader_src;
                        break;
                    case glowl::GLSLProgram::TessellationEvaluation:
                        tessellationEvaluation_src = shader_src;
                        break;
                    case glowl::GLSLProgram::ComputeShader:
                        compute_src = shader_src;

                        cs_define_insertion = compute_src.find("#version"); // find beginning of shader, i.e. version statement
                        cs_define_insertion = compute_src.find("\n", cs_define_insertion); // go to end of that line
                        cs_define_insertion += 2; // and move to the next line
                        compute_src.insert(cs_define_insertion, additional_cs_defines);

                        break;
                    default:
                        break;
                    }
                }

                bool prgm_error = false;

                if (!vertex_src.empty())
                    prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&vertex_src, GL_VERTEX_SHADER);
                if (!fragment_src.empty())
                    prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&fragment_src, GL_FRAGMENT_SHADER);
                if (!geometry_src.empty())
                    prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&geometry_src, GL_GEOMETRY_SHADER);
                if (!tessellationControl_src.empty())
                    prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&tessellationControl_src, GL_TESS_CONTROL_SHADER);
                if (!tessellationEvaluation_src.empty())
                    prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&tessellationEvaluation_src, GL_TESS_EVALUATION_SHADER);
                /*
                *	THIS SEEM TO BE OUTDATED IF IT WAS EVER TRUE
                *	A non-empty compute source string should only happen if all other sources are empty strings.
                *	I won't check for this though, assuming that nobody - i.e. me - fucked up a compute shader case above.
                */
                if (!compute_src.empty())
                    prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&compute_src, GL_COMPUTE_SHADER);


                prgm_error |= !m_shader_programs[idx].resource->link();

                if (prgm_error)
                {
                    std::cout << "Error during shader program creation of \"" << m_shader_programs[idx].resource->getId() << "\"" << std::endl;
                    std::cout << m_shader_programs[idx].resource->getLog();
                }

                m_shader_programs[idx].state = READY;

                return WeakResource<glowl::GLSLProgram>(m_shader_programs.back().id, m_shader_programs.back().resource.get(), m_shader_programs.back().state);
            }

            ResourceID ResourceManager::createShaderProgramAsync(
                std::string const& program_name,
                std::shared_ptr<std::vector<ShaderFilename>> const& shader_filenames,
                std::string const& additional_cs_defines)
            {
                // check if program of same name already exits
                {
                    std::shared_lock<std::shared_mutex> prgm_lock(m_shader_programs_mutex);
                    auto search = m_name_to_shader_program_idx.find(program_name);
                    if (search != m_name_to_shader_program_idx.end())
                        return m_shader_programs[search->second].id;
                }

                size_t idx = m_shader_programs.size();
                ResourceID rsrc_id = generateResourceID();

                {
                    std::unique_lock<std::shared_mutex> lock(m_shader_programs_mutex);
                    m_shader_programs.push_back(Resource<glowl::GLSLProgram>(rsrc_id));
                    m_name_to_shader_program_idx.insert(std::pair<std::string, size_t>(program_name, idx));
                    m_id_to_shader_program_idx.insert(std::pair<uint, size_t>(m_shader_programs.back().id.value(), idx));
                }

                m_renderThread_tasks.push([this, idx, program_name, shader_filenames, additional_cs_defines]() {
                    //m_renderThread_tasks.push([this,idx,name,paths]() {
                    m_shader_programs[idx].resource = std::make_unique<glowl::GLSLProgram>();
                    m_shader_programs[idx].state = READY;
                    m_shader_programs[idx].resource->setId(program_name);
                    m_shader_programs[idx].resource->init();

                    m_shader_programs[idx].resource = std::make_unique<glowl::GLSLProgram>();
                    m_shader_programs[idx].resource->setId(program_name);
                    m_shader_programs[idx].resource->init();

                    std::string vertex_src;
                    std::string tessellationControl_src;
                    std::string tessellationEvaluation_src;
                    std::string geometry_src;
                    std::string fragment_src;
                    std::string compute_src;

                    size_t cs_define_insertion;

                    for (auto& shader_filename : *shader_filenames)
                    {
                        auto shader_src = ResourceLoading::readShaderFile(shader_filename.first.c_str());

                        switch (shader_filename.second)
                        {
                        case glowl::GLSLProgram::VertexShader:
                            vertex_src = shader_src;

                            {
                                //TODO Scan vertex shader for input parameters
                                unsigned int param_idx = 0;
                                std::string line;
                                std::istringstream tokenStream(shader_src);
                                while (std::getline(tokenStream, line, '\n'))
                                {
                                    std::stringstream ss(line);
                                    std::string token;

                                    ss >> token;

                                    if (std::strcmp("in", token.c_str()) == 0)
                                    {
                                        ss >> token; // this should be the data type
                                        ss >> token; // this should be the variable name

                                        token.erase(token.end() - 1);
                                        m_shader_programs[idx].resource->bindAttribLocation(param_idx++, token.c_str());

                                        //std::cout<<"Input parameter name: "<<buffer<<std::endl;
                                    }
                                }
                            }

                            break;
                        case glowl::GLSLProgram::FragmentShader:
                            fragment_src = shader_src;

                            {
                                // And scan fragment shader for output parameters
                                unsigned int param_idx = 0;
                                std::string line;
                                std::istringstream tokenStream(shader_src);
                                while (std::getline(tokenStream, line, '\n'))
                                {
                                    std::stringstream ss(line);
                                    std::string token;

                                    ss >> token;

                                    if (std::strcmp("in", token.c_str()) == 0)
                                    {
                                        ss >> token; // this should be the data type
                                        ss >> token; // this should be the variable name

                                        token.erase(token.end() - 1);
                                        m_shader_programs[idx].resource->bindFragDataLocation(param_idx++, token.c_str());

                                        //std::cout<<"Input parameter name: "<<buffer<<std::endl;
                                    }
                                }
                            }

                            break;
                        case glowl::GLSLProgram::GeometryShader:
                            geometry_src = shader_src;
                            break;
                        case glowl::GLSLProgram::TessellationControl:
                            tessellationControl_src = shader_src;
                            break;
                        case glowl::GLSLProgram::TessellationEvaluation:
                            tessellationEvaluation_src = shader_src;
                            break;
                        case glowl::GLSLProgram::ComputeShader:
                            compute_src = shader_src;

                            cs_define_insertion = compute_src.find("#version"); // find beginning of shader, i.e. version statement
                            cs_define_insertion = compute_src.find("\n", cs_define_insertion); // go to end of that line
                            cs_define_insertion += 2; // and move to the next line
                            compute_src.insert(cs_define_insertion, additional_cs_defines);

                            break;
                        default:
                            break;
                        }
                    }

                    bool prgm_error = false;

                    if (!vertex_src.empty())
                        prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&vertex_src, GL_VERTEX_SHADER);
                    if (!fragment_src.empty())
                        prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&fragment_src, GL_FRAGMENT_SHADER);
                    if (!geometry_src.empty())
                        prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&geometry_src, GL_GEOMETRY_SHADER);
                    if (!tessellationControl_src.empty())
                        prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&tessellationControl_src, GL_TESS_CONTROL_SHADER);
                    if (!tessellationEvaluation_src.empty())
                        prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&tessellationEvaluation_src, GL_TESS_EVALUATION_SHADER);
                    /*
                    *	THIS SEEM TO BE OUTDATED IF IT WAS EVER TRUE
                    *	A non-empty compute source string should only happen if all other sources are empty strings.
                    *	I won't check for this though, assuming that nobody - i.e. me - fucked up a compute shader case above.
                    */
                    if (!compute_src.empty())
                        prgm_error |= !m_shader_programs[idx].resource->compileShaderFromString(&compute_src, GL_COMPUTE_SHADER);


                    prgm_error |= !m_shader_programs[idx].resource->link();

                    if (prgm_error)
                    {
                        std::cout << "Error during shader program creation of \"" << m_shader_programs[idx].resource->getId() << "\"" << std::endl;
                        std::cout << m_shader_programs[idx].resource->getLog();
                    }

                    m_shader_programs[idx].state = READY;
                });

                return m_shader_programs.back().id;
            }


            WeakResource<glowl::Texture2D> ResourceManager::createTexture2D(
                const std::string& name,
                glowl::TextureLayout const& layout,
                GLvoid * data,
                bool generateMipmap)
            {
                {
                    std::shared_lock<std::shared_mutex> tex_lock(m_textures_2d_mutex);
                    auto search = m_name_to_textures_2d_idx.find(name);
                    if (search != m_name_to_textures_2d_idx.end())
                        return WeakResource<glowl::Texture2D>(
                            m_textures_2d[search->second].id,
                            m_textures_2d[search->second].resource.get(),
                            m_textures_2d[search->second].state);
                }

                size_t idx = m_textures_2d.size();
                ResourceID rsrc_id = generateResourceID();

                std::unique_lock<std::shared_mutex> lock(m_textures_2d_mutex);
                m_textures_2d.push_back(Resource<glowl::Texture2D>(rsrc_id));
                m_id_to_textures_2d_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_textures_2d_idx.insert(std::pair<std::string, size_t>(name, idx));

                m_textures_2d[idx].resource = std::make_unique<glowl::Texture2D>(name, layout, data, generateMipmap);
                m_textures_2d[idx].state = READY;

                return WeakResource<glowl::Texture2D>(
                    m_textures_2d[idx].id,
                    m_textures_2d[idx].resource.get(),
                    m_textures_2d[idx].state);
            }

            ResourceID ResourceManager::createTexture2DAsync(
                const std::string& name,
                glowl::TextureLayout const& layout,
                GLvoid * data,
                bool generateMipmap)
            {
                {
                    std::shared_lock<std::shared_mutex> tex_lock(m_textures_2d_mutex);
                    auto search = m_name_to_textures_2d_idx.find(name);
                    if (search != m_name_to_textures_2d_idx.end())
                        return m_textures_2d[search->second].id;
                }

                size_t idx = m_textures_2d.size();
                ResourceID rsrc_id = generateResourceID();

                std::unique_lock<std::shared_mutex> lock(m_textures_2d_mutex);
                m_textures_2d.push_back(Resource<glowl::Texture2D>(rsrc_id));
                m_id_to_textures_2d_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_textures_2d_idx.insert(std::pair<std::string, size_t>(name, idx));

                m_renderThread_tasks.push([this, idx, name, layout, data, generateMipmap]() {
                    std::unique_lock<std::shared_mutex> tex_lock(m_textures_2d_mutex);

                    m_textures_2d[idx].resource = std::make_unique<glowl::Texture2D>(name, layout, data, generateMipmap);
                    m_textures_2d[idx].state = READY;
                });

                return m_textures_2d[idx].id;
            }

            WeakResource<glowl::Texture2DArray> ResourceManager::createTexture2DArray(
                std::string const& name,
                glowl::TextureLayout const& layout,
                GLvoid * data,
                bool generateMipmap)
            {
                {
                    std::shared_lock<std::shared_mutex> tex_lock(m_texArr_mutex);
                    auto search = m_name_to_textureArray_idx.find(name);
                    if (search != m_name_to_textureArray_idx.end())
                        return WeakResource<glowl::Texture2DArray>(
                            m_textureArrays[search->second].id,
                            m_textureArrays[search->second].resource.get(),
                            m_textureArrays[search->second].state);
                }

                size_t idx = m_textureArrays.size();
                ResourceID rsrc_id = generateResourceID();

                std::unique_lock<std::shared_mutex> lock(m_texArr_mutex);
                m_textureArrays.push_back(Resource<glowl::Texture2DArray>(rsrc_id));
                m_id_to_textureArray_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_textureArray_idx.insert(std::pair<std::string, size_t>(name, idx));

                m_textureArrays[idx].resource = std::make_unique<glowl::Texture2DArray>(name, layout, data, generateMipmap);
                m_textureArrays[idx].state = READY;

                return WeakResource<glowl::Texture2DArray>(
                    m_textureArrays[idx].id,
                    m_textureArrays[idx].resource.get(),
                    m_textureArrays[idx].state);
            }

            ResourceID ResourceManager::createTexture2DArrayAsync(
                const std::string& name,
                const glowl::TextureLayout & layout,
                GLvoid * data,
                bool generateMipmap)
            {
                {
                    std::shared_lock<std::shared_mutex> tex_lock(m_texArr_mutex);
                    auto search = m_name_to_textureArray_idx.find(name);
                    if (search != m_name_to_textureArray_idx.end())
                        return m_textureArrays[search->second].id;
                }

                size_t idx = m_textureArrays.size();
                ResourceID rsrc_id = generateResourceID();

                std::unique_lock<std::shared_mutex> lock(m_texArr_mutex);
                m_textureArrays.push_back(Resource<glowl::Texture2DArray>(rsrc_id));
                m_id_to_textureArray_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_textureArray_idx.insert(std::pair<std::string, size_t>(name, idx));

                m_renderThread_tasks.push([this, idx, name, layout, data, generateMipmap]() {
                    std::unique_lock<std::shared_mutex> tex_lock(m_texArr_mutex);

                    m_textureArrays[idx].resource = std::make_unique<glowl::Texture2DArray>(name, layout, data, generateMipmap);
                    m_textureArrays[idx].state = READY;
                });

                return m_textureArrays[idx].id;
            }

            WeakResource<glowl::Texture3D> ResourceManager::createTexture3D(
                const std::string name,
                glowl::TextureLayout const& layout,
                GLvoid* data)
            {
                {
                    std::shared_lock<std::shared_mutex> tex_lock(m_textures_3d_mutex);
                    auto search = m_name_to_textures_3d_idx.find(name);
                    if (search != m_name_to_textures_3d_idx.end())
                        return WeakResource<glowl::Texture3D>(
                            m_textures_3d[search->second].id,
                            m_textures_3d[search->second].resource.get(),
                            m_textures_3d[search->second].state);
                }

                size_t idx = m_textures_3d.size();
                ResourceID rsrc_id = generateResourceID();

                std::unique_lock<std::shared_mutex> lock(m_textures_3d_mutex);
                m_textures_3d.push_back(Resource<glowl::Texture3D>(rsrc_id));
                m_id_to_textures_3d_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_textures_3d_idx.insert(std::pair<std::string, size_t>(name, idx));

                m_textures_3d[idx].resource = std::make_unique<glowl::Texture3D>(name, layout, data);
                m_textures_3d[idx].state = READY;

                return WeakResource<glowl::Texture3D>(
                    m_textures_3d[idx].id,
                    m_textures_3d[idx].resource.get(),
                    m_textures_3d[idx].state);
            }

            WeakResource<glowl::FramebufferObject> ResourceManager::createFramebufferObject(
                std::string const& name,
                uint width,
                uint height,
                bool has_depth,
                bool has_stencil)
            {
                {
                    std::shared_lock<std::shared_mutex> tex_lock(m_fbo_mutex);
                    auto search = m_name_to_FBO_idx.find(name);
                    if (search != m_name_to_FBO_idx.end())
                        return WeakResource<glowl::FramebufferObject>(
                            m_FBOs[search->second].id,
                            m_FBOs[search->second].resource.get(),
                            m_FBOs[search->second].state
                            );
                }

                size_t idx = m_FBOs.size();
                ResourceID rsrc_id = generateResourceID();

                std::unique_lock<std::shared_mutex> lock(m_fbo_mutex);
                m_FBOs.push_back(Resource<glowl::FramebufferObject>(rsrc_id));
                m_id_to_FBO_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_FBO_idx.insert(std::pair<std::string, size_t>(name, idx));

                m_FBOs[idx].resource = std::make_unique<glowl::FramebufferObject>(name,width, height, has_depth, has_stencil);
                m_FBOs[idx].state = READY;

                return WeakResource<glowl::FramebufferObject>(
                    m_FBOs[idx].id,
                    m_FBOs[idx].resource.get(),
                    m_FBOs[idx].state);
            }

            WeakResource<glowl::BufferObject> ResourceManager::createBufferObject(
                std::string const& name,
                GLenum target,
                GLvoid const* data,
                GLsizeiptr byte_size,
                GLenum usage)
            {

                {
                    std::shared_lock<std::shared_mutex> tex_lock(m_buffers_mutex);
                    auto search = m_name_to_buffer_idx.find(name);
                    if (search != m_name_to_buffer_idx.end())
                        return WeakResource<glowl::BufferObject>(
                            m_buffers[search->second].id,
                            m_buffers[search->second].resource.get(),
                            m_buffers[search->second].state
                            );
                }

                size_t idx = m_buffers.size();
                ResourceID rsrc_id = generateResourceID();

                std::unique_lock<std::shared_mutex> lock(m_buffers_mutex);
                m_buffers.push_back(Resource<glowl::BufferObject>(rsrc_id));
                m_id_to_buffer_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_buffer_idx.insert(std::pair<std::string, size_t>(name, idx));

                m_buffers[idx].resource = std::make_unique<glowl::BufferObject>(target, data, byte_size, usage);
                m_buffers[idx].state = READY;

                return WeakResource<glowl::BufferObject>(
                    m_buffers[idx].id,
                    m_buffers[idx].resource.get(),
                    m_buffers[idx].state);
            }


            void ResourceManager::updateBufferObject(
                ResourceID id,
                GLvoid const* data,
                GLsizeiptr byte_size)
            {
                std::shared_lock<std::shared_mutex> bufferObject_lock(m_buffers_mutex);

                auto search = m_id_to_buffer_idx.find(id.value());

                if (search != m_id_to_buffer_idx.end())
                {
                    updateBufferObject(search->second, data, byte_size);
                }
            }

            void ResourceManager::updateBufferObject(
                std::string const& name,
                GLvoid const* data,
                GLsizeiptr byte_size)
            {
                std::shared_lock<std::shared_mutex> bufferObject_lock(m_buffers_mutex);

                auto search = m_name_to_buffer_idx.find(name);

                if (search != m_name_to_buffer_idx.end())
                {
                    updateBufferObject(search->second, data, byte_size);
                }
            }

            void ResourceManager::updateBufferObject(
                size_t idx,
                GLvoid const* data,
                GLsizeiptr byte_size)
            {
                m_buffers[idx].resource->bufferSubData(data, byte_size);
            }

            WeakResource<glowl::Texture2DArray> ResourceManager::getTexture2DArray(ResourceID id) const
            {
                std::shared_lock<std::shared_mutex> texArr_lock(m_texArr_mutex);

                auto search = m_id_to_textureArray_idx.find(id.value());

                WeakResource<glowl::Texture2DArray> retval(id, nullptr, NOT_READY);

                if (search != m_id_to_textureArray_idx.end())
                {
                    retval.id = m_textureArrays[search->second].id;
                    retval.resource = m_textureArrays[search->second].resource.get();
                    retval.state = m_textureArrays[search->second].state;
                }

                return retval;
            }

            WeakResource<glowl::FramebufferObject> ResourceManager::getFramebufferObject(std::string const& name) const
            {
                std::shared_lock<std::shared_mutex> fbo_lock(m_fbo_mutex);

                auto search = m_name_to_FBO_idx.find(name);

                WeakResource<glowl::FramebufferObject> retval(invalidResourceID(), nullptr, NOT_READY);

                if (search != m_name_to_FBO_idx.end())
                {
                    retval.id = m_FBOs[search->second].id;
                    retval.resource = m_FBOs[search->second].resource.get();
                    retval.state = m_FBOs[search->second].state;
                }

                return retval;
            }

        }
    }
}