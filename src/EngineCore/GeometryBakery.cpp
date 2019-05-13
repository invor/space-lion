#include "GeometryBakery.hpp"

#include <array>

namespace GeometryBakery
{
	std::tuple<VertexData, IndexData, VertexLayout> createTriangle()
	{
		VertexData vertices(3*6*4); // 3 triangles * 6 float entries * bytesize
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

		VertexLayout layout(24, {VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,12) });

		return std::tuple<VertexData, IndexData, VertexLayout>(vertices,indices,layout);
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
			floatView[(i*18)+0] = (width / 2.0) * width_signs[i];
			floatView[(i*18)+1] = 0.0f;
			floatView[(i*18)+2] = (height / 2.0f) * height_signs[i];
			floatView[(i*18)+3] = 0.0f;
			floatView[(i*18)+4] = 1.0f;
			floatView[(i*18)+5] = 0.0f;
			floatView[(i*18)+6] = 1.0f;
			floatView[(i*18)+7] = 0.0f;
			floatView[(i*18)+8] = 0.0f;
			floatView[(i*18)+9] = 0.5f;
			floatView[(i*18)+10] = 0.5f;
			floatView[(i*18)+11] = 1.0f;
			floatView[(i*18)+12] = 1.0f;
			floatView[(i*18)+13] = (width_signs[i] + 1.0f) / 2.0f;
			floatView[(i*18)+14] = (height_signs[i] + 1.0f) / 2.0f;
			floatView[(i*18)+15] = 0.0f;
			floatView[(i*18)+16] = 0.0f;
			floatView[(i*18)+17] = 1.0f;
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

	std::tuple<VertexData, IndexData, VertexLayout> createBox()
	{
		struct Vertex_pntcub
		{
			Vertex_pntcub(
				float x, float y, float z,
				float nx, float ny, float nz,
				float tx, float ty, float tz,
				uint8_t r, uint8_t g, uint8_t b, uint8_t a,
				float u, float v,
				float bx, float by, float bz)
				: x(x), y(y), z(z), nx(nx), ny(ny), nz(nz), tx(tx), ty(ty), tz(tz), r(r), g(g), b(b), a(a), u(u), v(v), bx(bx), by(by), bz(bz) {}

			float x, y, z;
			float nx, ny, nz;
			float tx, ty, tz;
			uint8_t r, g, b, a;
			float u, v;
			float bx, by, bz;
		};

		/*	if default box not already in list, continue here */
		std::vector<uint8_t> vertices(24*sizeof(Vertex_pntcub));
		std::vector<uint32_t> indices(36);

		Vertex_pntcub* vertexArray = reinterpret_cast<Vertex_pntcub*>(vertices.data());

		/*	front face */
		vertexArray[0] = Vertex_pntcub(-0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		vertexArray[1] = Vertex_pntcub(-0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, 0.0, 1.0, 0.0, 1.0, 0.0);
		vertexArray[2] = Vertex_pntcub(0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, 1.0, 1.0, 0.0, 1.0, 0.0);
		vertexArray[3] = Vertex_pntcub(0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, 1.0, 0.0, 0.0, 1.0, 0.0);
		/*	right face */
		vertexArray[4] = Vertex_pntcub(0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		vertexArray[5] = Vertex_pntcub(0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, 0.0, 1.0, 0.0, 1.0, 0.0);
		vertexArray[6] = Vertex_pntcub(0.5, 0.5, -0.5, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, 1.0, 1.0, 0.0, 1.0, 0.0);
		vertexArray[7] = Vertex_pntcub(0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, 1.0, 0.0, 0.0, 1.0, 0.0);
		/*	left face */
		vertexArray[8] = Vertex_pntcub(-0.5, -0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		vertexArray[9] = Vertex_pntcub(-0.5, 0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, 0.0, 1.0, 0.0, 1.0, 0.0);
		vertexArray[10] = Vertex_pntcub(-0.5, 0.5, 0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, 1.0, 1.0, 0.0, 1.0, 0.0);
		vertexArray[11] = Vertex_pntcub(-0.5, -0.5, 0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, 1.0, 0.0, 0.0, 1.0, 0.0);
		/*	back face */
		vertexArray[12] = Vertex_pntcub(0.5, -0.5, -0.5, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		vertexArray[13] = Vertex_pntcub(0.5, 0.5, -0.5, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, 0.0, 1.0, 0.0, 1.0, 0.0);
		vertexArray[14] = Vertex_pntcub(-0.5, 0.5, -0.5, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, 1.0, 1.0, 0.0, 1.0, 0.0);
		vertexArray[15] = Vertex_pntcub(-0.5, -0.5, -0.5, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, 1.0, 0.0, 0.0, 1.0, 0.0);
		/*	bottom face */
		vertexArray[16] = Vertex_pntcub(-0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 1.0, 0.0, 0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
		vertexArray[17] = Vertex_pntcub(-0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 1.0, 0.0, 0.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, 0.0, 1.0, 0.0, 0.0, 1.0);
		vertexArray[18] = Vertex_pntcub(0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 1.0, 0.0, 0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, 1.0, 1.0, 0.0, 0.0, 1.0);
		vertexArray[19] = Vertex_pntcub(0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 1.0, 0.0, 0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)0.0, (GLubyte)1.0, 1.0, 0.0, 0.0, 0.0, 1.0);
		/*	top face */
		vertexArray[20] = Vertex_pntcub(-0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
		vertexArray[21] = Vertex_pntcub(-0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, (GLubyte)0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, 0.0, 1.0, 0.0, 0.0, 1.0);
		vertexArray[22] = Vertex_pntcub(0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)1.0, 1.0, 1.0, 0.0, 0.0, 1.0);
		vertexArray[23] = Vertex_pntcub(0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, (GLubyte)1.0, (GLubyte)1.0, (GLubyte)0.0, (GLubyte)1.0, 1.0, 0.0, 0.0, 0.0, 1.0);

		indices[0] = 0; indices[1] = 2; indices[2] = 1;
		indices[3] = 2; indices[4] = 0; indices[5] = 3;
		indices[6] = 4; indices[7] = 6; indices[8] = 5;
		indices[9] = 6; indices[10] = 4; indices[11] = 7;
		indices[12] = 8; indices[13] = 10; indices[14] = 9;
		indices[15] = 10; indices[16] = 8; indices[17] = 11;
		indices[18] = 12; indices[19] = 14; indices[20] = 13;
		indices[21] = 14; indices[22] = 12; indices[23] = 15;
		indices[24] = 16; indices[25] = 17; indices[26] = 18;
		indices[27] = 18; indices[28] = 19; indices[29] = 16;
		indices[30] = 20; indices[31] = 22; indices[32] = 21;
		indices[33] = 22; indices[34] = 20; indices[35] = 23;

		VertexLayout layout(60, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
								VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,12),
								VertexLayout::Attribute(GL_FLOAT,3, GL_FALSE, 24), 
								VertexLayout::Attribute(GL_UNSIGNED_BYTE,4, GL_TRUE, 36), 
								VertexLayout::Attribute(GL_FLOAT,2, GL_FALSE, 40), 
								VertexLayout::Attribute(GL_FLOAT,3, GL_FALSE, 48) });

		return std::tuple<VertexData, IndexData, VertexLayout>(vertices, indices, layout);
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
		for (uint subdivs = 0; subdivs<subdivions; subdivs++)
		{
			std::vector<unsigned int> refined_indices;
			refined_indices.reserve(indices.size() * 3);

			for (int i = 0; i<indices.size(); i = i + 3)
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