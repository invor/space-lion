#include "LandscapeBrickComponent.hpp"

template<typename ResourceManagerType>
inline EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::LandscapeBrickComponentManager(WorldState& world_state, ResourceManagerType& resource_manager)
    : m_world(world_state),
    m_resource_mngr(resource_manager),
    m_voxelize_featureCurves(true), 
    m_voxelize_featureMeshes(true), 
    m_voxelize_heightmaps(true)
{
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this]
        {
            this->voxelize_heightmapMesh_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelize_heightmapMesh_gather_c.glsl" }, "lcsp_voxelize_heightmap").resource;

            //this->surfacePropagation_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_pull_c.glsl" }).get();

            // Create surface nets programs

            this->updatePtexTilesDisplacement_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesDisplacement_c.glsl" }, "lscp_updatePtexTilesDisplacement").resource;
            this->updatePtexTilesTextures_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesTextures_c.glsl" }, "lscp_updatePtexTilesTextures").resource;

            this->surfaceNets_classify_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/surfaceNets_classify_c.glsl" }, "surfaceNets_classify").resource;
            this->surfaceNets_generateQuads_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/surfaceNets_generateQuads_c.glsl" }, "surfaceNets_generateQuads").resource;
            this->computePtexNeighbours_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/computePtexNeighbours_c.glsl" }, "landscape_computePtexNeighbours").resource;
            this->computePatchDistances_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/computePatchDistances_c.glsl" }, "landscape_computePatchDistances").resource;
            this->updateTextureTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updateTextureTiles_c.glsl" }, "landscape_updateTextureTiles").resource;
            this->textureBaking_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/textureBaking_c.glsl" }, "landscape_textureBaking").resource;
            this->setInitialLODTextureTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/set_initial_LOD_textureTiles_c.glsl" }, "landscape_initialLODTextureTiles").resource;
            this->setPtexVistaTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/setPtexVistaTiles_c.glsl" }, "landscape_setPtexVistaTiles").resource;
            this->updatePtexTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTiles_c.glsl" }, "landscape_updatePtexTiles").resource;
            this->updatePtexTilesMipmaps_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesMipmaps_c.glsl" }, "landscape_updatePtexTilesMipmaps").resource;

            // create transform feedback program manually
            transformFeedback_terrainOutput_prgm = new GLSLProgram();
            transformFeedback_terrainOutput_prgm->init();

            std::string vertex_src = ResourceLoading::readShaderFile("../resources/shaders/dfr_landscapeSurface_v.glsl");
            std::string tessellationControl_src = ResourceLoading::readShaderFile("../resources/shaders/dfr_landscapeSurface_tc.glsl");
            std::string tessellationEvaluation_src = ResourceLoading::readShaderFile("../resources/shaders/dfr_landscapeSurface_te.glsl");

            transformFeedback_terrainOutput_prgm->bindAttribLocation(0, "vPosition");
            transformFeedback_terrainOutput_prgm->bindAttribLocation(1, "vNormal");

            if (!vertex_src.empty())
                if (!transformFeedback_terrainOutput_prgm->compileShaderFromString(&vertex_src, GL_VERTEX_SHADER)) { std::cout << transformFeedback_terrainOutput_prgm->getLog(); }
            if (!tessellationControl_src.empty())
                if (!transformFeedback_terrainOutput_prgm->compileShaderFromString(&tessellationControl_src, GL_TESS_CONTROL_SHADER)) { std::cout << transformFeedback_terrainOutput_prgm->getLog(); }
            if (!tessellationEvaluation_src.empty())
                if (!transformFeedback_terrainOutput_prgm->compileShaderFromString(&tessellationEvaluation_src, GL_TESS_EVALUATION_SHADER)) { std::cout << transformFeedback_terrainOutput_prgm->getLog(); }

            // transform feedback varyings
            const char* varyings[2] = { "tf_position", "tf_normal" };
            glTransformFeedbackVaryings(transformFeedback_terrainOutput_prgm->getHandle(), 2, varyings, GL_INTERLEAVED_ATTRIBS);

            if (!transformFeedback_terrainOutput_prgm->link()) { std::cout << transformFeedback_terrainOutput_prgm->getLog(); }

            // Generate buffer
            glGenBuffers(1, &transformFeedback_terrainBuffer);
        });
}

template<typename ResourceManagerType>
EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::~LandscapeBrickComponentManager()
{
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::addComponent(Entity entity, Vec3 position, Vec3 dimension, uint res_x, uint res_y, uint res_z)
{
    //m_bricks.push_back(LandscapeBrickComponent(entity,dimension,res_x,res_y,res_z));
    m_bricks.emplace_back(LandscapeBrickComponent(entity, dimension, res_x, res_y, res_z));
    GCoreComponents::transformManager().addComponent(entity, position);

    uint idx = static_cast<uint>(m_bricks.size() - 1);

    m_index_map.insert(std::pair<uint, uint>(entity.id(), idx));

    // TODO Think about thread safety. Do I have to lock down the whole LandscapeBrickManager when calling methods from the render thread?
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx] { this->createGpuResources(idx); });

    GEngineCore::renderingPipeline().addPerFrameComputeGpuTask([this, idx]()
        {
            //std::unique_lock<std::mutex> ptex_lock(m_ptex_update);

            std::unique_lock<std::mutex> ptex_lock(m_ptex_update, std::try_to_lock);
            if (ptex_lock.owns_lock())
                updateTextureTiles(idx);
        });

#if EDITOR_MODE // preprocessor definition

    std::vector<float> cv_interface_vertices({ -dimension.x / 2.0f,-dimension.y / 2.0f,dimension.z / 2.0f,0.0f,1.0f,0.0f,1.0f,
                                                dimension.x / 2.0f,-dimension.y / 2.0f,dimension.z / 2.0f,0.0f,1.0f,0.0f,1.0f,
                                                dimension.x / 2.0f,-dimension.y / 2.0f,-dimension.z / 2.0f,0.0f,1.0f,0.0f,1.0f,
                                                -dimension.x / 2.0f,-dimension.y / 2.0f,-dimension.z / 2.0f,0.0f,1.0f,0.0f,1.0f,
                                                -dimension.x / 2.0f,dimension.y / 2.0f,dimension.z / 2.0f,0.0f,1.0f,0.0f,1.0f,
                                                dimension.x / 2.0f,dimension.y / 2.0f,dimension.z / 2.0f,0.0f,1.0f,0.0f,1.0f,
                                                dimension.x / 2.0f,dimension.y / 2.0f,-dimension.z / 2.0f,0.0f,1.0f,0.0f,1.0f,
                                                -dimension.x / 2.0f,dimension.y / 2.0f,-dimension.z / 2.0f,0.0f,1.0f,0.0f,1.0 });
    std::vector<uint32_t> cv_interface_indices({ 0,1, 0,3, 0,4, 1,2, 1,5, 2,3, 2,6, 3,7, 4,5, 4,7, 5,6, 6,7 });

    VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
                                        VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });

    GRenderingComponents::interfaceMeshManager().addComponent(entity, "brick_" + std::to_string(entity.id()), "../resources/materials/editor/interface_cv.slmtl", cv_interface_vertices, cv_interface_indices, vertex_description, GL_LINES);

    // TODO add selectable component
#endif

}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::deleteComponent(uint index)
{

}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::addFeatureCurve(uint brick_index, Entity feature_curve_entity)
{
    m_bricks[brick_index].m_featureCurves.push_back(feature_curve_entity);
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::addFeatureMesh(uint brick_index, Entity feature_mesh_entity)
{
    m_bricks[brick_index].m_featureMeshes.push_back(feature_mesh_entity);
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::addHeightmapMesh(uint brick_index, Entity heightmap_entity)
{
    m_bricks[brick_index].m_heightmaps.push_back(heightmap_entity);
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::clearFeatureCurves(uint brick_index)
{
    m_bricks[brick_index].m_featureCurves.clear();
}

template<typename ResourceManagerType>
bool EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::isEmpty(uint brick_index)
{
    return (m_bricks[brick_index].m_featureCurves.size() == 0) ? true : false;
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::setNeighbour(uint index, NeighbourDirection direction, Entity neighbour)
{
    switch (direction)
    {
    case Landscape::LandscapeBrickComponentManager::EAST:
        m_bricks[index].m_eastern_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
        m_bricks[index].m_eastern_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
        m_bricks[index].m_eastern_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
        m_bricks[index].m_eastern_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
        m_bricks[index].m_east_neighbour = neighbour;
        break;
    case Landscape::LandscapeBrickComponentManager::WEST:
        m_bricks[index].m_western_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
        m_bricks[index].m_western_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
        m_bricks[index].m_western_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
        m_bricks[index].m_western_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
        m_bricks[index].m_west_neighbour = neighbour;
        break;
    case Landscape::LandscapeBrickComponentManager::DOWN:
        m_bricks[index].m_lower_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
        m_bricks[index].m_lower_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
        m_bricks[index].m_lower_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
        m_bricks[index].m_lower_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
        m_bricks[index].m_lower_neighbour = neighbour;
        break;
    case Landscape::LandscapeBrickComponentManager::UP:
        m_bricks[index].m_upper_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
        m_bricks[index].m_upper_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
        m_bricks[index].m_upper_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
        m_bricks[index].m_upper_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
        m_bricks[index].m_upper_neighbour = neighbour;
        break;
    case Landscape::LandscapeBrickComponentManager::SOUTH:
        m_bricks[index].m_southern_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
        m_bricks[index].m_southern_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
        m_bricks[index].m_southern_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
        m_bricks[index].m_southern_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
        m_bricks[index].m_south_neighbour = neighbour;
        break;
    case Landscape::LandscapeBrickComponentManager::NORTH:
        m_bricks[index].m_northern_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
        m_bricks[index].m_northern_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
        m_bricks[index].m_northern_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
        m_bricks[index].m_northern_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
        m_bricks[index].m_north_neighbour = neighbour;
        break;
    default:
        break;
    }
}

template<typename ResourceManagerType>
Vec3 EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::getDimension(uint index)
{
    return m_bricks[index].m_dimensions;
}

template<typename ResourceManagerType>
Vec3 EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::getResolution(uint index)
{
    return Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z);
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::setResolution(uint index, uint resX, uint resY, uint resZ)
{
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index, resX, resY, resZ] {	m_bricks[index].m_res_x = resX;
    m_bricks[index].m_res_y = resY;
    m_bricks[index].m_res_z = resZ;
    this->updateGpuResources(index); });
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::updateBrick(uint index)
{
    //while (!m_bricks[index].m_ptex_ready);
    //m_bricks[index].m_ptex_ready = false;
    //m_bricks[index].m_cancel_ptex_update = true;

    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->resetFields(index); });
    if (m_voxelize_featureCurves)
        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeFeatureCurves(index); });
    if (m_voxelize_featureMeshes)
        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeFeatureMeshes(index); });
    if (m_voxelize_heightmaps)
        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeHeightmapMeshes(index); });
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->averageVoxelization(index); });


    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeGuidanceField(index); });
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeNoiseField(index); });

    //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->expandFeatureCurves(index); });
    //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeFeatureCurves(index); });
    //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeGuidanceField(index); });

    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeSurfacePropagation(index); });
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->smoothSurfaceField(index); });
    //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeSurfaceMesh(index); } );
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeNaiveSurfaceNetsMesh(index); });
    //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->bakeSurfaceTextures(index); });

    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->updateDebugVolume(index); });

    //GEngineCore::renderingPipeline().addSingleExecutionGpuTask( [this,index] { 
    //	m_bricks[index].m_ptex_ready = true;
    //	m_bricks[index].m_cancel_ptex_update = false;
    //} );
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::updateNoiseAndMaterial(uint index)
{
    //TODO
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::updateCorrespondingBricks(Entity modified_feature_curve)
{
    //TODO write a more elegant version (currently brute force)

    uint brick_idx = 0;
    for (auto& brick : m_bricks)
    {
        for (auto& feature_curve : brick.m_featureCurves)
        {
            if (feature_curve == modified_feature_curve)
            {
                updateBrick(brick_idx);
            }
        }

        ++brick_idx;
    }
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::createGpuResources(uint index)
{
    //std::cout<<"Creating brick gpu resources."<<std::endl;

    if (m_bricks[index].m_head == nullptr)
    {
        m_bricks[index].m_head = GEngineCore::resourceManager().createSSBO("brick_" + std::to_string(index) + "head", (m_bricks[index].m_res_x * m_bricks[index].m_res_y * m_bricks[index].m_res_z) * 4 * 2, nullptr).resource;
    }
    if (m_bricks[index].m_guidancefield_data == nullptr)
    {
        m_bricks[index].m_guidancefield_data = GEngineCore::resourceManager().createSSBO("brick_" + std::to_string(index) + "guidancefield_data", (m_bricks[index].m_res_x * m_bricks[index].m_res_y * m_bricks[index].m_res_z) * 4 * 20 * 6, nullptr).resource;
    }


    TextureLayout rgba32f_layout(GL_RGBA32F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_FLOAT, 1,
        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
            std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

    // Create normal field resources
    if (m_bricks[index].m_normals == nullptr)
    {
        m_bricks[index].m_normals = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_normals", rgba32f_layout, nullptr).resource;
    }

    if (m_bricks[index].m_normals_backbuffer == nullptr)
    {
        m_bricks[index].m_normals_backbuffer = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_normals_backbuffer", rgba32f_layout, nullptr).resource;
    }

    // Create gradient field resources
    if (m_bricks[index].m_gradients == nullptr)
    {
        m_bricks[index].m_gradients = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_gradients", rgba32f_layout, nullptr).resource;
    }

    if (m_bricks[index].m_gradients_backbuffer == nullptr)
    {
        m_bricks[index].m_gradients_backbuffer = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_gradients_backbuffer", rgba32f_layout, nullptr).resource;
    }

    // Create atomic counter
    glGenBuffers(1, &(m_bricks[index].m_counter_buffer));
    GLuint zero = 0;
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, m_bricks[index].m_counter_buffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, 0);


    TextureLayout r16f_layout(GL_R16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RED, GL_HALF_FLOAT, 1,
        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
            std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

    // Create surface field
    if (m_bricks[index].m_surface == nullptr)
    {
        m_bricks[index].m_surface = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_surface", r16f_layout, nullptr).resource;
    }

    if (m_bricks[index].m_surface_backbuffer == nullptr)
    {
        m_bricks[index].m_surface_backbuffer = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_surface_backbuffer", r16f_layout, nullptr).resource;
    }


    TextureLayout r8ui_layout(GL_R8UI, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1,
        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
            std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

    if (m_bricks[index].m_surface_boundaryRegion == nullptr)
    {
        m_bricks[index].m_surface_boundaryRegion = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_surface_boundaryRegion", r8ui_layout, nullptr).resource;
    }



    TextureLayout rgba16f_layout(GL_RGBA16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_HALF_FLOAT, 1,
        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
            std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

    // Create noise parameters field
    if (m_bricks[index].m_noise_params == nullptr)
    {
        m_bricks[index].m_noise_params = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_noise_params", rgba16f_layout, nullptr).resource;
    }


    // Dummy surface mesh
    static float vertices[27] = { -10.0,0.0,-10.0, 0.0, 0.0,1.0,0.0, 0.0,0.0,
                                    10.0,0.0,-10.0, 0.0, 0.0,1.0,0.0, 0.0,0.0,
                                    0.0,0.0,10.0, 0.0, 0.0,1.0,0.0, 0.0,0.0 };
    std::vector<uint8_t> surface_vertices(reinterpret_cast<uint8_t*>(vertices), reinterpret_cast<uint8_t*>(vertices) + (24 * 4));
    std::vector<uint32_t> surface_indices({ 0,1,2 });
    //VertexLayout vertex_desc(8 * 4, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 3),VertexLayout::Attribute(GL_FLOAT,2,GL_FALSE,sizeof(GLfloat) * 6) });
    VertexLayout vertex_desc(9 * 4, { VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 4), VertexLayout::Attribute(GL_FLOAT, 1, GL_FALSE, sizeof(GLfloat) * 8) });
    m_bricks[index].m_surface_mesh = GEngineCore::resourceManager().createMesh("brick" + std::to_string(index) + "_surface_mesh", surface_vertices, surface_indices, vertex_desc, GL_PATCHES).resource;

    // Create landscape surface material
    //MaterialInfo surface_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_rocky_desert.slmtl");
    //m_bricks[index].m_surface_material = GEngineCore::resourceManager().createMaterial("../resources/materials/landscape_rocky_desert.slmtl",surface_material_info.shader_filepaths,surface_material_info.texture_filepaths).get();
    MaterialInfo surface_material_info = ResourceLoading::parseMaterial("../resources/materials/surfaceNets_landscape_coastline.slmtl");
    m_bricks[index].m_surface_material = GEngineCore::resourceManager().createMaterial("../resources/materials/surfaceNets_landscape_coastline.slmtl", surface_material_info.shader_filepaths, surface_material_info.texture_filepaths).resource;

    //GRenderingComponents::staticMeshManager().addComponent(m_bricks[index].m_entity, m_bricks[index].m_surface_mesh, m_bricks[index].m_surface_material);

    // Create shadow caster for landscape
    MaterialInfo shadowCast_material_info;
    shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/dfr_landscapeSurface_v.glsl");
    shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/dfr_landscapeSurface_tc.glsl");
    shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/landscape_shadowCaster_te.glsl");
    shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/pointlight_shadowMap_g.glsl");
    shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/pointlight_shadowMap_f.glsl");
    m_bricks[index].m_shadowCaster_material = GEngineCore::resourceManager().createMaterial("landscape_shadowCaster", shadowCast_material_info.shader_filepaths, shadowCast_material_info.texture_filepaths).resource;
    //GRenderingComponents::shadowCastMeshManager().addComponent(m_bricks[index].m_entity, m_bricks[index].m_surface_mesh, m_bricks[index].m_shadowCaster_material);

    // Ptex testing
    VertexLayout ptex_vertex_layout(6 * 4, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 3) });
    m_bricks[index].m_ptex_mesh = GEngineCore::resourceManager().createMeshAsync("brick" + std::to_string(index) + "_ptex_mesh", {}, {}, ptex_vertex_layout, GL_PATCHES);
    MaterialInfo surface_ptex_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_ptex.slmtl");
    m_bricks[index].m_ptex_material = GEngineCore::resourceManager().createMaterialAsync("landscape_ptex_material", surface_ptex_material_info.shader_filepaths, surface_ptex_material_info.texture_filepaths);
    m_bricks[index].m_ptex_parameters = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_parameters", 0, nullptr);
    m_bricks[index].m_ptex_parameters_backbuffer = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_parameters_backbuffer", 0, nullptr);
    m_bricks[index].m_ptex_bindless_texture_handles = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_bindless_texuture_handles", 0, nullptr);
    m_bricks[index].m_ptex_bindless_image_handles = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_bindless_image_handles", 0, nullptr);
    m_bricks[index].m_ptex_bindless_mipmap_image_handles = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_bindless_mipmap_image_handles", 0, nullptr);
    m_bricks[index].m_ptex_material_bth = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_material_bth", 0, nullptr);

    m_bricks[index].m_ptex_tiles_per_edge = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_tiles_per_edge", 0, nullptr);

    m_bricks[index].m_ptex_patch_distances_SSBO = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_patch_distances", 0, nullptr);
    m_bricks[index].m_ptex_availableTiles_SSBO = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_availableTiles", 0, nullptr);

    GRenderingComponents::ptexMeshManager().addComponent(m_bricks[index].m_entity, m_bricks[index].m_ptex_mesh, m_bricks[index].m_ptex_material, m_bricks[index].m_ptex_parameters, m_bricks[index].m_ptex_bindless_texture_handles, true);


    // create update/free SSBOs
    m_bricks[index].m_ptex_updatePatches_tgt_SSBO = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_updatePatches", 0, nullptr);
    m_bricks[index].m_ptex_updatePatches_src_SSBO = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_freeSlots", 0, nullptr);


    addDebugVolume(index);
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::updateGpuResources(uint index)
{
    m_bricks[index].m_head->reload((m_bricks[index].m_res_x * m_bricks[index].m_res_y * m_bricks[index].m_res_z) * 4 * 2, 0, 0);
    m_bricks[index].m_guidancefield_data->reload((m_bricks[index].m_res_x * m_bricks[index].m_res_y * m_bricks[index].m_res_z) * 4 * 8 * 5, 0, 0);

    TextureLayout rgba32f_layout(GL_RGBA32F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_FLOAT, 1,
        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
            std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

    m_bricks[index].m_normals->reload(rgba32f_layout, nullptr);
    m_bricks[index].m_normals_backbuffer->reload(rgba32f_layout, nullptr);

    m_bricks[index].m_gradients->reload(rgba32f_layout, nullptr);
    m_bricks[index].m_gradients_backbuffer->reload(rgba32f_layout, nullptr);

    TextureLayout r16f_layout(GL_R16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RED, GL_HALF_FLOAT, 1,
        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
            std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

    m_bricks[index].m_surface->reload(r16f_layout, nullptr);
    m_bricks[index].m_surface_backbuffer->reload(r16f_layout, nullptr);

    TextureLayout rgba16f_layout(GL_RGBA16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_HALF_FLOAT, 1,
        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
            std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
            std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

    m_bricks[index].m_noise_params->reload(rgba16f_layout, nullptr);
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::expandFeatureCurves(uint index)
{
    // For now, fetch texture data from GPU and expand feature curves on the CPU

    m_bricks[index].m_normals->bindTexture();
    std::vector<glm::vec4> normal_data(m_bricks[index].m_normals->getWidth() * m_bricks[index].m_normals->getHeight() * m_bricks[index].m_normals->getDepth());
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, normal_data.data());

    m_bricks[index].m_gradients->bindTexture();
    std::vector<glm::vec4> gradient_data(m_bricks[index].m_gradients->getWidth() * m_bricks[index].m_gradients->getHeight() * m_bricks[index].m_gradients->getDepth());
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, gradient_data.data());

    m_bricks[index].m_surface->bindTexture();
    std::vector<GLubyte> surface_data(m_bricks[index].m_surface->getWidth() * m_bricks[index].m_surface->getHeight() * m_bricks[index].m_surface->getDepth());
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_UNSIGNED_BYTE, surface_data.data());

    for (auto& feature_curve : m_bricks[index].m_featureCurves)
    {
        //TODO scale relative to brick size

        uint curve_idx = GLandscapeComponents::featureCurveManager().getIndex(feature_curve);

        Vec3 end_point = GLandscapeComponents::featureCurveManager().calculateCurvePoint(curve_idx, 1.0);

        float distance = 0.0;

        for (int i = 0; i < 200; i++)
        {
            // get position in world space
            Vec3 point = Vec3(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(feature_curve)) * glm::vec4(end_point, 1.0));
            // get brick origin in world space
            Vec3 brick_origin = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity)) - m_bricks[index].m_dimensions / 2.0f;
            // compute position in brick volume space
            point = (point - brick_origin) / m_bricks[index].m_dimensions;

            if (!((point.x >= 0.0 && point.x <= 1.0) &&
                (point.y >= 0.0 && point.y <= 1.0) &&
                (point.z >= 0.0 && point.z <= 1.0)))
            {
                break;
            }

            // Check if near to another feature curve (naive)
            //	for (auto& fc : m_bricks[index].m_featureCurves)
            //	{
            //		uint fc_idx = GLandscapeComponents::featureCurveManager().getIndex(fc);
            //		GLandscapeComponents::featureCurveManager().
            //		for(auto& cv : fc->)
            //	}


            point *= Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z);

            // compute normal data access index
            uint normal_data_idx = (uint)point.x + ((uint)point.y * m_bricks[index].m_res_x) + ((uint)point.z * m_bricks[index].m_res_x * m_bricks[index].m_res_y);

            // compute direction based on vector field using trilinear interpolation

            uint x_offset = 1;
            uint y_offset = m_bricks[index].m_res_x;
            uint z_offset = (m_bricks[index].m_res_x * m_bricks[index].m_res_y);

            uint c000_idx = (uint)point.x + ((uint)point.y * m_bricks[index].m_res_x) + ((uint)point.z * m_bricks[index].m_res_x * m_bricks[index].m_res_y);
            uint c001_idx = c000_idx + z_offset;
            uint c010_idx = c000_idx + y_offset;
            uint c011_idx = c000_idx + y_offset + z_offset;
            uint c100_idx = c000_idx + x_offset;
            uint c101_idx = c000_idx + x_offset + z_offset;
            uint c110_idx = c000_idx + x_offset + y_offset;
            uint c111_idx = c000_idx + x_offset + y_offset + z_offset;

            Vec3 n00 = Vec3(normal_data[c000_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(normal_data[c100_idx]) * (point.x - int(point.x));
            Vec3 n01 = Vec3(normal_data[c001_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(normal_data[c101_idx]) * (point.x - int(point.x));
            Vec3 n10 = Vec3(normal_data[c010_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(normal_data[c110_idx]) * (point.x - int(point.x));
            Vec3 n11 = Vec3(normal_data[c011_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(normal_data[c111_idx]) * (point.x - int(point.x));
            Vec3 n0 = n00 * (1.0f - (point.y - int(point.y))) + n10 * (point.y - int(point.y));
            Vec3 n1 = n01 * (1.0f - (point.y - int(point.y))) + n11 * (point.y - int(point.y));
            Vec3 normal = glm::normalize(n0 * (1.0f - (point.z - int(point.z))) + n1 * (point.y - int(point.z)));

            Vec3 g00 = Vec3(gradient_data[c000_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(gradient_data[c100_idx]) * (point.x - int(point.x));
            Vec3 g01 = Vec3(gradient_data[c001_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(gradient_data[c101_idx]) * (point.x - int(point.x));
            Vec3 g10 = Vec3(gradient_data[c010_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(gradient_data[c110_idx]) * (point.x - int(point.x));
            Vec3 g11 = Vec3(gradient_data[c011_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(gradient_data[c111_idx]) * (point.x - int(point.x));
            Vec3 g0 = g00 * (1.0f - (point.y - int(point.y))) + g10 * (point.y - int(point.y));
            Vec3 g1 = g01 * (1.0f - (point.y - int(point.y))) + g11 * (point.y - int(point.y));
            Vec3 gradient = glm::normalize(g0 * (1.0f - (point.z - int(point.z))) + g1 * (point.y - int(point.z)));


            Vec3 tangent = 3.0f * glm::normalize(end_point - GLandscapeComponents::featureCurveManager().calculateCurvePoint(curve_idx, 0.95f));

            //Vec3 dir = glm::normalize(glm::cross(normal, tangent));
            //dir = glm::normalize(glm::cross(dir, normal)) * 0.1f;

            Vec3 dir = glm::normalize(glm::cross(normal, gradient)) * 0.1f;

            // make sure to follow the guidance field in the direction of the tangent
            float sign = (glm::dot(dir, tangent) < 0.0f) ? -1.0f : 1.0f;

            dir *= sign;

            distance += glm::length(dir);

            //TODO break condition
            if (distance > 5.0)
            {
                GLandscapeComponents::featureCurveManager().addControlVertex(curve_idx, end_point + dir);
                distance = 0.0;
            }

            end_point += dir;


#if EDITOR_MODE // preprocessor definition

            Entity normals = GEngineCore::entityManager().create();
            GCoreComponents::transformManager().addComponent(normals, Vec3(0.0));

            Vec3 v0 = Vec3(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(feature_curve)) * glm::vec4(end_point, 1.0));;
            Vec3 v1 = v0 + gradient;
            std::vector<float> cv_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
                                                        v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0 });
            std::vector<uint> cv_interface_indices({ 0,1 });

            VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
                VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });

            GRenderingComponents::interfaceMeshManager().addComponent(normals, "brick_" + std::to_string(normals.id()), "../resources/materials/editor/interface_cv.slmtl", cv_interface_vertices, cv_interface_indices, vertex_description, GL_LINES);

            // TODO add selectable component
#endif
        }
    }

    // continue pipeline
    //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeFeatureCurves(index); });
    //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeGuidanceField(index); });

    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeSurfacePropagation(index); });
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeSurfaceMesh(index); });

    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->addDebugVolume(index); });
}



/*
    void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::updateTextureTiles(uint index)
    {
        WeakResource<ShaderStorageBufferObject> ptex_bindless_texture_handles_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_texture_handles);
        WeakResource<ShaderStorageBufferObject> ptex_bindless_images_handles_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_image_handles);
        WeakResource<ShaderStorageBufferObject> ptex_parameters_backbuffer_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters_backbuffer);
        WeakResource<ShaderStorageBufferObject> ptex_parameters_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters);
        WeakResource<ShaderStorageBufferObject> ptex_material_bth_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_material_bth);
        WeakResource<ShaderStorageBufferObject> updatePatches_SSBO_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_updatePatches_tgt_SSBO);
        WeakResource<ShaderStorageBufferObject> availableTiles_SSBO_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_availableTiles_SSBO);

        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        // copy previous ptex params to backbuffer
        ShaderStorageBufferObject::copy(ptex_parameters_resource.resource, ptex_parameters_backbuffer_resource.resource);

        // upload update information to GPU
        updatePatches_SSBO_resource.resource->reload(m_bricks[index].m_ptex_updatePatches_tgt);
        availableTiles_SSBO_resource.resource->reload(m_bricks[index].m_ptex_availableTiles_uploadBuffer);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // TODO per LOD level dispatch computes
        int update_patch_offset = 0;
        int texture_slot_offset = 0;
        int tile_size_multiplier = 32;

        for (size_t i = 0; i < m_bricks[index].m_ptex_update_bin_sizes.size() - 1; ++i)
        {
            // available textures should always be >= update patches per LOD
            assert(m_bricks[index].m_ptex_availableTiles_bin_sizes[i] >= m_bricks[index].m_ptex_update_bin_sizes[i]);

            uint32_t bin_size = m_bricks[index].m_ptex_update_bin_sizes[i];
            float texture_lod = static_cast<float>(i);

            // TODO split bin size depending on amount of texels to avoid single large tasks

            uint32_t update_texel_cnt = bin_size * tile_size_multiplier * tile_size_multiplier * 8 * 8;
            int sub_tasks = std::max(1, int(update_texel_cnt / 2000000));
            sub_tasks = 1; // Splitting into sub tasks currently produces buggy surface tiles

            uint32_t sub_bin_size = bin_size / sub_tasks;
            uint32_t remaining_bin_size = bin_size;

            uint32_t remaining_tex_bin_size = m_bricks[index].m_ptex_availableTiles_bin_sizes[i];

            for (int j = 0; j < sub_tasks; ++j)
            {
                GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this,
                    index,
                    texture_lod,
                    sub_bin_size,
                    remaining_bin_size,
                    tile_size_multiplier,
                    update_patch_offset,
                    texture_slot_offset,
                    ptex_bindless_images_handles_resource,
                    ptex_bindless_texture_handles_resource,
                    ptex_parameters_resource,
                    ptex_parameters_backbuffer_resource,
                    updatePatches_SSBO_resource,
                    ptex_material_bth_resource,
                    availableTiles_SSBO_resource]()
                {
                    GLuint64 t_0, t_1;
                    unsigned int queryID[2];
                    // generate two queries
                    glGenQueries(2, queryID);
                    glQueryCounter(queryID[0], GL_TIMESTAMP);

                    // Bind all SSBOs
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bricks[index].m_surface_mesh->getVboHandle());
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bricks[index].m_surface_mesh->getIboHandle());
                    ptex_bindless_images_handles_resource.resource->bind(2);
                    ptex_parameters_resource.resource->bind(3);
                    ptex_material_bth_resource.resource->bind(5);
                    updatePatches_SSBO_resource.resource->bind(6);
                    availableTiles_SSBO_resource.resource->bind(7);
                    ptex_bindless_texture_handles_resource.resource->bind(8);

                    updatePtexTilesDisplacement_prgm->use();

                    updatePtexTilesDisplacement_prgm->setUniform("texture_lod", texture_lod + 1.0f); //TODO more accurate computation of fitting mipmap level for source textures
                    updatePtexTilesDisplacement_prgm->setUniform("update_patch_offset", update_patch_offset);
                    updatePtexTilesDisplacement_prgm->setUniform("texture_slot_offset", texture_slot_offset);

                    uint32_t task_bin_size = std::min(sub_bin_size, remaining_bin_size);
                    updatePtexTilesDisplacement_prgm->dispatchCompute(tile_size_multiplier, tile_size_multiplier, task_bin_size);

                    glMemoryBarrier(GL_ALL_BARRIER_BITS);

                    // set GLSL program
                    updatePtexTilesTextures_prgm->use();

                    updatePtexTilesTextures_prgm->setUniform("texture_lod", texture_lod + 1.0f); //TODO more accurate computation of fitting mipmap level for source textures
                    updatePtexTilesTextures_prgm->setUniform("update_patch_offset", update_patch_offset);
                    updatePtexTilesTextures_prgm->setUniform("texture_slot_offset", texture_slot_offset);

                    task_bin_size = std::min(sub_bin_size, remaining_bin_size);
                    updatePtexTilesTextures_prgm->dispatchCompute(tile_size_multiplier, tile_size_multiplier, task_bin_size);

                    glMemoryBarrier(GL_ALL_BARRIER_BITS);



                    glQueryCounter(queryID[1], GL_TIMESTAMP);

                    // wait until the results are available
                    GLint stopTimerAvailable = 0;
                    while (!stopTimerAvailable)
                        glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

                    // get query results
                    glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
                    glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

                    //std::cout << "Updating " << task_bin_size << " surface " << tile_size_multiplier * 8 << "x" << tile_size_multiplier * 8 << " patches in ";
                    //std::cout << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
                });

                update_patch_offset += std::min(sub_bin_size, remaining_bin_size);
                texture_slot_offset += std::min(sub_bin_size, remaining_bin_size);

                remaining_tex_bin_size -= std::min(sub_bin_size, remaining_bin_size);
                remaining_bin_size -= sub_bin_size;
            }

            texture_slot_offset += remaining_tex_bin_size;

            tile_size_multiplier /= 2;
        }


        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index, ptex_parameters_resource, updatePatches_SSBO_resource, update_patch_offset]()
        {
            // Assign vista tiles (no recomutation necessary)
            if (m_bricks[index].m_ptex_update_bin_sizes.back() > 0)
            {
                setPtexVistaTiles_prgm->use();

                ptex_parameters_resource.resource->bind(3);
                updatePatches_SSBO_resource.resource->bind(6);

                setPtexVistaTiles_prgm->setUniform("update_patch_offset", update_patch_offset);

                int texture_base_idx = m_bricks[index].m_ptex_textures.size() - (m_bricks[index].m_ptex_lod_bin_sizes.back() * 4) / 2048;
                setPtexVistaTiles_prgm->setUniform("texture_base_idx", texture_base_idx);
                setPtexVistaTiles_prgm->setUniform("vista_patch_cnt", m_bricks[index].m_ptex_update_bin_sizes.back());

                setPtexVistaTiles_prgm->dispatchCompute(1, 1, (m_bricks[index].m_ptex_update_bin_sizes.back() / 32) + 1);
            }

            m_ptex_update_in_progress.clear();
        });

    }
*/

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::updateTextureBaking(uint index)
{
    // don't start additional update task if some task are even already working on the data
    //{
    //	std::unique_lock<std::mutex> ptex_lock(m_ptex_update, std::try_to_lock);
    //	if (!ptex_lock.owns_lock()) return;
    //}

    if (!m_bricks[index].m_ptex_ready)
        return;

    m_bricks[index].m_ptex_ready = false;

    GEngineCore::taskSchedueler().submitTask([this, index]() {

        std::unique_lock<std::mutex> ptex_lock(m_ptex_update);

        computePatchDistances(index);

        computeTextureTileUpdateList(index);

        //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
        //	{
        //		std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
        //	
        //		updateTextureTiles(index);
        //	});

        });
}


template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::setDebugField(uint index, Datafield field_selection)
{
    if (field_selection == m_bricks[index].m_debugField_selection)
        return;

    m_bricks[index].m_debugField_selection = field_selection;

    updateDebugVolume(index);
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::setSurfaceMaterial(uint index, ResourceID mat)
{
    m_bricks[index].m_surface_material = mat;

    GRenderingComponents::staticMeshManager().updateComponent(m_bricks[index].m_entity, mat);
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::setVoxelizeFeatureCurves(bool voxelize)
{
    m_voxelize_featureCurves = voxelize;
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::setVoxelizeFeatureMeshes(bool voxelize)
{
    m_voxelize_featureMeshes = voxelize;
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::setVoxelizeHeightmaps(bool voxelize)
{
    m_voxelize_heightmaps = voxelize;
}

template<typename ResourceManagerType>
void EngineCore::Graphics::Landscape::LandscapeBrickComponentManager<ResourceManagerType>::updateShaderPrograms()
{
    GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this]
        {
            this->voxelize_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_gather_c.glsl" }, "lcsp_voxelize").resource;
            this->voxelize_mesh_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelize_mesh_gather_c.glsl" }, "lcsp_voxelize_mesh").resource;
            this->voxelize_heightmapMesh_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelize_heightmapMesh_gather_c.glsl" }, "lcsp_voxelize_heightmap").resource;
            this->average_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_average_c.glsl" }, "lcsp_voxelize_average").resource;
            this->reset_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_reset_c.glsl" }, "lcsp_reset").resource;
            this->buildGuidance_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/buildGuidanceField_c.glsl" }, "lcsp_buildGuidance").resource;

            this->buildNoiseField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/buildNoiseField_c.glsl" }, "lcsp_buildNoiseField").resource;
            this->copyNoiseField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/copyTexture3D_RGBA16_c.glsl" }, "lcsp_copyNoiseField").resource;

            //this->surfacePropagation_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_pull_c.glsl" }).get();
            this->surfacePropagationInit_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_initBoundaryRegion_c.glsl" }, "lcsp_surfacePropagationInit").resource;
            this->surfacePropagation_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_distApprox_c.glsl" }, "lcsp_surfacePropagation").resource;
            this->copySurfaceField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/copyTexture3D_R16_c.glsl" }, "lcsp_copySurfaceField").resource;
            this->smooth_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/seperatedGaussian3d_c.glsl" }, "lcsp_smooth").resource;

            this->classifyVoxels_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_classify_c.glsl" }, "lcsp_classifyVoxels").resource;

            // Generate prgms for different datatypes on different levels by insertig define files
            this->buildHpLvl_R8_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, ("#define SRC_R8UI\n#define TGT_R8UI\n"), "lcsp_buildHpLvl_R8").resource;
            this->buildHpLvl_R8toR16_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R8UI\n#define TGT_R16UI\n", "lcsp_buildHpLvl_R8toR16").resource;
            this->buildHpLvl_R16_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R16UI\n#define TGT_R16UI\n", "lcsp_buildHpLvl_R16").resource;
            this->buildHpLvl_R16toR32_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R16UI\n#define TGT_R32UI\n", "lcsp_buildHpLvl_R16to32").resource;
            this->buildHpLvl_R32_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R32UI\n#define TGT_R32UI\n", "lcsp_buildHpLvl_R32").resource;

            this->generateTriangles_prgms.resize();
            this->generateTriangles_prgms[0] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 6\n", "lcsp_generateTriangles_L6").resource;
            this->generateTriangles_prgms[1] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 7\n", "lcsp_generateTriangles_L7").resource;
            this->generateTriangles_prgms[2] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 8\n", "lcsp_generateTriangles_L8").resource;
            this->generateTriangles_prgms[3] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 9\n", "lcsp_generateTriangles_L9").resource;
            this->generateTriangles_prgms[4] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 10\n", "lcsp_generateTriangles_L10").resource;

            // Create surface nets programs
            this->surfaceNets_classify_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/surfaceNets_classify_c.glsl" }, "surfaceNets_classify").resource;
            this->surfaceNets_generateQuads_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/surfaceNets_generateQuads_c.glsl" }, "surfaceNets_generateQuads").resource;
            this->computePtexNeighbours_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/computePtexNeighbours_c.glsl" }, "landscape_computePtexNeighbours").resource;
            this->computePatchDistances_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/computePatchDistances_c.glsl" }, "landscape_computePatchDistances").resource;
            this->updateTextureTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updateTextureTiles_c.glsl" }, "landscape_updateTextureTiles").resource;
            this->textureBaking_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/textureBaking_c.glsl" }, "landscape_textureBaking").resource;
            this->setInitialLODTextureTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/set_initial_LOD_textureTiles_c.glsl" }, "landscape_initialLODTextureTiles").resource;
            this->setPtexVistaTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/setPtexVistaTiles_c.glsl" }, "landscape_setPtexVistaTiles").resource;
            this->updatePtexTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTiles_c.glsl" }, "landscape_updatePtexTiles").resource;
            this->updatePtexTilesDisplacement_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesDisplacement_c.glsl" }, "lscp_updatePtexTilesDisplacement").resource;
            this->updatePtexTilesTextures_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesTextures_c.glsl" }, "lscp_updatePtexTilesTextures").resource;
        });
}