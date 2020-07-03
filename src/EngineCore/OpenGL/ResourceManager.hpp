/**
* \class ResourceManager
*
* \brief Graphic resource management.
*
* This class creates, stores and manages resources (mostly graphic resources e.g. textures, meshes, shader programs).
* Each resource is stored by a unique pointer to the resource object and (for now) only raw pointers are
* used to access the resources from outside.
*
* \author Michael Becher
*/

#ifndef ResourceManager_hpp
#define ResourceManager_hpp

#define _CRT_SECURE_NO_DEPRECATE

/*	std includes */
#include <list>
#include <fstream>
#include <sstream>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <shared_mutex>

#include <glowl/glowl.h>

/*	Include space-lion headers */
#include "../BaseResourceManager.hpp"
#include "../EntityManager.hpp"
#include "../GenericTextureLayout.hpp"
#include "../types.hpp"

#define DEBUG_OUTPUT 0

namespace EngineCore
{
    namespace Graphics
    {
        namespace OpenGL
        {

            class ResourceManager : public BaseResourceManager<
                glowl::BufferObject,
                glowl::Mesh,
                glowl::GLSLProgram,
                glowl::Texture2D,
                glowl::Texture3D>
            {
            public:
                typedef glowl::TextureLayout TextureLayout;
                typedef glowl::VertexLayout  VertexLayout;
                typedef GLenum               IndexFormatType;
                typedef GLenum               PrimitiveTopologyType;

                ResourceManager() : BaseResourceManager() {}
                ResourceManager(ResourceManager const & cpy) = delete;
                ~ResourceManager() = default;

                /** Returns log string */
                std::string const& getLog() { return m_resourcelog; }

                /** Clear lists containing graphics resources */
                void clearAllResources();

                size_t computeVertexByteSize(VertexLayout const& vertex_layout)
                {
                    size_t retval = 0;

                    for (auto& attr : vertex_layout.attributes) {
                        //VertexLayout::Attribute ogl_attr(attr.size,attr.type,attr.normalized,attr.offset);
                        retval += glowl::computeAttributeByteSize({ attr.size,attr.type,attr.normalized, static_cast<GLsizei>(attr.offset) });
                    }

                    return retval;
                }

                size_t computeIndexByteSize(GLenum index_type)
                {
                    return glowl::computeByteSize(index_type);
                }

                constexpr IndexFormatType convertGenericIndexType(uint32_t index_type)
                {
                    IndexFormatType retval = index_type; // Assuming the "generic" index type to use GL values due to glTF usage
                    return retval;
                }

                constexpr PrimitiveTopologyType convertGenericPrimitiveTopology(uint32_t primitive_type)
                {
                    PrimitiveTopologyType retval = primitive_type; // Assuming the "generic" primitive type to use GL values due to glTF usage
                    return retval;
                }

                /**
                 * Assuming the "generic" vertex layout to use GL values due to glTF usage
                 */
                VertexLayout convertGenericGltfVertexLayout(GenericVertexLayout vertex_layout)
                {
                    VertexLayout retval;

                    for (auto stride : vertex_layout.strides)
                    {
                        retval.strides.emplace_back(stride);
                    }

                    for (auto& attrib : vertex_layout.attributes)
                    {
                        retval.attributes.emplace_back(VertexLayout::Attribute(attrib.size,attrib.type,attrib.normalized,attrib.offset));
                    }

                    return retval;
                }

                TextureLayout convertGenericTextureLayout(GenericTextureLayout texture_layout)
                {
                    TextureLayout retval;

                    retval.internal_format = texture_layout.internal_format;
                    retval.width = texture_layout.width;
                    retval.height = texture_layout.height;
                    retval.depth = texture_layout.depth;
                    retval.format = texture_layout.format;
                    retval.type = texture_layout.type;

                    retval.levels = texture_layout.levels;

                    for (auto& param : texture_layout.int_parameters)
                    {
                        retval.int_parameters.push_back({ param.first,param.second });
                    }

                    for (auto& param : texture_layout.float_parameters)
                    {
                        retval.float_parameters.push_back({ param.first,param.second });
                    }

                    return retval;
                }

#pragma region Create mesh
                //template<
                //	typename VertexContainer,
                //	typename IndexContainer>
                //ResourceID createMeshAsync(
                //	std::string const& name,
                //	std::shared_ptr<std::vector<VertexContainer>> const& vertex_data,
                //	std::shared_ptr<IndexContainer> const& index_data,
                //	std::shared_ptr<VertexDescriptor> const& vertex_layout,
                //	D3D_PRIMITIVE_TOPOLOGY const mesh_type);

                ResourceID allocateMeshAsync(
                    std::string const& name,
                    size_t vertex_cnt,
                    size_t index_cnt,
                    std::shared_ptr<VertexLayout> const& vertex_layout,
                    GLenum const index_type,
                    GLenum const mesh_type);

                template<
                    typename VertexContainer,
                    typename IndexContainer>
                    void updateMesh(
                        ResourceID rsrc_id,
                        size_t vertex_offset,
                        size_t index_offset,
                        std::vector<VertexContainer> const& vertex_data,
                        IndexContainer const& index_data);

                template<
                    typename VertexContainer,
                    typename IndexContainer>
                    void updateMeshAsync(
                        ResourceID rsrc_id,
                        size_t vertex_offset,
                        size_t index_offset,
                        std::shared_ptr<std::vector<VertexContainer>> const& vertex_data,
                        std::shared_ptr<IndexContainer> const& index_data);

#pragma endregion

#pragma region Create shader program
                typedef std::pair<std::string, glowl::GLSLProgram::ShaderType> ShaderFilename;

                /**
                 * Creates a GLSLprogram object
                 * \param paths Gives the paths to all shader files.
                 * \return Returns shared pointer to GLSL shader program.
                 */
                WeakResource<glowl::GLSLProgram> createShaderProgram(
                    std::string const& program_name,
                    std::vector<ShaderFilename> const& shader_filenames,
                    std::string const& additional_cs_defines = "");

                ResourceID createShaderProgramAsync(
                    std::string const& program_name,
                    std::shared_ptr<std::vector<ShaderFilename>> const& shader_filenames,
                    std::string const& additional_cs_defines = "");
#pragma endregion

#pragma region Create 2D textures
                /**
                 * \brief Creates a 2D texture from a given data array
                 * \param id Identifier for the texture
                 * \param layout Texture format and size
                 * \param data Pointer to the actual texture data.
                 * \param generateMipmap Specifies whether a mipmap should be generated for the texture
                 * \return Returns a pointer to new texture or existing texture if given id already exits
                 */
                WeakResource<glowl::Texture2D> createTexture2D(
                    std::string const&   name,
                    glowl::TextureLayout const& layout,
                    GLvoid*              data,
                    bool                 generateMipmap = false);

                ResourceID createTexture2DAsync(
                    std::string const&   name,
                    glowl::TextureLayout const& layout,
                    GLvoid*              data,
                    bool                 generateMipmap = false
                );

                template<typename TexelDataContainer>
                ResourceID createTexture2DAsync(
                    std::string const& name,
                    glowl::TextureLayout const& layout,
                    std::shared_ptr<TexelDataContainer> const& data,
                    bool generateMipmap = false
                );

                WeakResource<glowl::Texture2DArray> createTexture2DArray(
                    std::string const& name,
                    const glowl::TextureLayout & layout,
                    GLvoid * data,
                    bool generateMipmap = false);

                ResourceID createTexture2DArrayAsync(
                    std::string const& name,
                    const glowl::TextureLayout & layout,
                    GLvoid * data,
                    bool generateMipmap = false);
#pragma endregion

                /**
                 * \brief Create a 3D texture from a given data array
                 * \param id Identifier for the texture
                 * \param layout Texture format and size
                 * \param data Pointer to the actual texture data
                 * \return Returns a pointer to new texture or existing texture if given id already exits
                 */
                WeakResource<glowl::Texture3D> createTexture3D(
                    const std::string name,
                    glowl::TextureLayout const& layout,
                    GLvoid* data);

                WeakResource<glowl::FramebufferObject> createFramebufferObject(
                    std::string const& name,
                    uint width,
                    uint height,
                    bool has_depth = false,
                    bool has_stencil = false);

                template<typename Container>
                WeakResource<glowl::BufferObject> createBufferObject(
                    std::string const& name,
                    GLenum target,
                    Container const& datastorage,
                    GLenum usage = GL_DYNAMIC_DRAW);

                WeakResource<glowl::BufferObject> createBufferObject(
                    std::string const& name,
                    GLenum target,
                    GLvoid const* data,
                    GLsizeiptr byte_size,
                    GLenum usage = GL_DYNAMIC_DRAW);

                template<typename Container>
                void updateBufferObject(ResourceID id, Container const& datastorage)
                {
                    updateBufferObject(id, datastorage.data(), static_cast<GLsizeiptr>(datastorage.size() * sizeof(Container::value_type)));
                }

                template<typename Container>
                void updateBufferObject(std::string const& name, Container const& datastorage)
                {
                    updateBufferObject(name, datastorage.data(), static_cast<GLsizeiptr>(datastorage.size() * sizeof(Container::value_type)));
                }

                void updateBufferObject(
                    ResourceID id,
                    GLvoid const* data,
                    GLsizeiptr byte_size
                );

                void updateBufferObject(
                    std::string const& name,
                    GLvoid const* data,
                    GLsizeiptr byte_size
                );

                WeakResource<glowl::Texture2DArray> getTexture2DArray(ResourceID id) const;
                WeakResource<glowl::FramebufferObject> getFramebufferObject(std::string const& name) const;

            private:

                void updateBufferObject(
                    size_t idx,
                    GLvoid const* data,
                    GLsizeiptr byte_size
                );

            private:
                /** Log string */
                std::string m_resourcelog;

                /*
                 * Additional resource collections for resource types currently not included in base class
                 */
                std::vector<Resource<glowl::Texture2DArray>>      m_textureArrays;
                std::vector<Resource<glowl::TextureCubemapArray>> m_textureCubemapArrays;
                std::vector<Resource<glowl::FramebufferObject>>   m_FBOs;

                std::unordered_map<std::string, size_t> m_name_to_textureArray_idx;
                std::unordered_map<std::string, size_t> m_name_to_textureCubemapArray_idx;
                std::unordered_map<std::string, size_t> m_name_to_FBO_idx;

                std::unordered_map<uint, size_t> m_id_to_textureArray_idx;
                std::unordered_map<uint, size_t> m_id_to_textureCubemapArray_idx;
                std::unordered_map<uint, size_t> m_id_to_FBO_idx;

                mutable std::shared_mutex m_texArr_mutex;
                mutable std::shared_mutex m_texCubeArr_mutex;
                mutable std::shared_mutex m_fbo_mutex;
            };

            template<typename VertexContainer, typename IndexContainer>
            inline void ResourceManager::updateMesh(
                ResourceID rsrc_id,
                size_t vertex_offset,
                size_t index_offset,
                std::vector<VertexContainer> const & vertex_data,
                IndexContainer const & index_data)
            {
                std::unique_lock<std::shared_mutex> lock(m_meshes_mutex);

                auto query = m_id_to_mesh_idx.find(rsrc_id.value());

                if (query != m_id_to_mesh_idx.end())
                {
                    VertexLayout vertex_layout = m_meshes[query->second].resource->getVertexLayout();

                    //TODO some sanity checks, such as attrib cnt and mesh size?

                    for (int attrib_idx = 0; attrib_idx < vertex_layout.attributes.size(); ++attrib_idx)
                    {
                        size_t attrib_byte_size = computeAttributeByteSize(vertex_layout.attributes[attrib_idx]);
                        size_t vertex_buffer_byte_offset = attrib_byte_size * vertex_offset;
                        m_meshes[query->second].resource->bufferVertexSubData(attrib_idx, vertex_data[attrib_idx], vertex_buffer_byte_offset);
                    }

                    size_t index_type_byte_size = computeByteSize(m_meshes[query->second].resource->getIndexFormat());
                    size_t index_byte_offset = index_offset * index_type_byte_size;
                    m_meshes[query->second].resource->bufferIndexSubData(index_data, index_byte_offset);
                }
            }

            template<typename VertexContainer, typename IndexContainer>
            inline void ResourceManager::updateMeshAsync(
                ResourceID rsrc_id,
                size_t vertex_offset,
                size_t index_offset,
                std::shared_ptr<std::vector<VertexContainer>> const & vertex_data,
                std::shared_ptr<IndexContainer> const & index_data)
            {
                m_renderThread_tasks.push(
                    /*std::async(*/[this, rsrc_id, vertex_offset, index_offset, vertex_data, index_data]() {

                    std::unique_lock<std::shared_mutex> lock(m_meshes_mutex);

                    auto query = m_id_to_mesh_idx.find(rsrc_id.value());

                    if (query != m_id_to_mesh_idx.end())
                    {
                        //while (!(m_meshes[query->second].state == READY)) {} // TODO somehow do this more efficiently

                        if (!(m_meshes[query->second].state == READY)) {
                            std::cerr << "ResourceManager - failed to update mesh" << std::endl;
                        }

                        VertexLayout vertex_layout = m_meshes[query->second].resource->getVertexLayout();

                        //TODO some sanity checks, such as attrib cnt and mesh size?

                        for (size_t attrib_idx = 0; attrib_idx < vertex_layout.attributes.size(); ++attrib_idx)
                        {
                            size_t attrib_byte_size = glowl::computeAttributeByteSize(vertex_layout.attributes[attrib_idx]);
                            size_t vertex_buffer_byte_offset = attrib_byte_size * vertex_offset;
                            m_meshes[query->second].resource->bufferVertexSubData(
                                attrib_idx,
                                (*vertex_data)[attrib_idx],
                                vertex_buffer_byte_offset);
                        }

                        size_t index_type_byte_size = glowl::computeByteSize(m_meshes[query->second].resource->getIndexType());
                        size_t index_byte_offset = index_offset * index_type_byte_size;
                        m_meshes[query->second].resource->bufferIndexSubData(
                            *index_data,
                            index_byte_offset);
                    }

                });
            }

            template<typename TexelDataContainer>
            inline ResourceID ResourceManager::createTexture2DAsync(
                std::string const & name,
                glowl::TextureLayout const & layout,
                std::shared_ptr<TexelDataContainer> const & data,
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
                m_name_to_textures_2d_idx.insert(std::pair<std::string, uint>(name, idx));

                m_renderThread_tasks.push([this, idx, name, layout, data, generateMipmap]() {
                    std::unique_lock<std::shared_mutex> tex_lock(m_textures_2d_mutex);

                    m_textures_2d[idx].resource = std::make_unique<glowl::Texture2D>(name, layout, data->data(), generateMipmap);
                    m_textures_2d[idx].state = READY;
                });

                return m_textures_2d[idx].id;
            }

            template<typename Container>
            WeakResource<glowl::BufferObject> ResourceManager::createBufferObject(
                std::string const& name,
                GLenum target,
                Container const& datastorage,
                GLenum usage)
            {
                {
                    std::shared_lock<std::shared_mutex> lock(m_buffers_mutex);
                    auto search = m_name_to_buffer_idx.find(name);
                    if (search != m_name_to_buffer_idx.end())
                        return WeakResource<glowl::BufferObject>(
                            m_buffers[search->second].id,
                            m_buffers[search->second].resource.get(),
                            m_buffers[search->second].state);
                }

                std::unique_lock<std::shared_mutex> lock(m_buffers_mutex);

                size_t idx = m_buffers.size();
                ResourceID rsrc_id = generateResourceID();

                m_buffers.push_back(Resource<glowl::BufferObject>(rsrc_id));
                m_id_to_buffer_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
                m_name_to_buffer_idx.insert(std::pair<std::string, size_t>(name, idx));

                m_buffers[idx].resource = std::make_unique<glowl::BufferObject>(target, datastorage, usage);
                m_buffers[idx].state = READY;

                return WeakResource<glowl::BufferObject>(
                    m_buffers[idx].id,
                    m_buffers[idx].resource.get(),
                    m_buffers[idx].state);
            }
        }
    }
}


#endif