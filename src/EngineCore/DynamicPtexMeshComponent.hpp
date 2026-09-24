#ifndef DynamicPtexMeshComponent_hpp
#define DynamicPtexMeshComponent_hpp

#include <memory>

#include "BaseSingleInstanceComponentManager2.hpp"
//#include "BaseMultiInstanceComponentManager2.hpp"
#include "BaseResourceManager.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        
        namespace RenderTaskTags {
            struct PtexMesh {};
        }

        struct DynamicPtexMeshComponentData
        {
            //Entity entity_;
            Entity entity;

            // CPU-side storage of Ptex mesh
            std::shared_ptr<std::vector<std::vector<float>>> mesh_vertex_data_;
            std::shared_ptr<std::vector<uint32_t>>           mesh_index_data_;

            // CPU-side storage of Ptex parameters
            struct PtexParameters
            {
                int32_t ngbr_ptex_param_indices[4]; // index into PtexParameters buffers
                int32_t ngbr_uv_transform_cases[4]; // uv transform cases for the 4 direc neighbours

                // Save space by storing index into buffer with all texture handles instead of storing 64bit handles
                // (for texture arrays handle is shared by many faces)
                uint32_t texture_index; // index into texture indirection buffer (where all bindless texture handles are stored)
                uint32_t base_slice; // slice in texture array
            };
            std::shared_ptr<std::vector<PtexParameters>> ptex_params_;

            
            size_t            lod_lvls_; // Number of details levels for texture tiles
            std::vector<uint> lod_bin_sizes_; // store number of texture tiles per LOD level
            
            std::vector<uint32_t> updatePatches_tgt_;
            std::vector<size_t> update_bin_sizes_;

            struct PatchInfo
            {
                Vec3   midpoint;
                float  distance;
                size_t lod_classification; // store LOD classification of patches
            };
            std::vector<PatchInfo> patch_info_;
            

            struct TextureSlot
            {
                uint32_t tex_idx;
                uint32_t base_slice;
            };
            std::vector<std::vector<TextureSlot>> availableTiles_; // texture tiles available for assignment grouped by lod level
            std::vector<TextureSlot>              availableTiles_uploadBuffer_; // linearized vector of available tiles
            std::vector<size_t>                   availableTiles_indexOffsets_; // index offsets for lod levels

            std::vector<TextureSlot> vistaTiles_;

            //GPU resourcecs
            ResourceID mesh_;
            ResourceID shader_;

            ResourceID ptex_params_buffer_;
            ResourceID bindless_texture_handles_;
            ResourceID bindless_image_handles_;
            ResourceID bindless_mipmap_image_handles_;


            //Update flags to signal the rendering pipeline if texture need to be updated
        };

        class DynamicPtexMeshComponentManager : public BaseSingleInstanceComponentManager2<DynamicPtexMeshComponentData, 10, 10>
        {
        public:
            //	/** Add component using existing GPU resources */
            //	size_t addComponent(Entity entity, uint32_t texture_tiles_cnt, uint32_t patch_cnt, ResourceID mesh, ResourceID material, ResourceID ptex_parameters, ResourceID ptex_bindless_texture_handles);
            //	
            //	void setPtexParameters(Entity entity, ResourceID ptex_params);
            //	
            //	ResourceID getMesh(uint index);
            //	
            //	ResourceID getMaterial(uint index);
            //	
            //	ResourceID getPtexParameters(uint index);
            //	
            //	ResourceID getPtexBindTextureHandles(uint index);
        };
    }
}

#endif // !DynamicPtexMeshComponent_hpp
