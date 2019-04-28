#ifndef _FBX_GEOMETRY_H
#define _FBX_GEOMETRY_H _FBX_GEOMETRY_H

#include "fbx_types.hpp"
#include "fbx_reader.hpp"

namespace FBX {
	class Geometry {
	public:
		/* Feature selection: use | to select multiple features */
		const static uint32_t
			NORMAL    = 0x01,
			TANGENT   = 0x02,
			COLOR     = 0x04,
			UVCOORD   = 0x08,
			BINORMAL  = 0x10,
			ALL       = 0x1f;

		Geometry();
		Geometry(Reader &reader, Reader::Node node, uint32_t use_features = ALL);
		~Geometry();

		/* Plane: (-1,-1,0)x(1,1,0); texture mapping (x,y,0) -> 0.5*(x+1, y+1) */
		static std::shared_ptr<Geometry> makePlane(uint32_t features = ALL);

		/* (-0.5,-0.5,-0.5)x(0.5,0.5,0.5) box with normals, tangents, colors and uvcoords */
		static std::shared_ptr<Geometry> makeBox(uint32_t features = ALL);

		static std::shared_ptr<Geometry> fbxLoadFirstGeometry(std::string filename, uint32_t features = ALL);
	private:
		/* not copyable */
		Geometry(const Geometry& other);
		Geometry& operator=(const Geometry& other);

	public:
		uint32_t features;
		std::vector<OpenGL::FullVertex> vertices;
		std::vector<unsigned int> triangle_indices;
		ColorRGB static_color;
	};
}

#endif
