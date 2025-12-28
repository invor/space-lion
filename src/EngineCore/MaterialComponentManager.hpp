/// <copyright file="MaterialComponentManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef MaterialComponentManager_hpp
#define MaterialComponentManager_hpp

#include <array>
#include <unordered_map>

#include "BaseMultiInstanceComponentManager2.hpp"
#include "BaseResourceManager.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        struct MaterialComponentData
        {
            enum TextureSemantic { ALBEDO, NORMAL, SPECULAR, METALLIC_ROUGHNESS, ROUGHNESS };

            Entity               entity;
            std::string          material_name;

            ResourceID           shader_program;

            std::array<float, 4> albedo_colour;
            std::array<float, 4> specular_colour;
            float                roughness;

            std::vector<std::pair<TextureSemantic, ResourceID>> textures;

            bool                 double_sided;
        };

        class MaterialComponentManager : public BaseMultiInstanceComponentManager2<MaterialComponentData, 100, 100>
        {
        public:

            MaterialComponentManager()
                : BaseMultiInstanceComponentManager2() {}
            ~MaterialComponentManager() = default;

            size_t addComponent(
                Entity      entity,
                std::string material_name,
                ResourceID  shader_program);

            size_t addComponent(
                Entity entity,
                std::string          material_name,
                ResourceID           shader_program,
                std::array<float, 4> albedo_colour,
                std::array<float, 4> specular_colour,
                float                roughness);

            template <typename ResourceIDContainer>
            size_t addComponent(
                Entity               entity,
                std::string          material_name,
                ResourceID           shader_program,
                std::array<float, 4> albedo_colour,
                std::array<float, 4> specular_colour,
                float                roughness,
                ResourceIDContainer  textures);

            inline std::array<float, 4> getAlbedoColour(size_t idx) const {
                auto [page_idx, idx_in_page] = data_.getIndices(idx);
                return data_(page_idx, idx_in_page).albedo_colour;
            }

            inline void setAlbedoColour(size_t idx, std::array<float, 4> albedo_colour) {
                auto [page_idx, idx_in_page] = data_.getIndices(idx);
                data_(page_idx, idx_in_page).albedo_colour = albedo_colour;
            }

            inline std::array<float, 4> getSpecularColour(size_t idx) const {
                auto [page_idx, idx_in_page] = data_.getIndices(idx);
                return data_(page_idx, idx_in_page).specular_colour;
            }

            inline float getRoughness(size_t idx) const {
                auto [page_idx, idx_in_page] = data_.getIndices(idx);
                return data_(page_idx, idx_in_page).roughness;
            }

            std::vector<ResourceID> getTextures(
                size_t idx, 
                MaterialComponentData::TextureSemantic semantic) const;
        };

        template <typename ResourceIDContainer>
        size_t MaterialComponentManager::addComponent(
            Entity               entity,
            std::string          material_name,
            ResourceID           shader_program,
            std::array<float, 4> albedo_colour,
            std::array<float, 4> specular_colour,
            float                roughness,
            ResourceIDContainer  textures)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    std::move(material_name),
                    shader_program,
                    albedo_colour,
                    specular_colour,
                    roughness,
                    std::vector<std::pair<MaterialComponentData::TextureSemantic, ResourceID>>(textures.begin(), textures.end())
                }
            );

            addIndex(entity.id(), index);

            return index;
        }
    }
}

#endif