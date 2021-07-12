#ifndef LandscapeBrickComponent_hpp
#define LandscapeBrickComponent_hpp

#include "BaseResourceManager.hpp"
#include "BaseSingleInstanceComponentManager.hpp"
#include "EntityManager.hpp"

namespace EngineCore {
    namespace Graphics {
        namespace Landscape
        {

            class LandscapeBrickComponentManager : public BaseSingleInstanceComponentManager
            {
            public:
                enum NeighbourDirection { EAST, WEST, DOWN, UP, SOUTH, NORTH };
                enum Datafield { NORMAL, GRADIENT, NOISE, SURFACE, SURFACE_BOUNDARY };

            private:
                struct LandscapeBrickComponent
                {
                    LandscapeBrickComponent(Entity entity, Vec3 dimensions, uint res_x, uint res_y, uint res_z)
                        : m_entity(entity), m_dimensions(dimensions), m_res_x(res_x), m_res_y(res_y), m_res_z(res_z),
                        m_normals(nullptr), m_normals_backbuffer(nullptr), m_head(nullptr), m_guidancefield_data(nullptr),
                        m_gradients(nullptr), m_gradients_backbuffer(nullptr),
                        m_noise_params(nullptr), m_surface(nullptr), m_surface_backbuffer(nullptr), m_surface_boundaryRegion(nullptr),
                        m_upper_neighbour(entity),
                        m_lower_neighbour(entity),
                        m_south_neighbour(entity),
                        m_north_neighbour(entity),
                        m_west_neighbour(entity),
                        m_east_neighbour(entity),
                        m_debugField_selection(Datafield::NORMAL),
                        m_lod_lvls(0),
                        m_ptex_ready(true),
                        m_cancel_ptex_update(false),
                        m_ptex_mesh(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_textures(),
                        m_ptex_material(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_material_bth(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_parameters(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_parameters_backbuffer(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_bindless_texture_handles(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_bindless_image_handles(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_bindless_mipmap_image_handles(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_tiles_per_edge(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_updatePatches_tgt_SSBO(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_updatePatches_src_SSBO(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_patch_distances_SSBO(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_availableTiles_SSBO(GEngineCore::resourceManager().getInvalidResourceID()),
                        m_ptex_decal_list_rsrc(GEngineCore::resourceManager().getInvalidResourceID()),
                        ptex_distance_computation_time(0.0),
                        ptex_distance_computation_avg_time(0.0),
                        ptex_distance_computation_max_time(0.0),
                        ptex_updateList_computation_time(0.0),
                        ptex_updateList_computation_avg_time(0.0),
                        ptex_updateList_computation_max_time(0.0),
                        ptex_tileUpdate_time(0.0),
                        ptex_tileUpdate_avg_time(0.0),
                        ptex_tileUpdate_max_time(0.0),
                        updates_made(0),
                        ptex_updated_primitives(0),
                        ptex_updated_primitives_avg(0),
                        ptex_updated_primitives_max(0) {}

                    Entity m_entity;

                    Vec3 m_dimensions;
                    uint m_res_x, m_res_y, m_res_z;

                    /** Volume containing surface normals. Guidance field for building the surface. */
                    Texture3D* m_normals; //Texture3D
                    Texture3D* m_normals_backbuffer; //Texture3D

                    /** Volume containing surface gradients. Part of the guidance field for building the surface. */
                    Texture3D* m_gradients;
                    Texture3D* m_gradients_backbuffer;

                    ShaderStorageBufferObject* m_head;
                    ShaderStorageBufferObject* m_guidancefield_data;
                    GLuint m_counter_buffer;

                    /** 3D field of 2D containing surface noise parameters */
                    Texture3D* m_noise_params;

                    /** 3D field of 2D entries containing surface signed distance field and surface propagation weights */
                    Texture3D* m_surface;
                    Texture3D* m_surface_backbuffer;
                    Texture3D* m_surface_boundaryRegion;

                    // Boundary conditions of brick
                    Texture3D* m_upper_boundary[4];
                    Texture3D* m_lower_boundary[4];
                    Texture3D* m_northern_boundary[4];
                    Texture3D* m_southern_boundary[4];
                    Texture3D* m_western_boundary[4];
                    Texture3D* m_eastern_boundary[4];

                    Entity m_upper_neighbour;
                    Entity m_lower_neighbour;
                    Entity m_south_neighbour;
                    Entity m_north_neighbour;
                    Entity m_west_neighbour;
                    Entity m_east_neighbour;

                    /** Mesh representation of computed surface */
                    Mesh* m_surface_mesh;
                    Material* m_surface_material;
                    Material* m_shadowCaster_material;

                    ResourceID				m_ptex_mesh;
                    std::vector<ResourceID> m_ptex_textures;
                    ResourceID				m_ptex_material;
                    ResourceID				m_ptex_material_bth; // ptex surface material bindless texture handles
                    ResourceID				m_ptex_parameters;
                    ResourceID				m_ptex_parameters_backbuffer;

                    ResourceID				m_ptex_bindless_texture_handles;
                    ResourceID				m_ptex_bindless_image_handles;
                    ResourceID				m_ptex_bindless_mipmap_image_handles;
                    ResourceID				m_ptex_tiles_per_edge;

                    /** CPU-side storage of computed ptex surface mesh */
                    std::vector<float>		m_mesh_vertex_data;
                    std::vector<uint32_t>	m_mesh_index_data;

                    struct PtexParameters
                    {
                        int32_t ngbr_ptex_param_indices[4]; // index into PtexParameters buffers
                        uint32_t tile_classification;

                        // Save space by storing index into buffer with all texture handles instead of storing 64bit handles
                        // (for texture arrays handle is shared by many faces)
                        uint32_t texture_index; // index into texture indirection buffer (where all bindless texture handles are stored)
                        uint32_t base_slice; // slice in texture array
                    };
                    std::vector<PtexParameters> m_mesh_ptex_params;


                    // Level of detail ptex tile management prototype

                    int m_lod_lvls;
                    bool m_ptex_ready;
                    bool m_cancel_ptex_update;

                    std::vector<uint>	m_ptex_updatePatches_tgt;
                    std::vector<uint>	m_ptex_updatePatches_src;
                    ResourceID			m_ptex_updatePatches_tgt_SSBO; // SSBO containing the tile IDs that need to be updated ordered by update target tile size
                    ResourceID			m_ptex_updatePatches_src_SSBO; // SSBO containing the tile IDs that need to be updated ordered by previous tile size
                    std::vector<uint>	m_ptex_update_bin_sizes;

                    struct PatchInfo
                    {
                        Vec3 midpoint;
                        float distance;
                        uint32_t tex_index;
                        uint32_t base_slice;
                    };

                    struct TextureSlot
                    {
                        uint32_t tex_index;
                        uint32_t base_slice;
                    };

                    ResourceID             m_ptex_patch_distances_SSBO;
                    std::vector<PatchInfo> m_ptex_patch_distances;
                    std::vector<uint>      m_ptex_lod_bin_sizes; // store number of texture tiles per LOD level
                    std::vector<uint>      m_ptex_active_patch_lod_classification; // store currently active (i.e. rendered) LOD classification of all patches
                    std::vector<uint>      m_ptex_latest_patch_lod_classification; // store lastest LOD classification of all patches

                    std::vector<std::list<TextureSlot>>	m_ptex_active_availableTiles;
                    std::vector<std::list<TextureSlot>>	m_ptex_latest_availableTiles;

                    std::vector<TextureSlot> m_ptex_availableTiles_uploadBuffer;
                    std::vector<uint>        m_ptex_availableTiles_bin_sizes;
                    ResourceID               m_ptex_availableTiles_SSBO;

                    struct Decals {
                        uint decal_cnt;
                        uint decal_idx_0;
                        uint decal_idx_1;
                        uint decal_idx_2;
                        uint decal_idx_3;
                        uint decal_idx_4;
                        uint decal_idx_5;
                        uint decal_idx_6;
                    };
                    std::vector<Decals> m_ptex_decal_list;
                    ResourceID          m_ptex_decal_list_rsrc; // buffer with a per-primitive list of decals

                    double ptex_distance_computation_time;
                    double ptex_distance_computation_avg_time;
                    double ptex_distance_computation_max_time;
                    double ptex_updateList_computation_time;
                    double ptex_updateList_computation_avg_time;
                    double ptex_updateList_computation_max_time;
                    double ptex_tileUpdate_time;
                    double ptex_tileUpdate_avg_time;
                    double ptex_tileUpdate_max_time;
                    uint updates_made;
                    uint ptex_updated_primitives;
                    uint ptex_updated_primitives_avg;
                    uint ptex_updated_primitives_max;


                    Datafield m_debugField_selection;

                    /** List of features curves located within the volume brick */
                    std::vector<Entity> m_featureCurves;

                    std::vector<Entity> m_featureMeshes;

                    std::vector<Entity> m_heightmaps;
                };

                std::vector<LandscapeBrickComponent> m_bricks;

                bool m_voxelize_featureCurves;
                bool m_voxelize_featureMeshes;
                bool m_voxelize_heightmaps;

                mutable std::mutex m_ptex_update;

                void expandFeatureCurves(uint index);

                /******************************************************
                * GPU methods (only call on render thread!)
                *****************************************************/

                ResourceID voxelize_prgm;
                ResourceID voxelize_mesh_prgm;
                ResourceID voxelize_heightmapMesh_prgm;
                ResourceID average_prgm;
                ResourceID reset_prgm;
                ResourceID buildGuidance_prgm;
                ResourceID buildNoiseField_prgm;
                ResourceID copyNoiseField_prgm;
                ResourceID surfacePropagationInit_prgm;
                ResourceID surfacePropagation_prgm;
                ResourceID copySurfaceField_prgm;
                ResourceID smooth_prgm;
                ResourceID classifyVoxels_prgm;
                ResourceID buildHpLvl_R8_prgm;
                ResourceID buildHpLvl_R8toR16_prgm;
                ResourceID buildHpLvl_R16_prgm;
                ResourceID buildHpLvl_R16toR32_prgm;
                ResourceID buildHpLvl_R32_prgm;
                std::vector<ResourceID> generateTriangles_prgms;

                ResourceID surfaceNets_classify_prgm;
                ResourceID surfaceNets_generateQuads_prgm;
                ResourceID computePtexNeighbours_prgm;
                ResourceID computePatchDistances_prgm;
                ResourceID updateTextureTiles_prgm; // deprecated
                ResourceID updatePtexTiles_prgm;
                ResourceID updatePtexTilesMipmaps_prgm;
                ResourceID textureBaking_prgm;
                ResourceID setInitialLODTextureTiles_prgm;
                ResourceID setPtexVistaTiles_prgm;
                ResourceID updatePtexTilesDisplacement_prgm; // future work
                ResourceID updatePtexTilesTextures_prgm; // future work

                ResourceID transformFeedback_terrainOutput_prgm;
                GLuint transformFeedback_terrainBuffer;

                void createGpuResources(uint index);
                void updateGpuResources(uint index);
                void resetFields(uint index);
                void voxelizeFeatureCurves(uint index);
                void voxelizeFeatureMeshes(uint index);
                void voxelizeHeightmapMeshes(uint index);
                void averageVoxelization(uint index);
                void computeGuidanceField(uint index, uint iterations = 0);
                void computeNoiseField(uint index, uint interations = 0);
                void computeSurfacePropagation(uint index, uint iterations = 0);
                void smoothSurfaceField(uint index);
                void computeSurfaceMesh(uint index);
                void computeNaiveSurfaceNetsMesh(uint index);
                void bakeSurfaceTextures(uint index);

                void computePatchDistances(uint index);
                void computeTextureTileUpdateList(uint index);
                void updateTextureTiles(uint index);

                void fillSurfaceMeshGaps(uint index);
                void addDebugVolume(uint index);
                void updateDebugVolume(uint index);

            public:
                LandscapeBrickComponentManager();
                ~LandscapeBrickComponentManager();

                //LandscapeBrickComponentManager(LandscapeBrickComponentManager const& cpy) = delete;

                void addComponent(Entity entity, Vec3 position, Vec3 dimension, uint res_x, uint res_y, uint res_z);

                void deleteComponent(uint index);

                void addFeatureCurve(uint brick_index, Entity feature_curve_entity);

                void addFeatureMesh(uint brick_index, Entity feature_mesh_entity);

                void addHeightmapMesh(uint brick_index, Entity heightmap_entity);

                void clearFeatureCurves(uint brick_index);

                bool isEmpty(uint brick_index);

                void setNeighbour(uint index, NeighbourDirection direction, Entity neighbour);

                void updateBrick(uint index);

                void updateNoiseAndMaterial(uint index);

                void updateTextureBaking(uint index);//TODO move

                template<typename EntityContainer>
                void updateBricks(EntityContainer brick_entities)
                {
                    std::vector<uint> brick_indices;
                    std::vector<uint> empty_bricks;
                    std::vector<uint> brick_iterations;
                    std::vector<uint> empty_brick_iterations;
                    uint max_iterations_cnt = 0;

                    for (auto brick : brick_entities)
                    {
                        uint brick_idx = getIndex(brick);

                        if (!isEmpty(brick_idx))
                        {
                            brick_indices.push_back(brick_idx);
                            brick_iterations.push_back(static_cast<uint>(std::sqrt(m_bricks[brick_idx].m_res_x * m_bricks[brick_idx].m_res_x +
                                m_bricks[brick_idx].m_res_y * m_bricks[brick_idx].m_res_y +
                                m_bricks[brick_idx].m_res_z * m_bricks[brick_idx].m_res_z)));

                            max_iterations_cnt = std::max(max_iterations_cnt, brick_iterations.back());
                        }
                        else
                        {
                            empty_bricks.push_back(brick_idx);
                            empty_brick_iterations.push_back(static_cast<uint>(std::sqrt(m_bricks[brick_idx].m_res_x * m_bricks[brick_idx].m_res_x +
                                m_bricks[brick_idx].m_res_y * m_bricks[brick_idx].m_res_y +
                                m_bricks[brick_idx].m_res_z * m_bricks[brick_idx].m_res_z)));
                        }
                    }

                    // Reset fields of bricks (both non-empty and empty)
                    for (auto brick_idx : brick_indices)
                        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().resetFields(brick_idx); });
                    for (auto brick_idx : empty_bricks)
                        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().resetFields(brick_idx); });

                    // Voxelize Feature Curves in non-empty bricks
                    for (auto brick_idx : brick_indices)
                        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().voxelizeFeatureCurves(brick_idx); });

                    // Compute guidance and noise fields
                    for (uint i = 0; i < max_iterations_cnt / 2; i++)
                    {
                        for (auto brick_idx : brick_indices)
                        {
                            GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeGuidanceField(brick_idx, 2); });
                            GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeNoiseField(brick_idx, 2); });
                        }
                    }
                    //	for (auto brick_idx : brick_indices)
                    //	{
                    //		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeGuidanceField(brick_idx, per_brick_iterations); });
                    //		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeNoiseField(brick_idx, per_brick_iterations); });
                    //	}


                    // Compute surface propagation
                    // Note: surface propagation does 2 iterations internally
                    for (uint i = 0; i < max_iterations_cnt / 2; i++)
                    {
                        for (auto brick_idx : brick_indices)
                        {
                            GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeSurfacePropagation(brick_idx, 1); });
                        }
                    }
                    //for (auto brick_idx : brick_indices)
                    //{
                    //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeSurfacePropagation(brick_idx, per_brick_iterations / 2); });
                    //}

                    // Smooth result of surface propagation
                    for (auto& brick_idx : brick_indices)
                        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().smoothSurfaceField(brick_idx); });

                    // Compute surface mesh
                    for (auto& brick_idx : brick_indices)
                        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeSurfaceMesh(brick_idx); });

                    //for (auto& brick_idx : brick_indices)
                    //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().addDebugVolume(brick_idx);
                    //																						GLandscapeComponents::landscapeManager().setReady(brick_idx); });

                    // Fill bricks without feature curves after all others have been computed
                    for (uint i = 0; i < max_iterations_cnt / 2; i++) //TODO use number of actually required iterations
                    {
                        for (auto brick_idx : empty_bricks)
                        {
                            GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeGuidanceField(brick_idx, 2); });
                            GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeNoiseField(brick_idx, 2); });
                        }
                    }

                    // Note: surface propagation does 2 iterations internally
                    for (uint i = 0; i < max_iterations_cnt / 2; i++) //TODO use number of actually required iterations
                    {
                        for (auto brick_idx : empty_bricks)
                        {
                            GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeSurfacePropagation(brick_idx, 1); });
                        }
                    }

                    for (auto& brick_idx : empty_bricks)
                        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().smoothSurfaceField(brick_idx); });

                    for (auto& brick_idx : empty_bricks)
                        GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx]
                            {
                                GLandscapeComponents::brickManager().computeSurfaceMesh(brick_idx);
                            });

                }

                void updateCorrespondingBricks(Entity modified_feature_curve);

                void exportMesh(uint index, std::string export_filepath);

                Vec3 getDimension(uint index);

                void setResolution(uint index, uint resX, uint resY, uint resZ);

                Vec3 getResolution(uint index);

                void setDebugField(uint index, Datafield field_selection);

                void setSurfaceMaterial(uint index, Material* mat);

                void setVoxelizeFeatureCurves(bool voxelize);
                void setVoxelizeFeatureMeshes(bool voxelize);
                void setVoxelizeHeightmaps(bool voxelize);

                void updateShaderPrograms();
            };

        }
    }
}

#endif // !FeatureCurveComponent_hpp