/// <copyright file="RenderTaskComponentManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef RenderTaskComponentManager_hpp
#define RenderTaskComponentManager_hpp

#include <set>

#include "EntityManager.hpp"
#include "BaseComponentManager.hpp"
#include "BaseResourceManager.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        class RenderTaskComponentManager : public BaseComponentManager
        {
        public:

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

                //size_t     cached_transform_idx;
                //size_t     cached_mesh_idx;
                //size_t     cached_material_idx;
            };


            void addComponent(Entity entity, ResourceID mesh, size_t mesh_component_subidx, ResourceID shader_prgm, size_t mtl_component_subidx, bool visible = true);

            std::vector<Data> const& getComponentData(); //TODO this is not thread safe, is it?

            std::vector<Data> getComponentDataCopy();

        private:

            std::vector<Data> m_data; //< store render task sorted by shader and mesh ResourceIDs
            std::shared_mutex m_data_mutex;
        };

    }
}

#endif //!RenderTaskComponentManager_hpp