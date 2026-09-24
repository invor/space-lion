#ifndef DynamicPtexMeshSystems_hpp
#define DynamicPtexMeshSystems_hpp

#include <array>
#include <assert.h>
#include <memory>
#include <numeric>
#include <span>
#include <string>
#include <vector>

#include "okayply.h"

#include "BaseComponentManager.hpp"
#include "DynamicPtexMeshComponent.hpp"
#include "utility.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        namespace
        {
            typedef std::shared_ptr<std::vector<GenericVertexLayout>> PlyVertexLayoutPtr;
            typedef std::shared_ptr<std::vector<std::vector<float>>>  PlyVertexDataPtr;
            typedef std::shared_ptr<std::vector<uint32_t>>            PlyIndexDataPtr;

            inline std::tuple<PlyVertexLayoutPtr, PlyVertexDataPtr, PlyIndexDataPtr>
                loadPlyMeshData(std::string const& filepath)
            {
                // create return values
                PlyVertexLayoutPtr vertex_layout = std::make_shared<std::vector<GenericVertexLayout>>();
                vertex_layout->push_back(GenericVertexLayout(12, { GenericVertexLayout::Attribute(3, 5126, false, 0) }));

                PlyVertexDataPtr vertex_data = std::make_shared<std::vector<std::vector<float>>>();
                vertex_data->emplace_back(std::vector<float>());
                vertex_data->shrink_to_fit();

                PlyIndexDataPtr index_data = std::make_shared<std::vector<uint32_t>>();

                // intermediate data storage for non-interleaved vertex positions
                std::span<float> x_data;
                std::span<float> y_data;
                std::span<float> z_data;

                okayply::root ply_root;
                ply_root.read(filepath);

                // Access all elements & properties without bloat.
                for (auto const& element_ref : ply_root.elements())
                {
                    auto& element = element_ref.get();
                    //std::cout << "\nELEMENT\n";
                    //std::cout << "name: " << element.name() << ", ptr: " << &element << ", size: " << element.size() << "\n";
                    //std::cout << "PROPERTY\n";
                    for (auto const& property_ref : element.properties())
                    {
                        auto& property = property_ref.get();

                        if (property.type() == typeid(float) && property.name() == "x")
                        {
                            x_data = property.get<float>();
                        }
                        else if (property.type() == typeid(float) && property.name() == "y")
                        {
                            y_data = property.get<float>();
                        }
                        else if (property.type() == typeid(float) && property.name() == "z")
                        {
                            z_data = property.get<float>();
                        }
                        else if (property.listType() == typeid(std::vector<uint32_t>) || property.listType() == typeid(std::vector<uint16_t>))
                        {
                            auto data = property.get<std::vector<uint32_t>>();
                            for (auto const& v : data)
                            {
                                assert(v.size() == 4);
                                index_data->push_back(v[0]);
                                index_data->push_back(v[1]);
                                index_data->push_back(v[2]);
                                index_data->push_back(v[3]);
                            }
                        }
                    }
                }

                assert(x_data.size() == y_data.size());
                assert(y_data.size() == z_data.size());

                float min_x = std::numeric_limits<float>::max();
                float max_x = std::numeric_limits<float>::min();

                vertex_data->front().reserve(x_data.size() * 3);
                for (size_t i = 0; i < x_data.size(); ++i)
                {
                    vertex_data->front().push_back(x_data[i]);
                    vertex_data->front().push_back(y_data[i]);
                    vertex_data->front().push_back(z_data[i]);

                    min_x = std::min(x_data[i], min_x);
                    max_x = std::max(x_data[i], max_x);
                }

                std::cout << "x min: " << min_x << std::endl;
                std::cout << "x max: " << max_x << std::endl << std::endl;

                return { vertex_layout, vertex_data, index_data };
            }

            struct PatchNeighbourInfo
            {
                std::array<int32_t, 4> neighbours;
                std::array<int32_t, 4> uv_transform_case;

                size_t neighbours_found = 0;
            };
            inline std::shared_ptr < std::vector<PatchNeighbourInfo>>
                computePatchNeighbourhoodInfo(PlyIndexDataPtr indices)
            {
                size_t patch_cnt = indices->size() / 4;

                std::shared_ptr<std::vector<PatchNeighbourInfo>> patch_info
                    = std::make_shared<std::vector<PatchNeighbourInfo>>(patch_cnt);

                for (size_t patch_idx = 0; patch_idx < patch_cnt; ++patch_idx)
                {
                    if ((*patch_info)[patch_idx].neighbours_found == 4)
                    {
                        continue;
                    }

                    std::array<uint32_t, 4> mesh_indices = {
                            indices->at(patch_idx * 4),
                            indices->at(patch_idx * 4 + 1),
                            indices->at(patch_idx * 4 + 2),
                            indices->at(patch_idx * 4 + 3)
                    };

                    // compare against remaining patches to find neighbours
                    for (size_t cmp_patch_idx = patch_idx + 1; cmp_patch_idx < patch_cnt; ++cmp_patch_idx)
                    {
                        if ((*patch_info)[cmp_patch_idx].neighbours_found == 4)
                        {
                            continue;
                        }

                        std::array<uint32_t, 4> cmp_mesh_indices = {
                            indices->at(cmp_patch_idx * 4),
                            indices->at(cmp_patch_idx * 4 + 1),
                            indices->at(cmp_patch_idx * 4 + 2),
                            indices->at(cmp_patch_idx * 4 + 3)
                        };


                        for (size_t comparison_case = 0; comparison_case < 16; ++comparison_case)
                        {
                            size_t edge_idx = comparison_case / 4;
                            size_t cmp_edge_idx = comparison_case % 4;

                            // get mesh indices for edge vertices
                            uint32_t ev0 = mesh_indices[edge_idx];
                            uint32_t ev1 = mesh_indices[(edge_idx + 1) % 4];

                            uint32_t cmp_ev0 = cmp_mesh_indices[cmp_edge_idx];
                            uint32_t cmp_ev1 = cmp_mesh_indices[(cmp_edge_idx + 1) % 4];

                            // if patch vertices are always ccw, vertices of shared edges will be reversed
                            if (ev0 == cmp_ev1 && ev1 == cmp_ev0)
                            {
                                // found shared edge, enter neighboor into both patches parameters
                                (*patch_info)[patch_idx].neighbours[edge_idx] = cmp_patch_idx;
                                (*patch_info)[cmp_patch_idx].neighbours[cmp_edge_idx] = patch_idx;

                                // indentify uv transform case between neighboors
                                (*patch_info)[patch_idx].uv_transform_case[edge_idx] = (edge_idx * 4) + cmp_edge_idx;
                                (*patch_info)[cmp_patch_idx].uv_transform_case[cmp_edge_idx] = (cmp_edge_idx * 4) + edge_idx; //is it really this simple?

                                (*patch_info)[patch_idx].neighbours_found += 1;
                                (*patch_info)[cmp_patch_idx].neighbours_found += 1;

                                break;
                            }
                        }

                        if ((*patch_info)[patch_idx].neighbours_found == 4)
                        {
                            break;
                        }

                        //for (size_t edge_idx = 0; edge_idx < 4; ++edge_idx)
                        //{
                        //    // get mesh indices for edge vertices
                        //    uint32_t ev0 = mesh_indices[edge_idx];
                        //    uint32_t ev1 = mesh_indices[(edge_idx + 1) % 4];
                        //
                        //    for (size_t cmp_edge_idx = 0; cmp_edge_idx < 4; ++cmp_edge_idx)
                        //    {
                        //        uint32_t cmp_ev0 = cmp_mesh_indices[cmp_edge_idx];
                        //        uint32_t cmp_ev1 = cmp_mesh_indices[(cmp_edge_idx + 1) % 4];
                        //
                        //        // if patch vertices are always ccw, vertices of shared edges will be reversed
                        //        if (ev0 == cmp_ev1 && ev1 == cmp_ev0)
                        //        {
                        //            // found shared edge, enter neighboor into both patches parameters
                        //            (*patch_info)[patch_idx].neighbours[edge_idx] = cmp_patch_idx;
                        //            (*patch_info)[cmp_patch_idx].neighbours[cmp_edge_idx] = patch_idx;
                        //
                        //            // indentify uv transform case between neighboors
                        //            (*patch_info)[patch_idx].uv_transform_case[edge_idx] = (edge_idx * 4) + cmp_edge_idx;
                        //            (*patch_info)[cmp_patch_idx].uv_transform_case[cmp_edge_idx] = (cmp_edge_idx * 4) + edge_idx; //is it really this simple?
                        //        }
                        //    }
                        //}
                    }
                }

                return patch_info;
            }
        }

        inline void computePatchDistances(
            EngineCore::Common::TransformComponentManager& transform_mngr,
            EngineCore::Graphics::CameraComponentManager& camera_mngr,
            EngineCore::Graphics::DynamicPtexMeshComponentManager& ptex_mngr,
            Utility::TaskScheduler& task_scheduler,
            Entity entity)
        {
            //TODO ptex component not thread safe like this...
            auto ptex_idx = ptex_mngr.getIndex(entity);
            if (ptex_idx == EngineCore::Utility::SingleInstanceIndexMap::invalidIndex())
            {
                return;
            }

            auto& ptex_component = ptex_mngr.getComponent(ptex_idx);

            size_t quad_cnt = ptex_component.ptex_params_->size();




            std::vector<std::pair<size_t, size_t>> from_to_pairs = utility::buildComponentProcessingRanges(quad_cnt, 12);


            for (auto from_to : from_to_pairs) {
                task_scheduler.submitTask(
                    [&transform_mngr, &camera_mngr, &ptex_component, entity, from_to]() {

                        // get ptex mesh transform
                        Mat4x4 mesh_model_mx = transform_mngr.getWorldTransformation(transform_mngr.getIndex(entity));

                        // get camera transform
                        Entity camera_entity = camera_mngr.getActiveCamera();
                        Mat4x4 view_mx = glm::inverse(transform_mngr.getWorldTransformation(transform_mngr.getIndex(camera_entity)));
                        Mat4x4 proj_mx = camera_mngr.getProjectionMatrix(camera_mngr.getIndex(camera_entity).front());
                        Mat4x4 view_proj_mx = proj_mx * view_mx;

                        Vec3 camera_position = Vec3(glm::inverse(view_mx) * Vec4(0.0, 0.0, 0.0, 1.0));

                        for (size_t quad_idx = from_to.first; quad_idx < from_to.second; ++quad_idx)
                        {
                            uint32_t patch_indices[4];
                            Vec4 patch_vertices[4];

                            Vec3 midpoint(0.0f);

                            for (int i = 0; i < 4; ++i)
                            {
                                patch_indices[i] = (*ptex_component.mesh_index_data_)[quad_idx * 4 + i];

                                //TODO make the vertex attribute count generic
                                patch_vertices[i].x = ptex_component.mesh_vertex_data_->front()[patch_indices[i] * 3 + 0];
                                patch_vertices[i].y = ptex_component.mesh_vertex_data_->front()[patch_indices[i] * 3 + 1];
                                patch_vertices[i].z = ptex_component.mesh_vertex_data_->front()[patch_indices[i] * 3 + 2];
                                patch_vertices[i].w = 1.0;

                                patch_vertices[i] = mesh_model_mx * patch_vertices[i];

                                midpoint += Vec3(patch_vertices[i]);
                            }

                            midpoint = midpoint / 4.0f;

                            //camera_position.x = std::floor(camera_position.x / 4.0f) * 4.0f;
                            //camera_position.y = std::floor(camera_position.y / 4.0f) * 4.0f;
                            //camera_position.z = std::floor(camera_position.z / 4.0f) * 4.0f;
                            ptex_component.patch_info_[quad_idx].midpoint = midpoint;
                            ptex_component.patch_info_[quad_idx].distance = glm::length(camera_position - midpoint);

                            //std::cout << "Distance: " << glm::length(camera_position - midpoint) << std::endl;

                            // "Frustrum" culling
                            bool frustrum_test[4] = { false,false,false,false };

                            for (int i = 0; i < 4; ++i)
                            {
                                // Transform patch vertices to clip space
                                //vec4 cs_pos = proj_mx * view_mx * model_mx * patch_vertices[i];

                                Vec4 cs_pos = view_proj_mx * patch_vertices[i];

                                cs_pos.w = cs_pos.w * 1.25;

                                // Test against frustrum, if outside set very large distance
                                if (cs_pos.x < -cs_pos.w || cs_pos.x > cs_pos.w)
                                    frustrum_test[i] = true;

                                if (cs_pos.y < -cs_pos.w || cs_pos.y > cs_pos.w)
                                    frustrum_test[i] = true;

                                if (cs_pos.z < -cs_pos.w || cs_pos.z > cs_pos.w)
                                    frustrum_test[i] = true;
                            }

                            //TODO debug and reenable frustrum test
                            if (frustrum_test[0] && frustrum_test[1] && frustrum_test[2] && frustrum_test[3])
                            {
                                ptex_component.patch_info_[quad_idx].distance = 9999.0;
                            }
                        }
                    }
                );
            }

            task_scheduler.waitWhileBusy();
        }

        //TODO (CPU-side) computation of update tiles after computing patch distances
        inline void computeTextureTileUpdateLists(
            EngineCore::Graphics::DynamicPtexMeshComponentManager& ptex_mngr,
            Entity entity)
        {
            auto& ptex_component = ptex_mngr.getComponent(ptex_mngr.getIndex(entity));

            // sort distance values, keep track of original indices
            size_t patch_cnt = ptex_component.patch_info_.size();
            int max_lod_lvl = ptex_component.lod_bin_sizes_.size() - 1;

            // initialize original index locations
            std::vector<size_t> patch_indices(patch_cnt);
            std::iota(patch_indices.begin(), patch_indices.end(), 0);

            // sort indexes based on comparing patch distance values
            std::sort(patch_indices.begin(), patch_indices.end(),
                [&ptex_component](size_t i1, size_t i2) {return ptex_component.patch_info_[i1].distance < ptex_component.patch_info_[i2].distance; });

            // DEBUGING
            //for (int i=0; i< patch_indices.size(); ++i)
            //{
            //	std::cout << ptex_component.patch_distances[patch_indices[i]] << " : " << patch_indices[i] << std::endl;
            //}

            // classify distance values based on LOD bin sizes
            std::vector<int> classification(patch_cnt);
            size_t remaining_patches = patch_cnt;

            //std::vector<float> lod_distance_steps({ 11.5f,23.0f,45.0f,88.0f,175.0f,999999.0f }); // values measured for optimal mipmap level
            //std::vector<float> lod_distance_steps({ 12.0f,24.0f,48.0f,96.0f,192.0f,999999.0f }); // values measured for optimal mipmap level
            std::vector<float> lod_distance_steps({ 3.5f,4.5f,5.5f,7.0f,9.0f,999999.0f }); // values measured for optimal mipmap level
            std::vector<uint> remaining_lod_bin_size = ptex_component.lod_bin_sizes_;

            int lod_bin = 0;
            for (auto patch_idx : patch_indices)
            {
                while ((remaining_lod_bin_size[lod_bin] == 0) || (ptex_component.patch_info_[patch_idx].distance >= lod_distance_steps[lod_bin]))
                {
                    if (lod_bin == max_lod_lvl)
                        break;

                    ++lod_bin;
                }

                classification[patch_idx] = lod_bin;

                remaining_lod_bin_size[lod_bin] = remaining_lod_bin_size[lod_bin] - 1;
            }

            // compare classification with previous classification
            size_t update_patches_cnt = 0;
            std::vector<std::vector<uint32_t>> update_patches_tgt(ptex_component.lod_bin_sizes_.size());

            // Vista LoD doesnt need to be included for freed and available slots
            std::vector<std::vector<DynamicPtexMeshComponentData::TextureSlot>> freed_slots(ptex_component.availableTiles_.size());

            for (size_t patch_idx = 0; patch_idx < classification.size(); ++patch_idx)
            {
                size_t previous_classification = ptex_component.patch_info_[patch_idx].lod_classification;

                if (classification[patch_idx] != previous_classification && update_patches_tgt[classification[patch_idx]].size() < 65535u)
                //if (classification[patch_idx] != previous_classification)
                {
                    update_patches_tgt[classification[patch_idx]].push_back(patch_idx); // add patch to update list with new classification

                    if (previous_classification != max_lod_lvl) // if previously not vista LOD add to available texture slots
                    {
                        DynamicPtexMeshComponentData::TextureSlot free_slot;
                        free_slot.tex_idx = (*ptex_component.ptex_params_)[patch_idx].texture_index;
                        free_slot.base_slice = (*ptex_component.ptex_params_)[patch_idx].base_slice;
                        //ptex_component.latest_availableTiles[previous_classification].push_back(free_slot);
                        freed_slots[previous_classification].push_back(free_slot);

                        //TODO assert tex index fits bin index
                    }

                    ptex_component.patch_info_[patch_idx].lod_classification = classification[patch_idx]; // update stored classification

                    ++update_patches_cnt;
                }
            }

            // copy update patches to continous memory
            ptex_component.updatePatches_tgt_.clear();
            ptex_component.updatePatches_tgt_.resize(update_patches_cnt);

            ptex_component.update_bin_sizes_.clear();
            //ptex_component.update_bin_sizes.reserve(ptex_component.lod_bin_sizes.size());

            std::ostringstream bin_size_log;

            size_t tgt_copied_elements = 0;
            for (size_t i = 0; i < ptex_component.lod_bin_sizes_.size(); ++i)
            {
                std::copy(update_patches_tgt[i].begin(), update_patches_tgt[i].end(), ptex_component.updatePatches_tgt_.begin() + tgt_copied_elements);

                tgt_copied_elements += update_patches_tgt[i].size();

                bin_size_log << "Update bin size:" << update_patches_tgt[i].size() << std::endl;

                ptex_component.update_bin_sizes_.push_back(update_patches_tgt[i].size());
            }

            // Sort both available and newly freed texture tile slots independent and append freed slots to available slot lists afterwards
            for (int i = 0; i < freed_slots.size(); ++i)
            {
                std::sort(ptex_component.availableTiles_[i].begin(), ptex_component.availableTiles_[i].end(),
                    [](const DynamicPtexMeshComponentData::TextureSlot& a, const DynamicPtexMeshComponentData::TextureSlot& b) -> bool
                    {
                        return (a.tex_idx != b.tex_idx) ? (a.tex_idx > b.tex_idx) : (a.base_slice > b.base_slice);
                    });

                std::sort(freed_slots[i].begin(), freed_slots[i].end(),
                    [](const DynamicPtexMeshComponentData::TextureSlot& a, const DynamicPtexMeshComponentData::TextureSlot& b) -> bool
                    {
                        return (a.tex_idx != b.tex_idx) ? (a.tex_idx > b.tex_idx) : (a.base_slice > b.base_slice);
                    });

                ptex_component.availableTiles_[i].append_range(freed_slots[i]);
            }

            size_t available_tiles_cnt = 0;
            for (auto& tile_list : ptex_component.availableTiles_)
                available_tiles_cnt += tile_list.size();

            ptex_component.availableTiles_uploadBuffer_.clear();
            ptex_component.availableTiles_uploadBuffer_.resize(available_tiles_cnt);
            ptex_component.availableTiles_indexOffsets_.clear();

            std::ostringstream available_tiles_log;

            size_t copied_elements = 0;
            for (size_t i = 0; i < ptex_component.availableTiles_.size(); ++i)
            {
                bin_size_log << "Available tiles:" << ptex_component.availableTiles_[i].size() << std::endl;

                std::copy(ptex_component.availableTiles_[i].begin(), ptex_component.availableTiles_[i].end(), ptex_component.availableTiles_uploadBuffer_.begin() + copied_elements);

                copied_elements += ptex_component.availableTiles_[i].size();

                //available_tiles_log << "Available tiles:" << ptex_component.availableTiles[i].size() << std::endl;

                ptex_component.availableTiles_indexOffsets_.push_back(copied_elements);
            }

            if (tgt_copied_elements > 0)
                std::cout << bin_size_log.str() << available_tiles_log.str();


            //  // assign tiles on the CPU for debugging (without updating the texture content)
            //  std::vector<unsigned int> assigned_tiles_per_level(update_patches_tgt.size());
            //  for (size_t lod_lvl = 0; lod_lvl < (update_patches_tgt.size() - 1); ++lod_lvl)
            //  {
            //      for (size_t update_patch_idx = 0; update_patch_idx < update_patches_tgt[lod_lvl].size(); ++update_patch_idx)
            //      {
            //          unsigned int patch_idx = update_patches_tgt[lod_lvl][update_patch_idx];
            //  
            //          (*ptex_component.ptex_params_)[patch_idx].texture_index
            //              = ptex_component.availableTiles_[lod_lvl][assigned_tiles_per_level[lod_lvl]].tex_idx;
            //          (*ptex_component.ptex_params_)[patch_idx].base_slice
            //              = ptex_component.availableTiles_[lod_lvl][assigned_tiles_per_level[lod_lvl]].base_slice;
            //  
            //          assigned_tiles_per_level[lod_lvl] += 1;
            //      }
            //  }
            //  // assign vista level seperately as it is not part of the available tiles collection (since direct mapping between vista tiles and primitives exits)
            //  for (size_t update_patch_idx = 0; update_patch_idx < update_patches_tgt.back().size(); ++update_patch_idx)
            //  {
            //      unsigned int patch_idx = update_patches_tgt.back()[update_patch_idx];
            //  
            //      (*ptex_component.ptex_params_)[patch_idx].texture_index
            //          = ptex_component.vistaTiles_[assigned_tiles_per_level.back()].tex_idx;
            //      (*ptex_component.ptex_params_)[patch_idx].base_slice
            //          = ptex_component.vistaTiles_[assigned_tiles_per_level.back()].base_slice;
            //  
            //      assigned_tiles_per_level.back() += 1;
            //  }

            // After available texture tiles are copied to upload buffer, remove as many as will be used by update patches
            for (size_t i = 0; i < ptex_component.availableTiles_.size(); ++i)
            {
                auto it1 = ptex_component.availableTiles_[i].begin();
                auto it2 = it1 + ptex_component.update_bin_sizes_[i];
                ptex_component.availableTiles_[i].erase(it1, it2);
            }

            //TODO THREAD SAFETY 

            ptex_component.updated_primitives_ += tgt_copied_elements;
        }

        template<typename ResourceManagerType>
        inline void allocatePtexTextureTiles(
            ResourceManagerType& resource_manager,
            EngineCore::Graphics::MaterialComponentManager& mtl_mngr,
            EngineCore::Graphics::DynamicPtexMeshComponentManager& ptex_mngr,
            Entity entity)
        {
            auto& ptex_component = ptex_mngr.getComponent(ptex_mngr.getIndex(entity));

            uint primitive_cnt = ptex_component.mesh_index_data_->size() / 4; //Quad primitives -> 4 indices per primitive

            //TODO set layers according to how many texture make up the material defintion
            int layers = 2048;

            // TODO query GPU vendor and subsequently the available video memory to make an assumption about how much memory I want to spend on Ptex textures
            size_t available_ptex_memory = 1500000000; // GPU memory available for ptex textures given in byte
            size_t used_ptex_memory = 0;
            size_t texture_array_cnt = 0;

            // create vista texture tiles (8x8) for all surface patches
            size_t vista_tiles_array_cnt = static_cast<size_t>(std::ceil(static_cast<float>(primitive_cnt) / static_cast<float>(layers)));
            size_t vista_tiles_memory = vista_tiles_array_cnt * layers * (8 * 8) * 4;
            used_ptex_memory += vista_tiles_memory;
            texture_array_cnt += vista_tiles_array_cnt;

            // if more than ~0.5GB of available memory left, use 256x256 detail tiles
            ptex_component.lod_lvls_ = ((available_ptex_memory - used_ptex_memory) > 1000000000) ? 6 : 5;
            ptex_component.lod_bin_sizes_.resize(ptex_component.lod_lvls_);

            // create patch info storage and initialize lod classification to vista level
            ptex_component.patch_info_ = std::vector<DynamicPtexMeshComponentData::PatchInfo>(primitive_cnt);
            for (auto& pi : ptex_component.patch_info_) { pi.lod_classification = ptex_component.lod_lvls_ - 1; }

            // use one set, i.e. one texture array, of detail tiles
            size_t detail_tiles_memory = layers * static_cast<size_t>(std::pow(2, ptex_component.lod_lvls_ + 2)) * static_cast<size_t>(std::pow(2, ptex_component.lod_lvls_ + 2)) * 4;
            used_ptex_memory += detail_tiles_memory;
            texture_array_cnt += 1;
            ptex_component.lod_bin_sizes_[0] = layers;

            assert(available_ptex_memory > used_ptex_memory);

            // split remaining memory evenly among remaining lod levels
            size_t memory_per_lvl = static_cast<size_t>(std::floor(((available_ptex_memory - used_ptex_memory) / (ptex_component.lod_lvls_ - 2))));

            for (int i = 1; i < ptex_component.lod_lvls_ - 1; ++i) // skip detail and vista level
            {
                // compute number of texture arrays per level based on available memory per level
                size_t array_memory = layers * (std::pow(2, (ptex_component.lod_lvls_ + 2) - i) * std::pow(2, (ptex_component.lod_lvls_ + 2) - i)) * 4;
                size_t array_cnt = std::max(static_cast<size_t>(1), static_cast<size_t>(std::floor(memory_per_lvl / array_memory)));
                ptex_component.lod_bin_sizes_[i] = (array_cnt * layers);
                used_ptex_memory += array_memory * array_cnt;
                texture_array_cnt += array_cnt;
            }

            ptex_component.lod_bin_sizes_[ptex_component.lod_lvls_ - 1] = vista_tiles_array_cnt * layers;

            // clear current update target tiles (basically cancel current update) as it will no longer match the surface mesh
            ptex_component.updatePatches_tgt_.clear();

            // intialize available tiles (at the beginning, all tiles on all levels are available). exclude vista layer
            ptex_component.availableTiles_.resize(ptex_component.lod_lvls_ - 1);
            uint32_t assigned_tiles = 0;
            for (int i = 0; i < ptex_component.lod_lvls_ - 1; ++i)
            {
                ptex_component.availableTiles_[i].resize(ptex_component.lod_bin_sizes_[i]);
                //std::iota(ptex_component.availableTiles[i].begin(), ptex_component.availableTiles[i].end(), 0);
                for (auto& slot : ptex_component.availableTiles_[i])
                {
                    DynamicPtexMeshComponentData::TextureSlot new_slot;
                    new_slot.tex_idx = (assigned_tiles / layers);
                    new_slot.base_slice = assigned_tiles % 2048;
                    slot = new_slot;

                    assigned_tiles += 1;
                }
            }

            // CPU-side storage of vista tiles
            ptex_component.vistaTiles_.resize(ptex_component.lod_bin_sizes_[ptex_component.lod_lvls_ - 1]);
            for (auto& slot : ptex_component.vistaTiles_)
            {
                DynamicPtexMeshComponentData::TextureSlot new_slot;
                new_slot.tex_idx = (assigned_tiles / layers);
                new_slot.base_slice = assigned_tiles % 2048;
                slot = new_slot;

                assigned_tiles += 1;
            }

            //std::cout << "Num texture arrays: " << texture_array_cnt << std::endl;
            //std::cout << "Num layers per array: " << layers << std::endl;
            //std::cout << "Num lod levels: " << ptex_component.lod_lvls_ << std::endl;

            auto& textures = mtl_mngr.getComponent(mtl_mngr.getIndex(entity).front()).textures;
            textures.clear();
            textures.reserve(texture_array_cnt);

            //std::vector<std::vector<uint8_t>> debug_image_data(lod_lvls);

            std::array<std::array<uint8_t, 4>, 7> lod_colours =
            { {
                {{78,121,167,255}},
                {{242,142,42,255}},
                {{225,87,89,255}},
                {{118,183,178,255}},
                {{89,161,79,255}},
                {{237,201,72,255}},
                {{176,122,161,255}}
            } };

            for (size_t i = 0; i < ptex_component.lod_lvls_; ++i)
            {
                GenericTextureLayout tile_layout;
                tile_layout.internal_format = GenericTextureLayout::InternalFormat::RGBA8;
                tile_layout.format = GL_RGBA;
                tile_layout.type = GL_UNSIGNED_BYTE;
                tile_layout.width = static_cast<int>(std::pow(2, (ptex_component.lod_lvls_ + 2) - i)); //start at 8
                tile_layout.height = static_cast<int>(std::pow(2, (ptex_component.lod_lvls_ + 2) - i)); // times 8
                tile_layout.depth = layers;
                tile_layout.levels = 2;

                tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER });
                //tile_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER, GL_LINEAR });

                auto api_layout = resource_manager.convertGenericTextureLayout(tile_layout);

                std::shared_ptr<std::vector<uint8_t>> debug_image_data =
                    std::make_shared<std::vector<uint8_t>>(tile_layout.width * tile_layout.height * tile_layout.depth * 4);

                for (size_t pixel_idx = 0; pixel_idx < (tile_layout.width * tile_layout.height * tile_layout.depth); ++pixel_idx)
                {
                    (*debug_image_data)[pixel_idx * 4 + 0] = lod_colours[i][0];
                    (*debug_image_data)[pixel_idx * 4 + 1] = lod_colours[i][1];
                    (*debug_image_data)[pixel_idx * 4 + 2] = lod_colours[i][2];
                    (*debug_image_data)[pixel_idx * 4 + 3] = lod_colours[i][3];
                }

                size_t array_cnt = (ptex_component.lod_bin_sizes_[i]) / layers;
                for (size_t j = 0; j < array_cnt; ++j)
                {
                    std::string texture_identifier = "ptex_lvl_" + std::to_string(i) + "_array_ " + std::to_string(j);
                    auto tx_rsrcID = resource_manager.createTexture2DArrayAsync(texture_identifier, api_layout, debug_image_data, false);
                    textures.push_back({ MaterialComponentData::TextureSemantic::ALBEDO, tx_rsrcID });
                }
            }

            /*
            std::vector<uint8_t> image_data;
            TextureLayout image_layout;
            //ResourceLoading::loadPpmImageRGBA("../resources/textures/debug_uv_tile_64x64.ppm", image_data, image_layout);

            image_layout.internal_format = GL_RGBA8;
            image_layout.format = GL_RGBA;
            image_layout.type = GL_UNSIGNED_BYTE;
            image_layout.width = 16;
            image_layout.height = 16;
            image_layout.depth = layers;

            //std::vector<uint8_t> array_dummy_data(image_data.size()*layers, 255);
            //for (int i = 0; i < layers; i++)
            //{
            //	std::copy(image_data.data(), image_data.data() + image_data.size(), array_dummy_data.data() + (i * image_data.size()));
            //}
            */

            // bake lod texture tiles (intial lod baking differs from updating tiles..)
            //ptex_component.ready = true;
            //updateTextureBaking(index);

            //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
            //{
            //	std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
            //
            //computePatchDistances(index);
            //
            //GEngineCore::taskSchedueler().submitTask([this, index]() {
            //
            //	std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
            //
            //	computeTextureTileUpdateList(index);
            //
            //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
            //	{
            //		std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
            //		updateTextureTiles(index);
            //	});
            //
            //});
            //});
        }

        //TODO update texture tiles function (GPU, probably should be called duríng the resource setup step in the rendering pipeline)

        template<typename ResourceManagerType>
        inline Entity createPtexMesh(
            EntityManager& entity_mngr,
            ResourceManagerType& resource_mngr,
            EngineCore::Graphics::CameraComponentManager& camera_mngr,
            EngineCore::Graphics::MaterialComponentManager& mtl_mngr,
            EngineCore::Graphics::DynamicPtexMeshComponentManager& ptex_mngr,
            EngineCore::Graphics::RenderTaskComponentManager<RenderTaskTags::PtexMesh>& renderTask_mngr,
            EngineCore::Common::TransformComponentManager& transform_mngr,
            std::string const& filepath)
        {
            // load mesh data from ply file
            auto mesh_data = loadPlyMeshData(filepath);
            // compute neighbourhood information for the quad patches of the mesh
            auto patch_info = computePatchNeighbourhoodInfo(std::get<2>(mesh_data));

            DynamicPtexMeshComponentData ptex_component_data;

            ptex_component_data.entity = entity_mngr.create();
            ptex_component_data.mesh_vertex_data_ = std::get<1>(mesh_data);
            ptex_component_data.mesh_index_data_ = std::get<2>(mesh_data);

            ptex_component_data.ptex_params_ = std::make_shared<std::vector<DynamicPtexMeshComponentData::PtexParameters>>();
            ptex_component_data.ptex_params_->reserve(std::get<2>(mesh_data)->size() / 4);
            for (auto& pi : (*patch_info))
            {
                DynamicPtexMeshComponentData::PtexParameters params;
                params.ngbr_ptex_param_indices[0] = pi.neighbours[0];
                params.ngbr_ptex_param_indices[1] = pi.neighbours[1];
                params.ngbr_ptex_param_indices[2] = pi.neighbours[2];
                params.ngbr_ptex_param_indices[3] = pi.neighbours[3];
                params.ngbr_uv_transform_cases[0] = pi.uv_transform_case[0];
                params.ngbr_uv_transform_cases[1] = pi.uv_transform_case[1];
                params.ngbr_uv_transform_cases[2] = pi.uv_transform_case[2];
                params.ngbr_uv_transform_cases[3] = pi.uv_transform_case[3];

                params.texture_index = 0;
                params.base_slice = 0;

                ptex_component_data.ptex_params_->emplace_back(std::move(params));
            }
            // create per patch parameter buffer
            // side note: buffer creation in OpenGL and DX currently doesn't match up yet
            auto ptex_params_rsrcID = resource_mngr.createBufferObjectAsync(
                filepath + "_ptex_paramas",
                0x90D2,//GL_SHADER_STORAGE_BUFFER
                ptex_component_data.ptex_params_
            );
            ptex_component_data.ptex_params_buffer_ = ptex_params_rsrcID;

            // create shader for rendering the ptex surface
            std::string shader_root = "../HeatmapVisualization/shaders/";
            auto ptex_mesh_shader_names = std::make_shared<std::vector<EngineCore::Graphics::OpenGL::ResourceManager::ShaderFilename>>(
                std::initializer_list<EngineCore::Graphics::OpenGL::ResourceManager::ShaderFilename>{
                    { shader_root + "ptex_v.glsl", glowl::GLSLProgram::ShaderType::Vertex },
                    { shader_root + "ptex_tc.glsl", glowl::GLSLProgram::ShaderType::TessControl },
                    { shader_root + "ptex_te.glsl", glowl::GLSLProgram::ShaderType::TessEvaluation },
                    { shader_root + "ptex_f.glsl", glowl::GLSLProgram::ShaderType::Fragment }
            });
            auto ptex_mesh_shader_rsrc = resource_mngr.createShaderProgramAsync(
                "ptex_mesh_shader",
                ptex_mesh_shader_names
            );
            auto mtl_idx = mtl_mngr.addComponent(ptex_component_data.entity, "ptex_mesh_material", ptex_mesh_shader_rsrc);
            ptex_component_data.shader_ = ptex_mesh_shader_rsrc;

            // convert generic vertex layout
            std::shared_ptr<std::vector<typename ResourceManagerType::VertexLayout>> vertex_layouts
                = std::make_shared<std::vector<typename ResourceManagerType::VertexLayout>>();
            unsigned int base_input_slot = 0;
            for (auto& generic_vertex_layout : (*std::get<0>(mesh_data)))
            {
                vertex_layouts->push_back(resource_mngr.convertGenericGltfVertexLayout(generic_vertex_layout, base_input_slot));
                ++base_input_slot;
            }
            // create GPU mesh
            auto mesh_rsrcID = resource_mngr.allocateMeshAsync(
                filepath,
                std::get<1>(mesh_data)->front().size(),
                std::get<2>(mesh_data)->size(),
                vertex_layouts,
                0x1405, //GL_UNSIGNED_INT
                0x000E //GL_PATCHES
            );
            resource_mngr.updateMeshAsync(
                mesh_rsrcID,
                0,
                0,
                std::get<1>(mesh_data),
                std::get<2>(mesh_data)
            );
            ptex_component_data.mesh_ = mesh_rsrcID;

            auto empty_data = std::make_shared<std::vector<uint8_t>>(0);

            // create buffers holding different texture handles
            auto bindless_texture_handles_rsrcID = resource_mngr.createBufferObjectAsync(
                filepath + "_ptex_bindless_texture_handles",
                0x90D2,//GL_SHADER_STORAGE_BUFFER
                empty_data
            );
            ptex_component_data.bindless_texture_handles_ = bindless_texture_handles_rsrcID;

            auto bindless_image_handles_rsrcID = resource_mngr.createBufferObjectAsync(
                filepath + "_ptex_bindless_image_handles",
                0x90D2,//GL_SHADER_STORAGE_BUFFER
                empty_data
            );
            ptex_component_data.bindless_image_handles_ = bindless_image_handles_rsrcID;

            auto bindless_mipmap_image_handles_rsrcID = resource_mngr.createBufferObjectAsync(
                filepath + "_ptex_bindless_mipmap_image_handles",
                0x90D2,//GL_SHADER_STORAGE_BUFFER
                empty_data
            );
            ptex_component_data.bindless_mipmap_image_handles_ = bindless_mipmap_image_handles_rsrcID;


            auto ptex_idx = ptex_mngr.addComponent(ptex_component_data);
            auto xform_idx = transform_mngr.addComponent(ptex_component_data.entity);

            allocatePtexTextureTiles(resource_mngr, mtl_mngr, ptex_mngr, ptex_component_data.entity);

            //computePatchDistances(transform_mngr, camera_mngr, ptex_mngr, ptex_component_data.entity);
            //computeTextureTileUpdateLists(ptex_mngr, ptex_component_data.entity);

            // "finalize" by adding a render task
            renderTask_mngr.addComponent(ptex_component_data.entity, mesh_rsrcID, 0, ptex_mesh_shader_rsrc, 0, xform_idx, 0, mtl_idx);

            return ptex_component_data.entity;
        }
    }
}

#endif // !DynamicPtexMeshSystems_hpp
