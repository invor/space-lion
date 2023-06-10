/// <copyright file="MaterialComponentManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef MaterialComponentManager_hpp
#define MaterialComponentManager_hpp

#include <array>
#include <unordered_map>

#include "BaseMultiInstanceComponentManager.hpp"
#include "BaseResourceManager.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        class MaterialComponentManager : public BaseMultiInstanceComponentManager
        {
        public:

            enum TextureSemantic { ALBEDO, NORMAL, SPECULAR, METALLIC_ROUGHNESS, ROUGHNESS };

            MaterialComponentManager()
                : BaseMultiInstanceComponentManager() {}
            ~MaterialComponentManager() = default;

            void addComponent(
                Entity      entity,
                std::string material_name,
                ResourceID  shader_program);

            void addComponent(
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

            inline std::array<float, 4> getAlbedoColour(size_t idx) const {
                std::shared_lock<std::shared_mutex> lock(m_data_mutex);

                return m_component_data[idx].albedo_colour;
            }

            inline std::array<float, 4> getSpecularColour(size_t idx) const {
                std::shared_lock<std::shared_mutex> lock(m_data_mutex);

                return m_component_data[idx].specular_colour;
            }

            inline float getRoughness(size_t idx) const {
                std::shared_lock<std::shared_mutex> lock(m_data_mutex);

                return m_component_data[idx].roughness;
            }

            ResourceID getTextures(size_t component_idx, TextureSemantic semantic) const;

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
                    std::vector<std::pair<TextureSemantic,ResourceID>> const& textures,
                    bool double_sided = false
                )
                    : entity(entity),
                    material_name(name),
                    shader_program(shader_prgm),
                    albedo_colour(albedo_colour),
                    specular_colour(specular_colour),
                    roughness(roughness),
                    textures(textures.begin(),textures.end()),
                    double_sided(double_sided)
                {}

                Entity                  entity;
                std::string             material_name;

                ResourceID              shader_program;

                std::array<float, 4>    albedo_colour;
                std::array<float, 4>    specular_colour;
                float                   roughness;

                //std::unordered_multimap<TextureSemantic,ResourceID> textures;

                std::vector<std::pair<TextureSemantic, ResourceID>> textures;

                bool                    double_sided;
            };

            std::vector<ComponentData> m_component_data;
            mutable std::shared_mutex  m_data_mutex;
        };

        template <typename ResourceIDContainer>
        void MaterialComponentManager::addComponent(
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
                std::vector<std::pair<TextureSemantic, ResourceID>>(textures.begin(), textures.end())
            ));
        }
    }
}

#endif