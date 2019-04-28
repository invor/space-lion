#ifndef _FBX_OPENGL_GEOMETRY_H
#define _FBX_OPENGL_GEOMETRY_H _FBX_OPENGL_GEOMETRY_H

#include "fbx_types.hpp"
#include "fbx_geometry.hpp"

#include <GL/glew.h>

namespace FBX {
	namespace OpenGL {
		/* serializes only the used parts from FullVertex (defined by features) to memory */
		class GeometrySerialize {
		public:
			struct Settings {
				uint32_t features;
				size_t offset_position, offset_normal, offset_tangent, offset_color, offset_uvcoord, offset_binormal;
				size_t size_position, size_normal, size_tangent, size_color, size_uvcoord, size_binormal;
				size_t stride;
				Settings(uint32_t features);
			};

		private:
			Settings m_settings;

		public:
			/* FBX::Geometry feature constants */
			GeometrySerialize(uint32_t features);
			const Settings& settings() const { return m_settings; }

			/* allocate memory and serialize list of vertices */
			void serialize(uint8_t* &memory, size_t &memory_size, const std::vector<FullVertex> &vertices);

			/* serialize data to allocated memory */
			void serialize(uint8_t* memory, const FullVertex& data); /* serialize only selected parts (by features) */
			static void serialize(uint8_t *memory, const ColorRGBA& data);
			static void serialize(uint8_t *memory, const UVCoord& data);
			static void serialize(uint8_t *memory, const Vector3& data);
		};

		struct BindAttribLocations {
			/* empty mapping taking a fake parameter */
			explicit BindAttribLocations(bool) : ndx_position(-1), ndx_normal(-1), ndx_tangent(-1), ndx_color(-1), ndx_uvcoord(-1), ndx_binormal(-1) { }
			/* default mapping */
			explicit BindAttribLocations() : ndx_position(0), ndx_normal(1), ndx_tangent(2), ndx_color(3), ndx_uvcoord(4), ndx_binormal(5) { }
			GLint ndx_position, ndx_normal, ndx_tangent, ndx_color, ndx_uvcoord, ndx_binormal;
			/* features to load; don't load features with ndx < 0 */
			uint32_t features();
		};
		/* versioned constructor. don't change it, make a new one. */
		inline BindAttribLocations BindAttribLocationsV1(GLint ndx_position, GLint ndx_normal, GLint ndx_tangent, GLint ndx_color, GLint ndx_uvcoord, GLint ndx_binormal) {
			BindAttribLocations l(false);
			l.ndx_position = ndx_position;
			l.ndx_normal   = ndx_normal;
			l.ndx_tangent  = ndx_tangent;
			l.ndx_color    = ndx_color;
			l.ndx_uvcoord  = ndx_uvcoord;
			l.ndx_binormal = ndx_binormal;
			return l;
		}

		class Geometry {
		private:
			GLuint handle_vertex_array;
			GLuint handle_vertex_buffer;
			GLuint handle_index_buffer;

			GeometrySerialize m_serialize;
			size_t m_vertices;

			/* used if features don't include COLOR */
			GLint ndx_static_color;
			ColorRGB static_color;

			/* not copyable */
			Geometry(const Geometry &other);
			Geometry& operator=(const Geometry &other);

			void init(const std::vector<FullVertex> &vertices, const std::vector<unsigned int> &triangle_indices);
			void bindAttribLocations(const BindAttribLocations &locations);
		public:
			/* FBX::Geometry feature constants */
			Geometry(uint32_t features, const std::vector<FullVertex> &vertices, const std::vector<unsigned int> &triangle_indices, ColorRGB static_color = ColorRGB(), BindAttribLocations locations = BindAttribLocations());
			/* need data only temporarily to allocate opengl buffers */
			Geometry(const std::shared_ptr<FBX::Geometry> &geometry, BindAttribLocations locations = BindAttribLocations());
			~Geometry();

			void draw();

			uint32_t features() const { return m_serialize.settings().features; }
		};

		/* usage:
			glBindAttribLocation(program, 0, "position");
			glBindAttribLocation(program, 1, "normal");
			glBindAttribLocation(program, 2, "tangent");
			glBindAttribLocation(program, 3, "color");
			glBindAttribLocation(program, 4, "uvcoord");
			glBindAttribLocation(program, 5, "binormal");
			std::shared_ptr<OpenGL::Geometry> geometry = loadFirstGeometry("path/to/file.fbx", BindAttribLocationsV1(0, 1, 2, 3, 4, 5));
		 */
		std::shared_ptr<Geometry> loadFirstGeometry(std::string filename, BindAttribLocations locations = BindAttribLocations());
	}
}

#endif
