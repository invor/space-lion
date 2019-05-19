/// <copyright file="MaterialComponentManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef MaterialComponentManager_hpp
#define MaterialComponentManager_hpp

#include <array>

#include "BaseComponentManager.hpp"
#include "BaseResourceManager.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        template<typename ResourceManagerType>
        class MaterialComponentManager : public BaseComponentManager
        {
        public:
            MaterialComponentManager(ResourceManagerType* resource_manager)
                : BaseComponentManager(), m_resource_manager(resource_manager) {}
            ~MaterialComponentManager() = default;


            //template <typename ...T>
            //void addComponent(
            //	Entity entity,
            //	std::string material_name,
            //	ResourceID shader_program,
            //	T... textures)
            //{
            //	addComponent(entity, material_name, shader_program, std::initializer_list<T>{ textures... });
            //}

            inline void addComponent(
                Entity      entity,
                std::string material_name,
                ResourceID  shader_program);

            inline void addComponent(
                Entity entity,
                std::string          material_name,
                ResourceID           shader_program,
                std::array<float, 4> albedo_colour,
                std::array<float, 4> specular_colour,
                float                roughness);

            template <typename ResourceIDContainer>
            void addComponent(
                Entity               entity,
                std::string          material_name,
                ResourceID           shader_program,
                std::array<float, 4> albedo_colour,
                std::array<float, 4> specular_colour,
                float                roughness,
                ResourceIDContainer  textures);

            inline std::array<float, 4> getAlbedoColour(size_t idx) {
                std::shared_lock<std::shared_mutex> lock(m_data_mutex);

                return m_component_data[idx].albedo_colour;
            }

            inline std::array<float, 4> getSpecularColour(size_t idx) {
                std::shared_lock<std::shared_mutex> lock(m_data_mutex);

                return m_component_data[idx].specular_colour;
            }

            inline float getRoughness(size_t idx) {
                std::shared_lock<std::shared_mutex> lock(m_data_mutex);

                return m_component_data[idx].roughness;
            }

        private:

            struct ComponentData
            {
                ComponentData(
                    Entity                         entity,
                    std::string                    name,
                    ResourceID                     shader_prgm,
                    std::array<float, 4>           albedo_colour,
                    std::array<float, 4>           specular_colour,
                    float                          roughness,
                    std::vector<ResourceID> const& textures)
                    : entity(entity),
                    material_name(name),
                    shader_program(shader_program),
                    albedo_colour(albedo_colour),
                    specular_colour(specular_colour),
                    roughness(roughness),
                    textures(textures)
                {}


                Entity                  entity;
                std::string             material_name;

                ResourceID              shader_program;

                std::array<float, 4>    albedo_colour;
                std::array<float, 4>    specular_colour;
                float                   roughness;

                std::vector<ResourceID> textures;
            };

            std::vector<ComponentData> m_component_data;
            std::shared_mutex          m_data_mutex;

            ResourceManagerType*       m_resource_manager;
        };

        template<typename ResourceManagerType>
        inline void MaterialComponentManager<ResourceManagerType>::addComponent(
            Entity entity,
            std::string material_name,
            ResourceID shader_program)
        {
            addComponent(
                entity,
                material_name,
                shader_program,
                std::array<float, 4>{1.0f, 0.5f, 1.0f, 1.0f},
                std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f},
                0.8f,
                std::vector<ResourceID>());
        }

        template<typename ResourceManagerType>
        inline void MaterialComponentManager<ResourceManagerType>::addComponent(
            Entity entity,
            std::string material_name,
            ResourceID shader_program,
            std::array<float, 4> albedo_colour,
            std::array<float, 4> specular_colour,
            float roughness)
        {
            addComponent(
                entity,
                material_name,
                shader_program,
                albedo_colour,
                specular_colour,
                roughness,
                std::vector<ResourceID>());
        }

        template<typename ResourceManagerType>
        template <typename ResourceIDContainer>
        inline void MaterialComponentManager<ResourceManagerType>::addComponent(
            Entity               entity,
            std::string          material_name,
            ResourceID           shader_program,
            std::array<float, 4> albedo_colour,
            std::array<float, 4> specular_colour,
            float                roughness,
            ResourceIDContainer  textures)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_mutex);

            addIndex(entity.id(), m_component_data.size());

            m_component_data.push_back(ComponentData(
                entity,
                material_name,
                shader_program,
                albedo_colour,
                specular_colour,
                roughness,
                std::vector<ResourceID>(textures.begin(), textures.end())
            ));
        }

    }
}

#endif