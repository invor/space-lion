
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <exception>
#include <cstdint>

extern "C" {
	#include <string.h>
}

#include "fbx_reader.hpp"
#include "fbx_unzip.hpp"
#include "fbx_node_property.hpp"
#include "fbx_parser.hpp"
#include "fbx_property.hpp"
#include "fbx_opengl_geometry.hpp"

using namespace FBX;

void read_nested(const std::string indent, Reader& reader, Reader::NodeChildren nodes) {
	Reader::Node node;
	std::string indent2 = indent + "  ";

	while (reader.next(node, nodes)) {
		if (node.name == FBX::NodeName::FBXHeaderExtension) continue;
		if (node.name == FBX::NodeName::FileId) continue;
		if (node.name == FBX::NodeName::CreationTime) continue;
		if (node.name == FBX::NodeName::Creator) continue;

		if (node.name == FBX::NodeName::Properties70) {
			Parser::Properties70 props;
			props.parse(reader, node);
			props.show(std::cout, indent);
			continue;
		}

		std::cout << indent << "Node: " << node.name << "\n";
		for (uint32_t ndx = 0; ndx < node.properties.size(); ++ndx) {
			std::cout << indent << "  - " << node.properties.at(ndx) << "\n";
		}
		//std::cout << node.name << "\n";

		read_nested(indent2, reader, node.children);
	}
}

#include <iomanip>

int main(int argc, char **argv) {
	if (argc < 2) { printf("not enough arguments\n"); return 1; }

/*
	Reader reader(argv[1]);
	uint32_t version;
	Reader::NodeChildren nodes = reader.load(version);
	std::cout << "Version: " << version << "\n";

	read_nested(std::string(""), reader, nodes);
*/

	std::shared_ptr<Geometry> geometry = Geometry::fbxLoadFirstGeometry(argv[1]);

	std::cout.setf(std::ios_base::showpos);
	std::cout << std::setprecision(2) << std::fixed;
	for (OpenGL::FullVertex &v : geometry->vertices) {
		std::cout << "Vertex:"
			<< " Pos(" << v.position.x << ", " << v.position.y << ", " << v.position.z << ")"
			<< " Normal(" << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << ")"
			<< " Binormal(" << v.binormal.x << ", " << v.binormal.y << ", " << v.binormal.z << ")"
			<< " Tangent(" << v.tangent.x << ", " << v.tangent.y << ", " << v.tangent.z << ")"
			<< " UV(" << v.uvcoord.u << ", " << v.uvcoord.v << ")"
//			ColorRGBA color;
			<< "\n";
	}
	std::cout << "Triangles: ";
	for (unsigned int ndx : geometry->triangle_indices) {
		std::cout << ", " << ndx;
	}
	std::cout << "\n";
}
