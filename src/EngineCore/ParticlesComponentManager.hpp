#ifndef ParticlesComponentManager_hpp
#define ParticlesComponentManager_hpp

#include "BaseResourceManager.hpp"
#include "BaseMultiInstanceComponentManager2.hpp"
#include "ComponentStorage.hpp"

namespace EngineCore {
    namespace Graphics {

        namespace RenderTaskTags {
            struct Particles {};
        }

        struct ParticlesComponentData
        {
            Entity     entity;         ///< entity that owns the component
            size_t     particle_count;
            ResourceID particle_data;
        };

        template<typename ResourceManagerType>
        class ParticlesComponentManager : public BaseMultiInstanceComponentManager2<ParticlesComponentData,1000,1000>
        {
        private:
            ResourceManagerType* resource_mngr_;

        public:
            ParticlesComponentManager(ResourceManagerType* rsrc_mngr);
            ~ParticlesComponentManager() = default;

            template<typename ParticleType>
            size_t addComponent(Entity entity, std::shared_ptr<std::vector<ParticleType>> particle_data);
        };

        template<typename ResourceManagerType>
        template<typename ParticleType>
        inline size_t ParticlesComponentManager<ResourceManagerType>::addComponent(Entity entity, std::shared_ptr<std::vector<ParticleType>> particle_data)
        {
            auto idx_query = getIndex(entity);

            auto rsrc_id = resource_mngr_->createStructuredBufferAsync(
                std::to_string(entity.id()) + "_particles_" + std::to_string(idx_query.size()),
                particle_data
            );

            auto index = data_.addComponent(
                {
                    entity,
                    particle_data->size(),
                    rsrc_id
                }
            );

            addIndex(entity.id(), index);

            return index;
        }

        template<typename ResourceManagerType>
        inline ParticlesComponentManager<ResourceManagerType>::ParticlesComponentManager(ResourceManagerType* rsrc_mngr)
            : resource_mngr_(rsrc_mngr)
        {
        }
    }
}

#endif // !ParticlesComponentManager_hpp
