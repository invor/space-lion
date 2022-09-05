/// <copyright file="RenderTaskComponentManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef RenderTaskComponentManager_hpp
#define RenderTaskComponentManager_hpp

#include <set>

#include "EntityManager.hpp"
#include "BaseMultiInstanceComponentManager.hpp"
#include "BaseResourceManager.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        namespace RenderTaskTags{
            struct StaticMesh {};
        }

        template<typename TagType>
        class RenderTaskComponentManager : public BaseMultiInstanceComponentManager
        {
        public:

            // TODO API-agnostic rendering states

            enum class DepthTestState { 
                DISABLED,
                LESS,
                GREATER
            };

            enum class BlendState {
                DISABLED,
                ADDITIVE,
                ONE_MINUS_SRC_ALPHA
            };

            enum class CullingState {
                DISABLED,
                BACKFACE,
                FRONTFACE
            };

            struct Data
            {
                Data(Entity e, ResourceID mesh, size_t mesh_component_subidx, ResourceID shader_prgm, size_t mtl_component_subidx, bool visible)
                    : entity(e), mesh(mesh), mesh_component_subidx(mesh_component_subidx), shader_prgm(shader_prgm), mtl_component_subidx(mtl_component_subidx), visible(visible) {}

                inline friend bool operator< (const Data& lhs, const Data& rhs) {
                    return (lhs.shader_prgm.value() == rhs.shader_prgm.value() ? lhs.mesh.value() < rhs.mesh.value() : lhs.shader_prgm.value() < rhs.shader_prgm.value());
                }

                Entity     entity;
                ResourceID mesh;                  //< mesh resource used for the render task
                size_t     mesh_component_subidx; //< denotes which of the entity's mesh components to use for the render task (set to 0 if entity does NOT have multiple mesh components)
                ResourceID shader_prgm;           //< shader program resource used for the render task
                size_t     mtl_component_subidx;  //< denotes which of the entity's material components to use for the render task (set to 0 if entity does NOT have multiple material components)
                bool       visible;               //< used to show/hide object without completely removing render task

                size_t     cached_transform_idx;
                size_t     cached_mesh_idx;
                size_t     cached_material_idx;
            };

            void addComponent(
                Entity entity,
                ResourceID mesh,
                size_t mesh_component_subidx,
                ResourceID shader_prgm,
                size_t mtl_component_subidx,
                size_t cached_transform_idx,
                size_t cached_mesh_idx,
                size_t cached_material_idx,
                bool visible = true);

            std::vector<Data> & getComponentData(); //TODO this is not thread safe, is it?

            std::vector<Data> getComponentDataCopy() const;

        private:

            std::vector<Data>         m_data; //< store render task sorted by shader and mesh ResourceIDs
            mutable std::shared_mutex m_data_mutex;
        };


        template<typename TagType>
        void RenderTaskComponentManager<TagType>::addComponent(
            Entity entity,
            ResourceID mesh,
            size_t mesh_component_subidx,
            ResourceID shader_prgm,
            size_t mtl_component_subidx,
            size_t cached_transform_idx,
            size_t cached_mesh_idx,
            size_t cached_material_idx,
            bool visible)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_mutex);

            addIndex(entity.id(), m_data.size());

            m_data.emplace_back(Data(entity, mesh, mesh_component_subidx, shader_prgm, mtl_component_subidx, visible));

            m_data.back().cached_transform_idx = cached_transform_idx;
            m_data.back().cached_mesh_idx = cached_mesh_idx;
            m_data.back().cached_material_idx = cached_material_idx;

            std::sort(m_data.begin(), m_data.end());
        }

        template<typename TagType>
        inline std::vector<typename RenderTaskComponentManager<TagType>::Data>& RenderTaskComponentManager<TagType>::getComponentData()
        {
            return m_data;
        }

        template<typename TagType>
        std::vector<typename RenderTaskComponentManager<TagType>::Data> RenderTaskComponentManager<TagType>::getComponentDataCopy() const
        {
            std::vector<EngineCore::Graphics::RenderTaskComponentManager<TagType>::Data> retval;

            {
                std::shared_lock<std::shared_mutex> lock(m_data_mutex);

                retval = m_data;
            }

            return retval;
        }


    }
}

#endif //!RenderTaskComponentManager_hpp