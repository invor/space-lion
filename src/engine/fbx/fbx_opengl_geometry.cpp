#include "fbx_opengl_geometry.hpp"

#include <string.h>

namespace FBX {
	namespace OpenGL {
		void GeometrySerialize::serialize(uint8_t *memory, const OpenGL::ColorRGBA& data) {
			memory[0] = data.r; memory[1] = data.g; memory[2] = data.b; memory[3] = data.a;
		}
		void GeometrySerialize::serialize(uint8_t *memory, const OpenGL::UVCoord& data) {
			memcpy(memory                , &data.u, sizeof(float));
			memcpy(memory + sizeof(float), &data.v, sizeof(float));
		}
		void GeometrySerialize::serialize(uint8_t *memory, const OpenGL::Vector3& data) {
			memcpy(memory                  , &data.x, sizeof(float));
			memcpy(memory + sizeof(float)  , &data.y, sizeof(float));
			memcpy(memory + 2*sizeof(float), &data.z, sizeof(float));
		}
		void GeometrySerialize::serialize(uint8_t *memory, const OpenGL::FullVertex& data) {
#define USE_FEATURE(feature, comp) if (m_settings.features & feature) serialize(memory + m_settings.offset_##comp, data.comp);
			serialize(memory + m_settings.offset_position, data.position);
			USE_FEATURE(FBX::Geometry::NORMAL  , normal)
			USE_FEATURE(FBX::Geometry::TANGENT , tangent)
			USE_FEATURE(FBX::Geometry::COLOR   , color)
			USE_FEATURE(FBX::Geometry::UVCOORD , uvcoord)
			USE_FEATURE(FBX::Geometry::BINORMAL, binormal)
#undef USE_FEATURE
		}

		GeometrySerialize::Settings::Settings(uint32_t features) : features(features) {
			offset_position = 0; size_position = 3*sizeof(float);
			size_t offset = size_position;
#define USE_FEATURE(feature, ofs, size) if (features & feature) { offset_##ofs = offset; size_##ofs = size; offset += size; } else { offset_##ofs = size_##ofs = 0; }
			USE_FEATURE(FBX::Geometry::NORMAL, normal, 3*sizeof(float))
			USE_FEATURE(FBX::Geometry::TANGENT, tangent, 3*sizeof(float))
			USE_FEATURE(FBX::Geometry::COLOR, color, 4) // 4 bytes
			USE_FEATURE(FBX::Geometry::UVCOORD, uvcoord, 2*sizeof(float))
			USE_FEATURE(FBX::Geometry::BINORMAL, binormal, 3*sizeof(float))
#undef USE_FEATURE
			stride = offset;
		}

		GeometrySerialize::GeometrySerialize(uint32_t features) : m_settings(features) { }

		void GeometrySerialize::serialize(uint8_t* &memory, size_t &memory_size, const std::vector<OpenGL::FullVertex> &vertices) {
			memory_size = vertices.size() * m_settings.stride;
			memory = new uint8_t[memory_size];
			for (size_t i = 0; i < vertices.size(); ++i) {
				uint8_t *element = memory + m_settings.stride * i;
				serialize(element, vertices[i]);
			}
		}

		uint32_t BindAttribLocations::features() {
			return 0
				| (ndx_normal   >= 0 ? FBX::Geometry::NORMAL   : 0)
				| (ndx_tangent  >= 0 ? FBX::Geometry::TANGENT  : 0)
				| (ndx_color    >= 0 ? FBX::Geometry::COLOR    : 0)
				| (ndx_uvcoord  >= 0 ? FBX::Geometry::UVCOORD  : 0)
				| (ndx_binormal >= 0 ? FBX::Geometry::BINORMAL : 0);
		}

		Geometry::Geometry(uint32_t features, const std::vector<FullVertex> &vertices, const std::vector<unsigned int> &triangle_indices, ColorRGB static_color, BindAttribLocations locations)
		: handle_vertex_array(0), handle_vertex_buffer(0), handle_index_buffer(0), m_serialize(features), ndx_static_color(-1), static_color(static_color)
		{
			init(vertices, triangle_indices);

			bindAttribLocations(locations);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER,0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		Geometry::Geometry(const std::shared_ptr<FBX::Geometry> &geometry, BindAttribLocations locations)
		: handle_vertex_array(0), handle_vertex_buffer(0), handle_index_buffer(0), m_serialize(geometry->features), ndx_static_color(-1), static_color(geometry->static_color)
		{
			init(geometry->vertices, geometry->triangle_indices);

			bindAttribLocations(locations);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER,0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		Geometry::~Geometry() {
			glDeleteVertexArrays(1, &handle_vertex_array);
			glDeleteBuffers(1, &handle_vertex_buffer);
			glDeleteBuffers(1, &handle_index_buffer);
		}

		void Geometry::init(const std::vector<FullVertex> &vertices, const std::vector<unsigned int> &triangle_indices) {
			glGenVertexArrays(1, &handle_vertex_array);
			glGenBuffers(1, &handle_vertex_buffer);
			glGenBuffers(1, &handle_index_buffer);

			uint8_t* memory;
			size_t memory_size;
			m_serialize.serialize(memory, memory_size, vertices);

			glBindVertexArray(handle_vertex_array);
			glBindBuffer(GL_ARRAY_BUFFER, handle_vertex_buffer);
			glBufferData(GL_ARRAY_BUFFER, memory_size, memory, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle_index_buffer);
			if (sizeof(unsigned int) == sizeof(GLuint)) {
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_indices.size() * sizeof(GLuint), triangle_indices.data(), GL_STATIC_DRAW);
			} else {
				std::vector<GLuint> indices(triangle_indices.begin(), triangle_indices.end());
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
			}
			m_vertices = triangle_indices.size();

			delete[] memory;
		}

		void Geometry::bindAttribLocations(const BindAttribLocations &locations) {
			uint32_t f = features();
			const GeometrySerialize::Settings& s(m_serialize.settings());

			if (locations.ndx_position >= 0) {
				glEnableVertexAttribArray(locations.ndx_position);
				glVertexAttribPointer(locations.ndx_position, 3, GL_FLOAT        , GL_FALSE, (GLsizei) s.stride, (void*) s.offset_position);
			}
			if (f & FBX::Geometry::NORMAL && locations.ndx_normal >= 0) {
				glEnableVertexAttribArray(locations.ndx_normal);
				glVertexAttribPointer(locations.ndx_normal  , 3, GL_FLOAT        , GL_FALSE, (GLsizei) s.stride, (void*) s.offset_normal);
			}
			if (f & FBX::Geometry::TANGENT && locations.ndx_tangent >= 0) {
				glEnableVertexAttribArray(locations.ndx_tangent);
				glVertexAttribPointer(locations.ndx_tangent , 3, GL_FLOAT        , GL_FALSE, (GLsizei) s.stride, (void*) s.offset_tangent);
			}
			if (f & FBX::Geometry::COLOR && locations.ndx_color >= 0) {
				glEnableVertexAttribArray(locations.ndx_color);
				glVertexAttribPointer(locations.ndx_color   , 4, GL_UNSIGNED_BYTE, GL_TRUE , (GLsizei) s.stride, (void*) s.offset_color);
				ndx_static_color = -1;
			} else {
				ndx_static_color = locations.ndx_color;
			}
			if (f & FBX::Geometry::UVCOORD && locations.ndx_uvcoord >= 0) {
				glEnableVertexAttribArray(locations.ndx_uvcoord);
				glVertexAttribPointer(locations.ndx_uvcoord , 2, GL_FLOAT        , GL_FALSE, (GLsizei) s.stride, (void*) s.offset_uvcoord);
			}
			if (f & FBX::Geometry::BINORMAL && locations.ndx_binormal >= 0) {
				glEnableVertexAttribArray(locations.ndx_binormal);
				glVertexAttribPointer(locations.ndx_binormal, 3, GL_FLOAT        , GL_FALSE, (GLsizei) s.stride, (void*) s.offset_binormal);
			}
		}

		void Geometry::draw() {
			glBindVertexArray(handle_vertex_array);
			if (ndx_static_color >= 0) {
				glVertexAttrib4f(ndx_static_color, (GLfloat) static_color.red, (GLfloat) static_color.green, (GLfloat) static_color.blue, 0.0f);
			}
			glDrawElements(GL_TRIANGLES, (GLsizei) m_vertices, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}

		std::shared_ptr<Geometry> loadFirstGeometry(std::string filename, BindAttribLocations locations) {
			std::shared_ptr<FBX::Geometry> g = FBX::Geometry::fbxLoadFirstGeometry(filename, locations.features());
			if (!g) return std::shared_ptr<Geometry>();

			std::shared_ptr<Geometry> pgeometry(new Geometry(g, locations));
			return pgeometry;
		}
	}
}
