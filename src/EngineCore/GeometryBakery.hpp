#ifndef GeometryBakery_hpp
#define GeometryBakery_hpp

#include <memory>
#include <tuple>
#include <vector>

#include "GenericVertexLayout.hpp"
#include "types.hpp"

typedef std::vector<std::vector<uint8_t>> VertexData;
typedef std::vector<uint32_t>             IndexData;
typedef std::vector<GenericVertexLayout>  VertexDataDescriptor;

typedef std::shared_ptr<VertexData>          VertexDataPtr;
typedef std::shared_ptr<IndexData>           IndexDataPtr;
typedef std::shared_ptr<VertexDataDescriptor> VertexDataDescriptorPtr;

namespace EngineCore
{
    namespace Graphics
    {
        enum VertexAttributeBitMask
        {
            NONE      = 0x0,
            POSITION  = 0x1,
            NORMAL    = 0x2,
            TANGENT   = 0x4,
            BITANGENT = 0x8,
            UV        = 0x10,
            COLOR     = 0x20
        };

        /**
        * \brief Creates and return triangle geometry
        */
        std::tuple<VertexData, IndexData, VertexDataDescriptor> createTriangle();

        /**
        * \brief Creates and return plane (quad) geometry
        */
        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createPlane(
            float width,
            float height,
            int attribute_bit_mask,
            bool z_up = false);

        /**
        * \brief Creates and returns unit box geometry
        */
        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createBox();

        /**
        * \brief Create an ico sphere mesh
        * \param subdivisions Control the subdivions of the sphere
        * \return Returns shared pointer to the mesh
        */
        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createIcoSphere(uint subdivions, float radius = 1.0f);

        /**
        *
        */
        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createBone(float bone_length = 1.0);

        /**
        * \brief Creates and returns a cylinder geometry
        * \param segments controls the subdivisions of the cylinder
        */
        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createCylinder(float radius, float height, int segments = 8);

        /**
        * \brief Creates and returns a truncated cone geometry
        * \param segments controls the subdivision of the truncated cone
        */
        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createTruncatedCone(float base_radius, float top_radius, float height, int segments = 8);

        /**
        * Credit goes to Dan: https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal/66918075#66918075
        */
        template<typename IndexType>
        void makeTangents(uint32_t nIndices, IndexType* indices,
            const glm::vec3* positions, const glm::vec3* normals,
            const glm::vec2* texCoords, glm::vec4* tangents)
        {
            uint32_t inconsistentUvs = 0;
            for (uint32_t l = 0; l < nIndices; ++l) tangents[indices[l]] = glm::vec4(0);
            for (uint32_t l = 0; l < nIndices; ++l) {
                uint32_t i = indices[l];
                uint32_t j = indices[(l + 1) % 3 + l / 3 * 3];
                uint32_t k = indices[(l + 2) % 3 + l / 3 * 3];
                glm::vec3 n = normals[i];
                glm::vec3 v1 = positions[j] - positions[i], v2 = positions[k] - positions[i];
                glm::vec2 t1 = texCoords[j] - texCoords[i], t2 = texCoords[k] - texCoords[i];

                // Is the texture flipped?
                float uv2xArea = t1.x * t2.y - t1.y * t2.x;
                if (std::abs(uv2xArea) < 0x1p-20)
                    continue;  // Smaller than 1/2 pixel at 1024x1024
                float flip = uv2xArea > 0 ? 1 : -1;
                // 'flip' or '-flip'; depends on the handedness of the space.
                if (tangents[i].w != 0 && tangents[i].w != -flip) ++inconsistentUvs;
                tangents[i].w = -flip;

                // Project triangle onto tangent plane
                v1 -= n * dot(v1, n);
                v2 -= n * dot(v2, n);
                // Tangent is object space direction of texture coordinates
                glm::vec3 s = normalize((t2.y * v1 - t1.y * v2) * flip);

                // Use angle between projected v1 and v2 as weight
                float angle = std::acos(dot(v1, v2) / (length(v1) * length(v2)));
                tangents[i] += glm::vec4(s * angle, 0);
            }
            for (uint32_t l = 0; l < nIndices; ++l) {
                glm::vec4& t = tangents[indices[l]];
                t = glm::vec4(normalize(glm::vec3(t.x, t.y, t.z)), t.w);
            }
            // std::cerr << inconsistentUvs << " inconsistent UVs\n";
        }
    }
}

#endif // !GeometryBakery_hpp
