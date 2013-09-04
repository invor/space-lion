#include "fbx_geometry.hpp"
#include "fbx_property.hpp"

#include <unordered_map>
#include <deque>

namespace FBX {
	static void loadArrayData(std::vector<OpenGL::ColorRGBA> &result, const std::vector<double>& data) {
		if (data.size() % 3 != 0) throw Exception("data length not a multiple of 3");
		result.reserve(data.size() / 3);
		for (size_t i = 0; i < data.size(); i += 3) {
			result.push_back(OpenGL::ColorRGBA(data[i], data[i+1], data[i+2]));
		}
	}

	static void loadArrayData(std::vector<OpenGL::UVCoord> &result, const std::vector<double>& data) {
		if (data.size() % 2 != 0) throw Exception("data length not a multiple of 2");
		result.reserve(data.size() / 2);
		for (size_t i = 0; i < data.size(); i += 2) {
			result.push_back(OpenGL::UVCoord((float) data[i], (float) data[i + 1]));
		}
	}

	static void loadArrayData(std::vector<OpenGL::Vector3> &result, const std::vector<double>& data) {
		if (data.size() % 3 != 0) throw Exception("data length not a multiple of 3");
		result.reserve(data.size() / 3);
		for (size_t i = 0; i < data.size(); i += 3) {
			result.push_back(OpenGL::Vector3((float) data[i], (float) data[i + 1], (float) data[i + 2]));
		}
	}

	template<typename T>
	struct Mapping {
		enum MappingInformationType {
			Empty,
			AllSame,
			ByVertice,
			ByPolygon,
			ByPolygonVertex,
//			ByModel,
//			ByFace,
//			ByEdge,
		};
		enum ReferenceInformationType {
			Direct,
			IndexToDirect,
		};

		static MappingInformationType mappingTypeFromString(const std::string &s) {
			if (s == "AllSame") return AllSame;
			if (s == "ByVertice") return ByVertice; // guessing: eByControlPoint
			if (s == "ByPolygon") return ByPolygon;
			if (s == "ByPolygonVertex") return ByPolygonVertex;
			// ByEdge - how would we do that?
			return Empty;
		}

		static ReferenceInformationType referenceTypeFromString(const std::string &s) {
			if (s == "Direct") return Direct;
			if (s == "IndexToDirect") return IndexToDirect;
			if (s == "Index") return IndexToDirect;
			throw new Exception("Unknown ReferenceInformationType");
		}

		std::vector<T> data;
		std::vector<int32_t> index;
		MappingInformationType mapping;
		ReferenceInformationType reference;

		Mapping() : mapping(Empty), reference(Direct) { }

		T _getIndex(uint32_t ndx) {
			switch (reference) {
			case Direct: return data.at(ndx);
			case IndexToDirect: return data.at((uint32_t) index.at(ndx));
			}
			throw new Exception("internal error");
		}

		T lookup(size_t polyNdx, uint32_t cpNdx) {
			switch (mapping) {
			case Empty:
				return T();
			case AllSame:
				return _getIndex(0);
			case ByVertice:
				return _getIndex(cpNdx);
			case ByPolygon:
				return _getIndex(polyNdx / 3); /* only triangles... */
			case ByPolygonVertex:
				return _getIndex(polyNdx);
			}
			throw new Exception("internal error");
		}

		bool load(Reader &reader, Reader::Node node, NodeName layerName, NodeName dataName) {
			Reader::Node n_layer, n_mappingtype, n_referencetype, n_data;
			if (!reader.find(n_layer, node, layerName)) return false;
			if (!reader.find(n_mappingtype, n_layer, NodeName::MappingInformationType)) return false;
			if (!reader.find(n_referencetype, n_layer, NodeName::ReferenceInformationType)) return false;
			if (!reader.find(n_data, n_layer, dataName)) return false;

			std::vector<double> &raw(n_data.properties.at(0).getDoubles());
			loadArrayData(data, raw);

			mapping = mappingTypeFromString(n_mappingtype.properties.at(0).getString());
			if (Empty == mapping) return false;
			reference = referenceTypeFromString(n_referencetype.properties.at(0).getString());
			if (reference == IndexToDirect) {
				if (!reader.find(n_data, n_layer, NodeName(dataName.toString() + "Index"))) return false;
				index = n_data.properties.at(0).getInt32s();
			}
			return true;
		}
	};

	struct GeometryLoader {
		std::vector<OpenGL::Vector3> controlpoints;
		std::vector< std::vector<uint32_t> > cp_variants_ndx;

		Mapping<OpenGL::Vector3> normals;
		Mapping<OpenGL::Vector3> binormals;
		Mapping<OpenGL::Vector3> tangents;
		Mapping<OpenGL::ColorRGBA> colors;
		Mapping<OpenGL::UVCoord> uvcoords;

		Reader &reader;
		Reader::Node &node;
		Geometry& geometry;
		uint32_t use_features;
		GeometryLoader(Reader &reader, Reader::Node &node, Geometry& geometry, uint32_t use_features)
		: reader(reader), node(node), geometry(geometry), use_features(use_features) { }

		uint32_t add(uint32_t cpNdx, const OpenGL::FullVertex &v) {
			for (uint32_t ndx: cp_variants_ndx.at(cpNdx)) {
				if (geometry.vertices[ndx] == v) return ndx;
			}
			uint32_t ndx = geometry.vertices.size();
			cp_variants_ndx[cpNdx].push_back(ndx);
			geometry.vertices.push_back(v);
			return ndx;
		}

		void load() {
			{
				Reader::Node n_controlpoints;
				if (!reader.find(n_controlpoints, node, NodeName::Vertices)) throw Exception("No Vertices in gemoetry node");
				std::vector<double> &cps(n_controlpoints.properties.at(0).getDoubles());
				if (cps.size() % 3 != 0) throw Exception("vertices length not a multiple of 3");
				controlpoints.reserve(cps.size() / 3);
				for (size_t i = 0; i < cps.size(); i += 3) {
					controlpoints.push_back(OpenGL::Vector3((float) cps[i], (float) cps[i + 1], (float) cps[i + 2]));
				}
				cp_variants_ndx.resize(controlpoints.size());
			}

			uint32_t features = 0;

			Parser::Properties70 props;
			{
				Reader::Node n_properties;
				if (reader.find(n_properties, node, NodeName::Properties70)) {
					props.parse(reader, n_properties);
				}
			}

			if (use_features & Geometry::NORMAL) {
				if (normals.load(reader, node, NodeName::LayerElementNormal, NodeName::Normals)) {
					features |= Geometry::NORMAL;
				}
			}

			if (use_features & Geometry::BINORMAL) {
				if (binormals.load(reader, node, NodeName::LayerElementBinormal, NodeName::Binormals)) {
					features |= Geometry::BINORMAL;
				}
			}

			if (use_features & Geometry::TANGENT) {
				if (tangents.load(reader, node, NodeName::LayerElementTangent, NodeName::Tangents)) {
					features |= Geometry::TANGENT;
				}
			}

			if (use_features & Geometry::UVCOORD) {
				if (uvcoords.load(reader, node, NodeName::LayerElementUV, NodeName::UV)) {
					features |= Geometry::UVCOORD;
				}
			}

			if (use_features & Geometry::COLOR) {
				if (colors.load(reader, node, NodeName::LayerElementColor, NodeName::Colors)) {
					features |= Geometry::COLOR;
				} else {
					Parser::Properties70::const_iterator it = props.properties.find(std::string("Color"));
					if (it != props.properties.end()) {
						geometry.static_color = it->second.value.get<ColorRGB>();
					}
				}
			}

			Reader::Node n_vertices;
			if (!reader.find(n_vertices, node, NodeName::PolygonVertexIndex)) throw Exception("No PolygonVertexIndex in gemoetry node");
			std::vector<int32_t> &polyVertInd(n_vertices.properties.at(0).getInt32s());

			geometry.triangle_indices.reserve(polyVertInd.size());
			for (size_t i = 0; i < polyVertInd.size(); ++i) {
				uint32_t ndx;
				if (i % 3 == 2) { /* 3rd edge */
					if (polyVertInd[i] >= 0) throw Exception("Only triangles are supported");
					ndx = ~(uint32_t) polyVertInd[i]; // negative index means bitwise negated index, and end of (closed) polygon
				} else {
					if (polyVertInd[i] < 0) throw Exception("Only triangles are supported");
					ndx = polyVertInd[i];
				}
				OpenGL::FullVertex vertex;
				vertex.position = controlpoints.at(ndx);
				vertex.normal   = normals.lookup(i, ndx);
				vertex.binormal = binormals.lookup(i, ndx);
				vertex.tangent  = tangents.lookup(i, ndx);
				vertex.color    = colors.lookup(i, ndx);
				vertex.uvcoord  = uvcoords.lookup(i, ndx);
				geometry.triangle_indices.push_back(add(ndx, vertex));
			}

			geometry.features = features;
		}
	};

	Geometry::Geometry(Reader &reader, Reader::Node node, uint32_t use_features) : features(0) {
		GeometryLoader(reader, node, *this, use_features).load();
	}

	Geometry::~Geometry() {
		features = 0;
	}

	Geometry::Geometry() : features(0) {
	}

	std::shared_ptr<Geometry> Geometry::makePlane(uint32_t features) {
		std::shared_ptr<Geometry> g(new Geometry());
		g->features = (Geometry::UVCOORD | Geometry::NORMAL) & features;
		OpenGL::FullVertex v;
		v.normal.set(0.0, 0.0, 1.0);
		v.position.set(-1.0, -1.0, 0.0); v.uvcoord.set(0.0, 0.0); g->vertices.push_back(v);
		v.position.set(-1.0,  1.0, 0.0); v.uvcoord.set(0.0, 1.0); g->vertices.push_back(v);
		v.position.set( 1.0,  1.0, 0.0); v.uvcoord.set(1.0, 1.0); g->vertices.push_back(v);
		v.position.set( 1.0, -1.0, 0.0); v.uvcoord.set(1.0, 0.0); g->vertices.push_back(v);
		static const unsigned int triangles[] = { 0, 2, 1, 2, 0, 3 };
		g->triangle_indices.insert(g->triangle_indices.end(), triangles, triangles + 6);
		return g;
	}

	std::shared_ptr<Geometry> Geometry::makeBox(uint32_t features) {
		std::shared_ptr<Geometry> g(new Geometry());
		g->features = (Geometry::UVCOORD | Geometry::NORMAL | Geometry::TANGENT | Geometry::COLOR) & features;
		OpenGL::FullVertex v;

		/*	front face */
		v.normal.set(0.0,0.0,1.0); v.tangent.set(1.0,0.0,0.0);
		v.position.set(-0.5,-0.5, 0.5); v.color.set(0.0, 0.0, 0.0, 1.0); v.uvcoord.set(0.0,0.0); g->vertices.push_back(v);
		v.position.set(-0.5, 0.5, 0.5); v.color.set(0.0, 1.0, 0.0, 1.0); v.uvcoord.set(0.0,1.0); g->vertices.push_back(v);
		v.position.set( 0.5, 0.5, 0.5); v.color.set(1.0, 1.0, 0.0, 1.0); v.uvcoord.set(1.0,1.0); g->vertices.push_back(v);
		v.position.set( 0.5,-0.5, 0.5); v.color.set(1.0, 0.0, 0.0, 1.0); v.uvcoord.set(1.0,0.0); g->vertices.push_back(v);
		/*	right face */
		v.normal.set(1.0,0.0,0.0); v.tangent.set(0.0,0.0,-1.0);
		v.position.set( 0.5,-0.5, 0.5); v.color.set(1.0, 0.0, 0.0, 1.0); v.uvcoord.set(0.0,0.0); g->vertices.push_back(v);
		v.position.set( 0.5, 0.5, 0.5); v.color.set(1.0, 1.0, 0.0, 1.0); v.uvcoord.set(0.0,1.0); g->vertices.push_back(v);
		v.position.set( 0.5, 0.5,-0.5); v.color.set(1.0, 1.0, 1.0, 1.0); v.uvcoord.set(1.0,1.0); g->vertices.push_back(v);
		v.position.set( 0.5,-0.5,-0.5); v.color.set(1.0, 0.0, 1.0, 1.0); v.uvcoord.set(1.0,0.0); g->vertices.push_back(v);
		/*	left face */
		v.normal.set(-1.0,0.0,0.0); v.tangent.set(0.0,0.0,1.0);
		v.position.set(-0.5,-0.5,-0.5); v.color.set(0.0, 0.0, 1.0, 1.0); v.uvcoord.set(0.0,0.0); g->vertices.push_back(v);
		v.position.set(-0.5, 0.5,-0.5); v.color.set(0.0, 1.0, 1.0, 1.0); v.uvcoord.set(0.0,1.0); g->vertices.push_back(v);
		v.position.set(-0.5, 0.5, 0.5); v.color.set(0.0, 1.0, 0.0, 1.0); v.uvcoord.set(1.0,1.0); g->vertices.push_back(v);
		v.position.set(-0.5,-0.5, 0.5); v.color.set(0.0, 0.0, 0.0, 1.0); v.uvcoord.set(1.0,0.0); g->vertices.push_back(v);
		/*	back face */
		v.normal.set(0.0,0.0,-1.0); v.tangent.set(-1.0,0.0,0.0);
		v.position.set( 0.5,-0.5,-0.5); v.color.set(1.0, 0.0, 1.0, 1.0); v.uvcoord.set(0.0,0.0); g->vertices.push_back(v);
		v.position.set( 0.5, 0.5,-0.5); v.color.set(1.0, 1.0, 1.0, 1.0); v.uvcoord.set(0.0,1.0); g->vertices.push_back(v);
		v.position.set(-0.5, 0.5,-0.5); v.color.set(0.0, 1.0, 1.0, 1.0); v.uvcoord.set(1.0,1.0); g->vertices.push_back(v);
		v.position.set(-0.5,-0.5,-0.5); v.color.set(0.0, 0.0, 1.0, 1.0); v.uvcoord.set(1.0,0.0); g->vertices.push_back(v);
		/*	bottom face */
		v.normal.set(0.0,-1.0,0.0); v.tangent.set(1.0,0.0,0.0);
		v.position.set(-0.5,-0.5, 0.5); v.color.set(0.0, 0.0, 0.0, 1.0); v.uvcoord.set(0.0,0.0); g->vertices.push_back(v);
		v.position.set(-0.5,-0.5,-0.5); v.color.set(0.0, 0.0, 1.0, 1.0); v.uvcoord.set(0.0,1.0); g->vertices.push_back(v);
		v.position.set( 0.5,-0.5,-0.5); v.color.set(1.0, 0.0, 1.0, 1.0); v.uvcoord.set(1.0,1.0); g->vertices.push_back(v);
		v.position.set( 0.5,-0.5, 0.5); v.color.set(1.0, 0.0, 0.0, 1.0); v.uvcoord.set(1.0,0.0); g->vertices.push_back(v);
		/*	top face */
		v.normal.set(0.0,1.0,0.0); v.tangent.set(1.0,0.0,0.0);
		v.position.set(-0.5, 0.5, 0.5); v.color.set(0.0, 1.0, 0.0, 1.0); v.uvcoord.set(0.0,0.0); g->vertices.push_back(v);
		v.position.set(-0.5, 0.5,-0.5); v.color.set(0.0, 1.0, 1.0, 1.0); v.uvcoord.set(0.0,1.0); g->vertices.push_back(v);
		v.position.set( 0.5, 0.5,-0.5); v.color.set(1.0, 1.0, 1.0, 1.0); v.uvcoord.set(1.0,1.0); g->vertices.push_back(v);
		v.position.set( 0.5, 0.5, 0.5); v.color.set(1.0, 1.0, 0.0, 1.0); v.uvcoord.set(1.0,0.0); g->vertices.push_back(v);

		static const unsigned int triangles[] = {
			0,2,1, 2,0,3,
			4,6,5, 6,4,7,
			8,10,9, 10,8,11,
			12,14,13, 14,12,15,
			16,17,18, 18,19,16,
			20,22,21, 22,20,23
		};

		g->triangle_indices.insert(g->triangle_indices.end(), triangles, triangles + 12);
		return g;
	}

	std::shared_ptr<Geometry> Geometry::fbxLoadFirstGeometry(std::string filename, uint32_t features) {
		Reader reader(filename);
		std::unordered_map<int64_t, Reader::Node> objects;
		Reader::NodeChildren n_root = reader.load();
		{
			Reader::Node n_objects;
			if (!reader.find(n_objects, n_root, NodeName::Objects)) throw new Exception("FBX file has no objects");
			Reader::Node n_obj;
			while (reader.next(n_obj, n_objects.children, NodeName::Geometry)) {
				objects.emplace(n_obj.properties.at(0).getInt64(), n_obj);
			}
		}
		int64_t rootobject;
		{
			Reader::Node n_documents, n_document, n_rootnode;
			if (!reader.find(n_documents, n_root, NodeName::Documents)) throw new Exception("FBX file has no documents");
			if (!reader.find(n_document, n_documents, NodeName::Document)) throw new Exception("FBX file has no document");
			if (!reader.find(n_rootnode, n_document, NodeName::RootNode)) throw new Exception("FBX document has no root node");
			rootobject = n_rootnode.properties.at(0).getInt64();
		}
		std::unordered_map<int64_t, std::vector<int64_t> > tree;
		{
			Reader::Node n_connections, n_connection;
			if (!reader.find(n_connections, n_root, NodeName::Connections)) throw new Exception("FBX file has no object connections");
			while (reader.next(n_connection, n_connections.children, NodeName::C)) {
				if (n_connection.properties.at(0).getString() != "OO") continue; /* whatever that could be */
				tree[n_connection.properties.at(2).getInt64()].push_back(n_connection.properties.at(1).getInt64());
			}
		}
		std::deque<int64_t> search;
		search.push_back(rootobject);
		while (!search.empty()) {
			int64_t node = search.front();
			search.pop_front();
			std::unordered_map<int64_t, Reader::Node>::const_iterator obj = objects.find(node);
			if (obj != objects.end()) {
				std::shared_ptr<Geometry> geometry(new Geometry(reader, obj->second, features));
				return geometry;
			}
			std::vector<int64_t> children = tree[node];
			search.insert(search.end(), children.begin(), children.end());
		}
		return std::shared_ptr<Geometry>();
	}
}
