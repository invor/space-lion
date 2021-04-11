#include "GeometryBakery.hpp"

#include <array>

namespace EngineCore
{
    namespace Graphics
    {
        std::tuple<VertexData, IndexData, VertexDataDescriptor> createTriangle()
        {
            VertexData vertices(3 * 6 * 4); // 3 triangles * 6 float entries * bytesize
            IndexData indices(3);

            float* floatView = reinterpret_cast<float*>(vertices.data());
            floatView[0] = -0.5f;
            floatView[1] = 0.0f;
            floatView[2] = 0.0f;
            floatView[3] = 0.0f;
            floatView[4] = 0.0f;
            floatView[5] = 1.0f;

            floatView[6] = 0.5f;
            floatView[7] = 0.0f;
            floatView[8] = 0.0f;
            floatView[9] = 0.0f;
            floatView[10] = 0.0f;
            floatView[11] = 1.0f;

            floatView[12] = 0.0f;
            floatView[13] = 1.0f;
            floatView[14] = 0.0f;
            floatView[15] = 0.0f;
            floatView[16] = 0.0f;
            floatView[17] = 1.0f;

            indices[0] = 0; indices[1] = 1; indices[2] = 2;

            VertexDataDescriptor layout(
                { GenericVertexLayout(24 , { GenericVertexLayout::Attribute(3, 5126, false,0), GenericVertexLayout::Attribute(3, 5126, false, 12) }) }
            );

            return std::tuple<VertexData, IndexData, VertexDataDescriptor>(vertices, indices, layout);
        }

        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createPlane(float width, float height)
        {
            struct Vec2
            {
                float u, v;
            };

            struct Vec3
            {
                float x, y, z;
            };

            struct Color
            {
                uint8_t r, g, b, a;
            };

            VertexDataPtr vertex_data = std::make_shared< std::vector<std::vector<uint8_t>>>(6);
            (*vertex_data)[0].resize(4 * sizeof(Vec3));
            (*vertex_data)[1].resize(4 * sizeof(Vec3));
            (*vertex_data)[2].resize(4 * sizeof(Vec3));
            (*vertex_data)[3].resize(4 * sizeof(Color));
            (*vertex_data)[4].resize(4 * sizeof(Vec2));
            (*vertex_data)[5].resize(4 * sizeof(Vec3));

            Vec3* position_ptr = reinterpret_cast<Vec3*>((*vertex_data)[0].data());
            Vec3* normal_ptr = reinterpret_cast<Vec3*>((*vertex_data)[1].data());
            Vec3* tangent_ptr = reinterpret_cast<Vec3*>((*vertex_data)[2].data());
            Color* color_ptr = reinterpret_cast<Color*>((*vertex_data)[3].data());
            Vec2* uv_ptr = reinterpret_cast<Vec2*>((*vertex_data)[4].data());
            Vec3* bitangent_ptr = reinterpret_cast<Vec3*>((*vertex_data)[5].data());

            std::array<float, 4> width_signs = { -1.0f,1.0f,1.0f,-1.0f };
            std::array<float, 4> height_signs = { -1.0f,-1.0f,1.0f,1.0f };

            for (int i = 0; i < 4; ++i)
            {
                position_ptr[i].x = (width / 2.0f) * width_signs[i];
                position_ptr[i].y = 0.0f;
                position_ptr[i].z = (height / 2.0f) * height_signs[i];

                normal_ptr[i].x = 0.0f;
                normal_ptr[i].y = 1.0f;
                normal_ptr[i].z = 0.0f;

                tangent_ptr[i].x = 1.0f;
                tangent_ptr[i].y = 0.0f;
                tangent_ptr[i].z = 0.0f;

                color_ptr[i].r = 128;
                color_ptr[i].g = 128;
                color_ptr[i].b = 255;
                color_ptr[i].a = 255;

                uv_ptr[i].u = (width_signs[i] + 1.0f) / 2.0f;
                uv_ptr[i].v = (height_signs[i] + 1.0f) / 2.0f;

                bitangent_ptr[i].x = 0.0f;
                bitangent_ptr[i].y = 0.0f;
                bitangent_ptr[i].z = 1.0f;
            }

            IndexDataPtr indices = std::make_shared<IndexData>(IndexData{ 0,3,1,3,2,0 });

            auto layout = std::make_shared<VertexDataDescriptor>(
                std::vector<GenericVertexLayout>{
                    GenericVertexLayout(12, { GenericVertexLayout::Attribute(5126 /*GL_FLOAT*/,3,false,0),
                        GenericVertexLayout::Attribute(3, 5126, false,0),
                        GenericVertexLayout::Attribute(3, 5126, false,0),
                        GenericVertexLayout::Attribute(4, 5121, false,0),
                        GenericVertexLayout::Attribute(2, 5126, false,0),
                        GenericVertexLayout::Attribute(3, 5126, false,0) })
                }
            );

            return std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr>(vertex_data, indices, layout);
        }

        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createBox()
        {
            struct Vec2
            {
                float u, v;
            };

            struct Vec3
            {
                float x, y, z;
            };

            struct Vec4
            {
                float x, y, z, w;
            };

            struct Color
            {
                uint8_t r, g, b, a;
            };

            std::vector<uint8_t> positions(24 * sizeof(Vec3));
            std::vector<uint8_t> normals(24 * sizeof(Vec3));
            std::vector<uint8_t> tangents(24 * sizeof(Vec4));
            std::vector<uint8_t> colors(24 * sizeof(Color));
            std::vector<uint8_t> uv_coords(24 * sizeof(Vec2));
            std::vector<uint8_t> bitangents(24 * sizeof(Vec3));

            Vec3* access_ptr = reinterpret_cast<Vec3*>(positions.data());
            /*	front face */
            access_ptr[0] = { -0.5f, -0.5f, 0.5f }; access_ptr[1] = { -0.5f, 0.5f, 0.5f };
            access_ptr[2] = { 0.5f, 0.5f, 0.5f }; access_ptr[3] = { 0.5f, -0.5f, 0.5f };
            /*	right face */
            access_ptr[4] = { 0.5f, -0.5f, 0.5f }; access_ptr[5] = { 0.5f, 0.5f, 0.5f };
            access_ptr[6] = { 0.5f, 0.5f, -0.5f }; access_ptr[7] = { 0.5f, -0.5f, -0.5f };
            /*	left face */
            access_ptr[8] = { -0.5, -0.5, -0.5 }; access_ptr[9] = { -0.5, 0.5, -0.5 };
            access_ptr[10] = { -0.5, 0.5, 0.5 }; access_ptr[11] = { -0.5, -0.5, 0.5 };
            /*	back face */
            access_ptr[12] = { 0.5, -0.5, -0.5 }; access_ptr[13] = { 0.5, 0.5, -0.5 };
            access_ptr[14] = { -0.5, 0.5, -0.5 }; access_ptr[15] = { -0.5, -0.5, -0.5 };
            /*	bottom face */
            access_ptr[16] = { -0.5, -0.5, 0.5 }; access_ptr[17] = { -0.5, -0.5, -0.5 };
            access_ptr[18] = { 0.5, -0.5, -0.5 }; access_ptr[19] = { 0.5, -0.5, 0.5 };
            /*	top face */
            access_ptr[20] = { -0.5, 0.5, 0.5 }; access_ptr[21] = { -0.5, 0.5, -0.5 };
            access_ptr[22] = { 0.5, 0.5, -0.5 }; access_ptr[23] = { 0.5, 0.5, 0.5 };

            access_ptr = reinterpret_cast<Vec3*>(normals.data());
            /*	front face */
            access_ptr[0] = { 0.0, 0.0, 1.0 }; access_ptr[1] = { 0.0, 0.0, 1.0 };
            access_ptr[2] = { 0.0, 0.0, 1.0 }; access_ptr[3] = { 0.0, 0.0, 1.0 };
            /*	right face */
            access_ptr[4] = { 1.0, 0.0, 0.0 }; access_ptr[5] = { 1.0, 0.0, 0.0 };
            access_ptr[6] = { 1.0, 0.0, 0.0 }; access_ptr[7] = { 1.0, 0.0, 0.0 };
            /*	left face */
            access_ptr[8] = { -1.0, 0.0, 0.0 }; access_ptr[9] = { -1.0, 0.0, 0.0 };
            access_ptr[10] = { -1.0, 0.0, 0.0 }; access_ptr[11] = { -1.0, 0.0, 0.0 };
            /*	back face */
            access_ptr[12] = { 0.0, 0.0, -1.0 }; access_ptr[13] = { 0.0, 0.0, -1.0 };
            access_ptr[14] = { 0.0, 0.0, -1.0 }; access_ptr[15] = { 0.0, 0.0, -1.0 };
            /*	bottom face */
            access_ptr[16] = { 0.0, -1.0, 0.0 }; access_ptr[17] = { 0.0, -1.0, 0.0 };
            access_ptr[18] = { 0.0, -1.0, 0.0 }; access_ptr[19] = { 0.0, -1.0, 0.0 };
            /*	top face */
            access_ptr[20] = { 0.0, 1.0, 0.0 }; access_ptr[21] = { 0.0, 1.0, 0.0 };
            access_ptr[22] = { 0.0, 1.0, 0.0 }; access_ptr[23] = { 0.0, 1.0, 0.0 };

            Vec4* tangent_access_ptr = reinterpret_cast<Vec4*>(tangents.data());
            /*	front face */
            tangent_access_ptr[0] = { 1.0, 0.0, 0.0, 1.0 }; tangent_access_ptr[1] = { 1.0, 0.0, 0.0, 1.0 };
            tangent_access_ptr[2] = { 1.0, 0.0, 0.0, 1.0 }; tangent_access_ptr[3] = { 1.0, 0.0, 0.0, 1.0 };
            /*	right face */
            tangent_access_ptr[4] = { 0.0, 0.0, -1.0, 1.0 }; tangent_access_ptr[5] = { 0.0, 0.0, -1.0, 1.0 };
            tangent_access_ptr[6] = { 0.0, 0.0, -1.0, 1.0 }; tangent_access_ptr[7] = { 0.0, 0.0, -1.0, 1.0 };
            /*	left face */
            tangent_access_ptr[8] = { 0.0, 0.0, 1.0 , 1.0 }; tangent_access_ptr[9] = { 0.0, 0.0, 1.0, 1.0 };
            tangent_access_ptr[10] = { 0.0, 0.0, 1.0, 1.0 }; tangent_access_ptr[11] = { 0.0, 0.0, 1.0, 1.0 };
            /*	back face */
            tangent_access_ptr[12] = { -1.0, 0.0, 0.0, 1.0 }; tangent_access_ptr[13] = { -1.0, 0.0, 0.0, 1.0 };
            tangent_access_ptr[14] = { -1.0, 0.0, 0.0, 1.0 }; tangent_access_ptr[15] = { -1.0, 0.0, 0.0, 1.0 };
            /*	bottom face */
            tangent_access_ptr[16] = { 1.0, 0.0, 0.0, 1.0 }; tangent_access_ptr[17] = { 1.0, 0.0, 0.0, 1.0 };
            tangent_access_ptr[18] = { 1.0, 0.0, 0.0, 1.0 }; tangent_access_ptr[19] = { 1.0, 0.0, 0.0, 1.0 };
            /*	top face */
            tangent_access_ptr[20] = { 1.0, 0.0, 0.0, 1.0 }; tangent_access_ptr[21] = { 1.0, 0.0, 0.0, 1.0 };
            tangent_access_ptr[22] = { 1.0, 0.0, 0.0, 1.0 }; tangent_access_ptr[23] = { 1.0, 0.0, 0.0, 1.0 };

            Color* col_access_ptr = reinterpret_cast<Color*>(colors.data());
            /*	front face */
            col_access_ptr[0] = { (uint8_t)0.0, (uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0 }; col_access_ptr[1] = { (uint8_t)0.0, (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0 };
            col_access_ptr[2] = { (uint8_t)1.0, (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0 }; col_access_ptr[3] = { (uint8_t)1.0, (uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0 };
            /*	right face */
            col_access_ptr[4] = { (uint8_t)1.0, (uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0 }; col_access_ptr[5] = { (uint8_t)1.0, (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0 };
            col_access_ptr[6] = { (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0 }; col_access_ptr[7] = { (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0 };
            /*	left face */
            col_access_ptr[8] = { (uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0 }; col_access_ptr[9] = { (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0 };
            col_access_ptr[10] = { (uint8_t)0.0, (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0 }; col_access_ptr[11] = { (uint8_t)0.0, (uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0 };
            /*	back face */
            col_access_ptr[12] = { (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0 }; col_access_ptr[13] = { (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0 };
            col_access_ptr[14] = { (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0 }; col_access_ptr[15] = { -(uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0 };
            /*	bottom face */
            col_access_ptr[16] = { (uint8_t)0.0, (uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0 }; col_access_ptr[17] = { (uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0 };
            col_access_ptr[18] = { (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0 }; col_access_ptr[19] = { (uint8_t)1.0, (uint8_t)0.0, (uint8_t)0.0, (uint8_t)1.0 };
            /*	top face */
            col_access_ptr[20] = { (uint8_t)0.0, (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0 }; col_access_ptr[21] = { (uint8_t)0.0, (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0 };
            col_access_ptr[22] = { (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0, (uint8_t)1.0 }; col_access_ptr[23] = { (uint8_t)1.0, (uint8_t)1.0, (uint8_t)0.0, (uint8_t)1.0 };

            Vec2* uv_access_ptr = reinterpret_cast<Vec2*>(uv_coords.data());
            /*	front face */
            uv_access_ptr[0] = { 0.0, 0.0 }; uv_access_ptr[1] = { 0.0, 1.0 };
            uv_access_ptr[2] = { 1.0, 1.0 }; uv_access_ptr[3] = { 1.0, 0.0 };
            /*	right face */
            uv_access_ptr[4] = { 0.0, 0.0 }; uv_access_ptr[5] = { 0.0, 1.0 };
            uv_access_ptr[6] = { 1.0, 1.0 }; uv_access_ptr[7] = { 1.0, 0.0 };
            /*	left face */
            uv_access_ptr[8] = { 0.0, 0.0 }; uv_access_ptr[9] = { 0.0, 1.0 };
            uv_access_ptr[10] = { 1.0, 1.0 }; uv_access_ptr[11] = { 1.0, 0.0 };
            /*	back face */
            uv_access_ptr[12] = { 0.0, 0.0 }; uv_access_ptr[13] = { 0.0, 1.0 };
            uv_access_ptr[14] = { 1.0, 1.0 }; uv_access_ptr[15] = { 1.0, 0.0 };
            /*	bottom face */
            uv_access_ptr[16] = { 0.0, 0.0 }; uv_access_ptr[17] = { 0.0, 1.0 };
            uv_access_ptr[18] = { 1.0, 1.0 }; uv_access_ptr[19] = { 1.0, 0.0 };
            /*	top face */
            uv_access_ptr[20] = { 0.0, 0.0 }; uv_access_ptr[21] = { 0.0, 1.0 };
            uv_access_ptr[22] = { 1.0, 1.0 }; uv_access_ptr[23] = { 1.0, 0.0 };

            access_ptr = reinterpret_cast<Vec3*>(bitangents.data());
            /*	front face */
            access_ptr[0] = { 0.0, 1.0, 0.0 }; access_ptr[1] = { 0.0, 1.0, 0.0 };
            access_ptr[2] = { 0.0, 1.0, 0.0 }; access_ptr[3] = { 0.0, 1.0, 0.0 };
            /*	right face */
            access_ptr[4] = { 0.0, 1.0, 0.0 }; access_ptr[5] = { 0.0, 1.0, 0.0 };
            access_ptr[6] = { 0.0, 1.0, 0.0 }; access_ptr[7] = { 0.0, 1.0, 0.0 };
            /*	left face */
            access_ptr[8] = { 0.0, 1.0, 0.0 }; access_ptr[9] = { 0.0, 1.0, 0.0 };
            access_ptr[10] = { 0.0, 1.0, 0.0 }; access_ptr[11] = { 0.0, 1.0, 0.0 };
            /*	back face */
            access_ptr[12] = { 0.0, 1.0, 0.0 }; access_ptr[13] = { 0.0, 1.0, 0.0 };
            access_ptr[14] = { 0.0, 1.0, 0.0 }; access_ptr[15] = { 0.0, 1.0, 0.0 };
            /*	bottom face */
            access_ptr[16] = { 0.0, 0.0, 1.0 }; access_ptr[17] = { 0.0, 0.0, 1.0 };
            access_ptr[18] = { 0.0, 0.0, 1.0 }; access_ptr[19] = { 0.0, 0.0, 1.0 };
            /*	top face */
            access_ptr[20] = { 0.0, 0.0, 1.0 }; access_ptr[21] = { 0.0, 0.0, 1.0 };
            access_ptr[22] = { 0.0, 0.0, 1.0 }; access_ptr[23] = { 0.0, 0.0, 1.0 };

            auto indices = std::make_shared< std::vector < uint32_t>>(36);
            (*indices)[0] = 0;   (*indices)[1] = 2;   (*indices)[2] = 1;
            (*indices)[3] = 2;   (*indices)[4] = 0;   (*indices)[5] = 3;
            (*indices)[6] = 4;   (*indices)[7] = 6;   (*indices)[8] = 5;
            (*indices)[9] = 6;   (*indices)[10] = 4;  (*indices)[11] = 7;
            (*indices)[12] = 8;  (*indices)[13] = 10; (*indices)[14] = 9;
            (*indices)[15] = 10; (*indices)[16] = 8;  (*indices)[17] = 11;
            (*indices)[18] = 12; (*indices)[19] = 14; (*indices)[20] = 13;
            (*indices)[21] = 14; (*indices)[22] = 12; (*indices)[23] = 15;
            (*indices)[24] = 16; (*indices)[25] = 17; (*indices)[26] = 18;
            (*indices)[27] = 18; (*indices)[28] = 19; (*indices)[29] = 16;
            (*indices)[30] = 20; (*indices)[31] = 22; (*indices)[32] = 21;
            (*indices)[33] = 22; (*indices)[34] = 20; (*indices)[35] = 23;

            auto layout = std::make_shared<VertexDataDescriptor>(
                std::vector<GenericVertexLayout>{
                    GenericVertexLayout( 12, { { 3, 5126 /* GL_FLOAT */, false, 0 } }),
                    GenericVertexLayout( 12, { { 3, 5126 /* GL_FLOAT */, false, 0 } }),
                    GenericVertexLayout( 16, { { 4, 5126 /* GL_FLOAT */, false, 0 } }),
                    GenericVertexLayout(  8, { { 2, 5126 /* GL_FLOAT */, false, 0 } })
                }
            );

            auto vertices = std::make_shared< std::vector <std::vector<uint8_t>>>( 
                std::vector<std::vector<uint8_t>>{ normals, positions, tangents, uv_coords});
                //std::vector<std::vector<uint8_t>>{ positions, normals, tangents, colors, uv_coords, bitangents });

            return std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr>(vertices, indices, layout);
        }

        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createIcoSphere(uint subdivions, float radius)
        {
            // Create intial icosahedron
            float x = 0.525731112119133606f * radius;
            float z = 0.850650808352039932f * radius;
            float nx = 0.525731112119133606f;
            float nz = 0.850650808352039932f;

            struct Vertex_pn
            {
                Vertex_pn(float x, float y, float z, float nx, float ny, float nz)
                    : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz) {}

                float x, y, z;
                float nx, ny, nz;
            };

            

            std::vector<Vertex_pn> vertices({ Vertex_pn(-x,0.0f,z,-nx,0.0f,nz),
                Vertex_pn(x,0.0f,z,nx,0.0f,nz),
                Vertex_pn(-x,0.0f,-z,-nx,0.0f,-nz),
                Vertex_pn(x,0.0f,-z,nx,0.0f,-nz),
                Vertex_pn(0.0f,z,x,0.0f,nz,nx),
                Vertex_pn(0.0f,z,-x,0.0f,nz,-nx),
                Vertex_pn(0.0f,-z,x,0.0f,-nz,nx),
                Vertex_pn(0.0f,-z,-x,0.0f,-nz,-nx),
                Vertex_pn(z,x,0.0f,nz,nx,0.0f),
                Vertex_pn(-z,x,0.0f,-nz,nx,0.0f),
                Vertex_pn(z,-x,0.0f,nz,-nx,0.0f),
                Vertex_pn(-z,-x,0.0f,-nz,-nx,0.0f) });
            IndexDataPtr indices = std::make_shared<IndexData>(
                std::vector<unsigned int>({
                    0,4,1,	0,9,4,	9,5,4,	4,5,8,	4,8,1,
                    8,10,1,	8,3,10,	5,3,8,	5,2,3,	2,7,3,
                    7,10,3,	7,6,10,	7,11,6,	11,0,6,	0,1,6,
                    6,1,10,	9,0,11,	9,11,2,	9,2,5,	7,2,11 })
            );

            // Subdivide icosahedron
            for (uint subdivs = 0; subdivs < subdivions; subdivs++)
            {
                IndexDataPtr refined_indices = std::make_shared<IndexData>();
                refined_indices->reserve(indices->size() * 3);

                for (int i = 0; i < indices->size(); i = i + 3)
                {
                    unsigned int idx1 = (*indices)[i];
                    unsigned int idx2 = (*indices)[i + 1];
                    unsigned int idx3 = (*indices)[i + 2];

                    Vec3 newVtx1((vertices[idx1].x + vertices[idx2].x),
                        (vertices[idx1].y + vertices[idx2].y),
                        (vertices[idx1].z + vertices[idx2].z));
                    newVtx1 = glm::normalize(newVtx1);
                    newVtx1 *= radius;

                    Vec3 newVtx2((vertices[idx2].x + vertices[idx3].x),
                        (vertices[idx2].y + vertices[idx3].y),
                        (vertices[idx2].z + vertices[idx3].z));
                    newVtx2 = glm::normalize(newVtx2);
                    newVtx2 *= radius;

                    Vec3 newVtx3((vertices[idx3].x + vertices[idx1].x),
                        (vertices[idx3].y + vertices[idx1].y),
                        (vertices[idx3].z + vertices[idx1].z));
                    newVtx3 = glm::normalize(newVtx3);
                    newVtx3 *= radius;

                    Vec3 newVtxNrml1((vertices[idx1].nx + vertices[idx2].nx),
                        (vertices[idx1].ny + vertices[idx2].ny),
                        (vertices[idx1].nz + vertices[idx2].nz));
                    newVtxNrml1 = glm::normalize(newVtx1);

                    Vec3 newVtxNrml2((vertices[idx2].nx + vertices[idx3].nx),
                        (vertices[idx2].ny + vertices[idx3].ny),
                        (vertices[idx2].nz + vertices[idx3].nz));
                    newVtxNrml2 = glm::normalize(newVtx2);

                    Vec3 newVtxNrml3((vertices[idx3].nx + vertices[idx1].nx),
                        (vertices[idx3].ny + vertices[idx1].ny),
                        (vertices[idx3].nz + vertices[idx1].nz));
                    newVtxNrml3 = glm::normalize(newVtx3);

                    unsigned int newIdx1 = static_cast<uint>(vertices.size());
                    vertices.push_back(Vertex_pn(newVtx1.x, newVtx1.y, newVtx1.z, newVtxNrml1.x, newVtxNrml1.y, newVtxNrml1.z));

                    unsigned int newIdx2 = newIdx1 + 1;
                    vertices.push_back(Vertex_pn(newVtx2.x, newVtx2.y, newVtx2.z, newVtxNrml2.x, newVtxNrml2.y, newVtxNrml2.z));

                    unsigned int newIdx3 = newIdx2 + 1;
                    vertices.push_back(Vertex_pn(newVtx3.x, newVtx3.y, newVtx3.z, newVtxNrml3.x, newVtxNrml3.y, newVtxNrml3.z));

                    refined_indices->push_back(idx1);
                    refined_indices->push_back(newIdx1);
                    refined_indices->push_back(newIdx3);

                    refined_indices->push_back(newIdx1);
                    refined_indices->push_back(idx2);
                    refined_indices->push_back(newIdx2);

                    refined_indices->push_back(newIdx3);
                    refined_indices->push_back(newIdx1);
                    refined_indices->push_back(newIdx2);

                    refined_indices->push_back(newIdx3);
                    refined_indices->push_back(newIdx2);
                    refined_indices->push_back(idx3);
                }

                indices->clear();

                indices = refined_indices;
            }

            auto layout = std::make_shared<VertexDataDescriptor>(std::vector<GenericVertexLayout>{
                 GenericVertexLayout( 24 , {
                         GenericVertexLayout::Attribute(3, 5126, false, 0), 
                         GenericVertexLayout::Attribute(3, 5126, false, 12)
                     })
            });

            // TOOD avoid this copying stuff...
            //VertexData vertex_data(vertices.begin(),vertices.end());
            //VertexData vertex_data((vertices.size() * 6)*4);
            //std::copy(reinterpret_cast<uint8_t*>(vertices.data()), reinterpret_cast<uint8_t*>(vertices.data()) + (vertices.size() * 6 *4), vertex_data.data());
            auto vertex_data = std::make_shared<VertexData>(VertexData(
                { std::vector<uint8_t>(reinterpret_cast<uint8_t*>(vertices.data()), reinterpret_cast<uint8_t*>(vertices.data()) + (vertices.size() * 6 * 4)) }
            ));

            return std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr>(vertex_data, indices, layout);
        }

        std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr> createBone(float bone_length)
        {
            std::vector<float> vertex_positions;
            std::vector<float> vertex_normals;
            IndexDataPtr indices = std::make_shared<IndexData>();

            int segments = 8;
            float radius = 0.05f * bone_length;
            float height = 1.0f * bone_length;
            float alpha = std::atan(radius / height);

            //TODO create cone base


            // create cone side
            for (int i = 0; i < segments; ++i) {
                float x = 0.0f, y = 0.0f, z = 0.0f;

                x = radius * std::sin(static_cast<float>(i) / static_cast<float>(segments) * 2.0f * 3.14159265359f);
                y = radius * std::cos(static_cast<float>(i) / static_cast<float>(segments) * 2.0f * 3.14159265359f);

                float nx = x, ny = y;
                float nz = std::tan(alpha) * radius;
                float nl = std::sqrt(nx * nx + ny * ny + nz * nz);

                vertex_positions.push_back(x);
                vertex_positions.push_back(y);
                vertex_positions.push_back(z);

                vertex_normals.push_back(nx / nl);
                vertex_normals.push_back(ny / nl);
                vertex_normals.push_back(nz / nl);
            }

            //TODO cone tip vertices
            for (int i = 0; i < segments; ++i) {
                float x = 0.0f, y = 0.0f, z = height;

                float nx = (vertex_normals[i * 3 + 0] + vertex_normals[((i + 1) % segments) * 3 + 0]) / 2.0f;
                float ny = (vertex_normals[i * 3 + 1] + vertex_normals[((i + 1) % segments) * 3 + 1]) / 2.0f;
                float nz = (vertex_normals[i * 3 + 2] + vertex_normals[((i + 1) % segments) * 3 + 2]) / 2.0f;

                vertex_positions.push_back(x);
                vertex_positions.push_back(y);
                vertex_positions.push_back(z);

                vertex_normals.push_back(nx);
                vertex_normals.push_back(ny);
                vertex_normals.push_back(nz);
            }

            for (uint32_t i = 0; i < segments; ++i) {
                indices->push_back(i);
                indices->push_back((i + 1) % segments);
                indices->push_back(i + segments);
            }


            auto layout = std::make_shared<VertexDataDescriptor>(std::vector<GenericVertexLayout>{
                    GenericVertexLayout(12 , { GenericVertexLayout::Attribute(3,5126,false,0) }),
                    GenericVertexLayout(12 , { GenericVertexLayout::Attribute(3,5126,false,0) })
                });

            // TOOD avoid this copying stuff...
            //VertexData vertex_data(vertices.begin(),vertices.end());
            //VertexData vertex_data((vertices.size() * 6)*4);
            //std::copy(reinterpret_cast<uint8_t*>(vertices.data()), reinterpret_cast<uint8_t*>(vertices.data()) + (vertices.size() * 6 *4), vertex_data.data());
            VertexDataPtr vertex_data = std::make_shared< std::vector<std::vector<uint8_t>>>(std::vector<std::vector<uint8_t>>{
                    std::vector<uint8_t>(reinterpret_cast<uint8_t*>(vertex_positions.data()), reinterpret_cast<uint8_t*>(vertex_positions.data()) + (vertex_positions.size() * 3 * 4)),
                    std::vector<uint8_t>(reinterpret_cast<uint8_t*>(vertex_normals.data()), reinterpret_cast<uint8_t*>(vertex_normals.data()) + (vertex_normals.size() * 3 * 4))
                });

            return std::tuple<VertexDataPtr, IndexDataPtr, VertexDataDescriptorPtr>(vertex_data, indices, layout);

        }
    }
}