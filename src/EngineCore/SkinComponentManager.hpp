#ifndef SkinComponentManager_hpp
#define SkinComponentManager_hpp

#include <vector>

#include "BaseSingleInstanceComponentManager.hpp"
#include "EntityManager.hpp"
#include "types.hpp"

namespace EngineCore {

    namespace Graphics {
        namespace RenderTaskTags {
            struct SkinnedMesh {};
        }
    }

    namespace Animation {
        class SkinComponentManager : public BaseSingleInstanceComponentManager
        {
        private:
            struct Data
            {
                Data(Entity entity, std::vector<Entity> const& joints, std::vector<Mat4x4> const& inverse_bind_matrices)
                    : entity(entity), joints(joints), inverse_bind_matrices(inverse_bind_matrices) {}

                Entity              entity;
                std::vector<Entity> joints;
                std::vector<Mat4x4> inverse_bind_matrices;
                // store and reference bind matrices directly in GPU buffer
            };

            std::vector<Data> m_data;
            std::shared_mutex m_data_access_mutex;

        public:
            SkinComponentManager() = default;
            ~SkinComponentManager() = default;

            void addComponent(Entity entity, std::vector<Entity> const& joints, std::vector<Mat4x4> const& inverse_bind_matrices);

            std::vector<Entity> const& getJoints(Entity entity);

            std::vector<Entity> const& getJoints(size_t component_idx);

            std::vector<Mat4x4> const& getInvsereBindMatrices(Entity entity);

            std::vector<Mat4x4> const& getInvsereBindMatrices(size_t component_idx);
        };
    }
}

#endif // !SkinComponentManager_hpp
