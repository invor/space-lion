#include "GeometryBakery.hpp"

#include <array>


namespace EngineCore
{
    namespace Graphics
    {
        std::tuple<VertexData, IndexData, VertexLayout> createTriangle()
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

            VertexLayout layout(24, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,12) });

            return std::tuple<VertexData, IndexData, VertexLayout>(vertices, indices, layout);
        }

        std::tuple<VertexData, IndexData, VertexLayout> createPlane(float width, float height)
        {
            VertexData vertices(4 * 18 * 4); // 4 vertices * 18 float entries * bytesize
            IndexData indices(6);

            float* floatView = reinterpret_cast<float*>(vertices.data());

            std::array<float, 4> width_signs = { -1.0f,1.0f,1.0f,-1.0f };
            std::array<float, 4> height_signs = { -1.0f,-1.0f,1.0f,1.0f };

            for (int i = 0; i < 4; ++i)
            {
                floatView[(i * 18) + 0] = (width / 2.0) * width_signs[i];
                floatView[(i * 18) + 1] = 0.0f;
                floatView[(i * 18) + 2] = (height / 2.0f) * height_signs[i];
                floatView[(i * 18) + 3] = 0.0f;
                floatView[(i * 18) + 4] = 1.0f;
                floatView[(i * 18) + 5] = 0.0f;
                floatView[(i * 18) + 6] = 1.0f;
                floatView[(i * 18) + 7] = 0.0f;
                floatView[(i * 18) + 8] = 0.0f;
                floatView[(i * 18) + 9] = 0.5f;
                floatView[(i * 18) + 10] = 0.5f;
                floatView[(i * 18) + 11] = 1.0f;
                floatView[(i * 18) + 12] = 1.0f;
                floatView[(i * 18) + 13] = (width_signs[i] + 1.0f) / 2.0f;
                floatView[(i * 18) + 14] = (height_signs[i] + 1.0f) / 2.0f;
                floatView[(i * 18) + 15] = 0.0f;
                floatView[(i * 18) + 16] = 0.0f;
                floatView[(i * 18) + 17] = 1.0f;
            }

            indices[0] = 0; indices[1] = 3; indices[2] = 1; indices[3] = 3; indices[4] = 2; indices[5] = 1;

            VertexLayout layout(72, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
                VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,12),
                VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,24),
                VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,36),
                VertexLayout::Attribute(GL_FLOAT,2,GL_FALSE,52),
                VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,60) });

            return std::tuple<VertexData, IndexData, VertexLayout>(vertices, indices, layout);
        }

        std::tuple<VertexDataPtr, IndexDataPtr, VertexLayoutPtr> createBox()
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

            std::vector<uint8_t> positions(24 * sizeof(Vec3));
            std::vector<uint8_t> normals(24 * sizeof(Vec3));
            std::vector<uint8_t> tangents(24 * sizeof(Vec3));
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

            access_ptr = reinterpret_cast<Vec3*>(tangents.data());
            /*	front face */
            access_ptr[0] = { 1.0, 0.0, 0.0 }; access_ptr[1] = { 1.0, 0.0, 0.0 };
            access_ptr[2] = { 1.0, 0.0, 0.0 }; access_ptr[3] = { 1.0, 0.0, 0.0 };
            /*	right face */
            access_ptr[4] = { 0.0, 0.0, -1.0 }; access_ptr[5] = { 0.0, 0.0, -1.0 };
            access_ptr[6] = { 0.0, 0.0, -1.0 }; access_ptr[7] = { 0.0, 0.0, -1.0 };
            /*	left face */
            access_ptr[8] = { 0.0, 0.0, 1.0 }; access_ptr[9] = { 0.0, 0.0, 1.0 };
            access_ptr[10] = { 0.0, 0.0, 1.0 }; access_ptr[11] = { 0.0, 0.0, 1.0 };
            /*	back face */
            access_ptr[12] = { -1.0, 0.0, 0.0 }; access_ptr[13] = { -1.0, 0.0, 0.0 };
            access_ptr[14] = { -1.0, 0.0, 0.0 }; access_ptr[15] = { -1.0, 0.0, 0.0 };
            /*	bottom face */
            access_ptr[16] = { 1.0, 0.0, 0.0 }; access_ptr[17] = { 1.0, 0.0, 0.0 };
            access_ptr[18] = { 1.0, 0.0, 0.0 }; access_ptr[19] = { 1.0, 0.0, 0.0 };
            /*	top face */
            access_ptr[20] = { 1.0, 0.0, 0.0 }; access_ptr[21] = { 1.0, 0.0, 0.0 };
            access_ptr[22] = { 1.0, 0.0, 0.0 }; access_ptr[23] = { 1.0, 0.0, 0.0 };

            Color* col_access_ptr = reinterpret_cast<Color*>(colors.data());
            /*	front face */
            col_access_ptr[0] = { (GLubyte)0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0 }; col_access_ptr[1] = { (GLubyte)0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0 };
            col_access_ptr[2] = { (GLubyte)1.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0 }; col_access_ptr[3] = { (GLubyte)1.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0 };
            /*	right face */
            col_access_ptr[4] = { (GLubyte)1.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0 }; col_access_ptr[5] = { (GLubyte)1.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0 };
            col_access_ptr[6] = { (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0 }; col_access_ptr[7] = { (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0 };
            /*	left face */
            col_access_ptr[8] = { (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0 }; col_access_ptr[9] = { (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0 };
            col_access_ptr[10] = { (GLubyte)0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0 }; col_access_ptr[11] = { (GLubyte)0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0 };
            /*	back face */
            col_access_ptr[12] = { (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0 }; col_access_ptr[13] = { (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0 };
            col_access_ptr[14] = { (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0 }; col_access_ptr[15] = { -(GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0 };
            /*	bottom face */
            col_access_ptr[16] = { (GLubyte)0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0 }; col_access_ptr[17] = { (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0 };
            col_access_ptr[18] = { (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0 }; col_access_ptr[19] = { (GLubyte)1.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0 };
            /*	top face */
            col_access_ptr[20] = { (GLubyte)0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0 }; col_access_ptr[21] = { (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0 };
            col_access_ptr[22] = { (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0 }; col_access_ptr[23] = { (GLubyte)1.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0 };

            Vec2* uv_access_ptr = reinterpret_cast<Vec2*>(uv_coords.data());
            /*	front face */
            access_ptr[0] = { 0.0, 0.0 }; access_ptr[1] = { 0.0, 1.0 };
            access_ptr[2] = { 1.0, 1.0 }; access_ptr[3] = { 1.0, 0.0 };
            /*	right face */
            access_ptr[4] = { 0.0, 0.0 }; access_ptr[5] = { 0.0, 1.0 };
            access_ptr[6] = { 1.0, 1.0 }; access_ptr[7] = { 1.0, 0.0 };
            /*	left face */
            access_ptr[8] = { 0.0, 0.0 }; access_ptr[9] = { 0.0, 1.0 };
            access_ptr[10] = { 1.0, 1.0 }; access_ptr[11] = { 1.0, 0.0 };
            /*	back face */
            access_ptr[12] = { 0.0, 0.0 }; access_ptr[13] = { 0.0, 1.0 };
            access_ptr[14] = { 1.0, 1.0 }; access_ptr[15] = { 1.0, 0.0 };
            /*	bottom face */
            access_ptr[16] = { 0.0, 0.0 }; access_ptr[17] = { 0.0, 1.0 };
            access_ptr[18] = { 1.0, 1.0 }; access_ptr[19] = { 1.0, 0.0 };
            /*	top face */
            access_ptr[20] = { 0.0, 0.0 }; access_ptr[21] = { 0.0, 1.0 };
            access_ptr[22] = { 1.0, 1.0 }; access_ptr[23] = { 1.0, 0.0 };

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

            auto layout = std::make_shared< VertexLayout>(60, 
                std::vector<VertexLayout::Attribute>{ VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
                                    VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,12),
                                    VertexLayout::Attribute(GL_FLOAT,3, GL_FALSE, 24),
                                    VertexLayout::Attribute(GL_UNSIGNED_BYTE,4, GL_TRUE, 36),
                                    VertexLayout::Attribute(GL_FLOAT,2, GL_FALSE, 40),
                                    VertexLayout::Attribute(GL_FLOAT,3, GL_FALSE, 48) });

            auto vertices = std::make_shared< std::vector <std::vector<uint8_t>>>( 
                std::vector<std::vector<uint8_t>>{ positions, normals, tangents, colors, uv_coords, bitangents });

            return std::tuple<VertexDataPtr, IndexDataPtr, VertexLayoutPtr>(vertices, indices, layout);
        }

        std::tuple<VertexData, IndexData, VertexLayout> createIcoSphere(uint subdivions)
        {
            // Create intial icosahedron
            GLfloat x = 0.525731112119133606f;
            GLfloat z = 0.850650808352039932f;

            struct Vertex_pn
            {
                Vertex_pn(float x, float y, float z, float nx, float ny, float nz)
                    : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz) {}

                float x, y, z;
                float nx, ny, nz;
            };

            std::vector<Vertex_pn> vertices({ Vertex_pn(-x,0.0f,z,0.0f,0.0f,0.0f),
                Vertex_pn(x,0.0f,z,0.0f,0.0f,0.0f),
                Vertex_pn(-x,0.0f,-z,0.0f,0.0f,0.0f),
                Vertex_pn(x,0.0f,-z,0.0f,0.0f,0.0f),
                Vertex_pn(0.0f,z,x,0.0f,0.0f,0.0f),
                Vertex_pn(0.0f,z,-x,0.0f,0.0f,0.0f),
                Vertex_pn(0.0f,-z,x,0.0f,0.0f,0.0f),
                Vertex_pn(0.0f,-z,-x,0.0f,0.0f,0.0f),
                Vertex_pn(z,x,0.0f,0.0f,0.0f,0.0f),
                Vertex_pn(-z,x,0.0f,0.0f,0.0f,0.0f),
                Vertex_pn(z,-x,0.0f,0.0f,0.0f,0.0f),
                Vertex_pn(-z,-x,0.0f,0.0f,0.0f,0.0f) });
            std::vector<unsigned int> indices({ 0,4,1,	0,9,4,	9,5,4,	4,5,8,	4,8,1,
                8,10,1,	8,3,10,	5,3,8,	5,2,3,	2,7,3,
                7,10,3,	7,6,10,	7,11,6,	11,0,6,	0,1,6,
                6,1,10,	9,0,11,	9,11,2,	9,2,5,	7,2,11 });

            // Subdivide icosahedron
            for (uint subdivs = 0; subdivs < subdivions; subdivs++)
            {
                std::vector<unsigned int> refined_indices;
                refined_indices.reserve(indices.size() * 3);

                for (int i = 0; i < indices.size(); i = i + 3)
                {
                    unsigned int idx1 = indices[i];
                    unsigned int idx2 = indices[i + 1];
                    unsigned int idx3 = indices[i + 2];

                    Vec3 newVtx1((vertices[idx1].x + vertices[idx2].x),
                        (vertices[idx1].y + vertices[idx2].y),
                        (vertices[idx1].z + vertices[idx2].z));
                    newVtx1 = glm::normalize(newVtx1);

                    Vec3 newVtx2((vertices[idx2].x + vertices[idx3].x),
                        (vertices[idx2].y + vertices[idx3].y),
                        (vertices[idx2].z + vertices[idx3].z));
                    newVtx2 = glm::normalize(newVtx2);

                    Vec3 newVtx3((vertices[idx3].x + vertices[idx1].x),
                        (vertices[idx3].y + vertices[idx1].y),
                        (vertices[idx3].z + vertices[idx1].z));
                    newVtx3 = glm::normalize(newVtx3);

                    unsigned int newIdx1 = static_cast<uint>(vertices.size());
                    vertices.push_back(Vertex_pn(newVtx1.x, newVtx1.y, newVtx1.z, 0.0, 0.0, 0.0));

                    unsigned int newIdx2 = newIdx1 + 1;
                    vertices.push_back(Vertex_pn(newVtx2.x, newVtx2.y, newVtx2.z, 0.0, 0.0, 0.0));

                    unsigned int newIdx3 = newIdx2 + 1;
                    vertices.push_back(Vertex_pn(newVtx3.x, newVtx3.y, newVtx3.z, 0.0, 0.0, 0.0));

                    refined_indices.push_back(idx1);
                    refined_indices.push_back(newIdx1);
                    refined_indices.push_back(newIdx3);

                    refined_indices.push_back(newIdx1);
                    refined_indices.push_back(idx2);
                    refined_indices.push_back(newIdx2);

                    refined_indices.push_back(newIdx3);
                    refined_indices.push_back(newIdx1);
                    refined_indices.push_back(newIdx2);

                    refined_indices.push_back(newIdx3);
                    refined_indices.push_back(newIdx2);
                    refined_indices.push_back(idx3);
                }

                indices.clear();

                indices = refined_indices;
            }

            VertexLayout layout(24, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,12) });

            // TOOD avoid this copying stuff...
            //VertexData vertex_data(vertices.begin(),vertices.end());
            //VertexData vertex_data((vertices.size() * 6)*4);
            //std::copy(reinterpret_cast<uint8_t*>(vertices.data()), reinterpret_cast<uint8_t*>(vertices.data()) + (vertices.size() * 6 *4), vertex_data.data());
            VertexData vertex_data(reinterpret_cast<uint8_t*>(vertices.data()), reinterpret_cast<uint8_t*>(vertices.data()) + (vertices.size() * 6 * 4));

            return std::tuple<VertexData, IndexData, VertexLayout>(vertex_data, indices, layout);
        }
    }
}