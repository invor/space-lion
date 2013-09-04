
#include "fbx_property.hpp"

namespace FBX {
	namespace Parser {
		const NodeName Properties70::name(NodeName::Properties70);
		void Properties70::parse(Reader &reader, const Reader::Node &node) {
			properties.clear();
			if (!node.properties.empty()) throw new Exception("Properties70 isn't supposed to have direct properties");
			Reader::Node pnode;
			Reader::NodeChildren children = node.children;
			while (reader.next(pnode, children)) {
				if (pnode.name != NodeName::P) throw new Exception("Properties70 can only have P subnodes");
				PropertyValue value;
				std::string key = pnode.properties.at(0).getString();

				value.stype = pnode.properties.at(1).getString();
				value.constructor = pnode.properties.at(2).getString();
				value.flags = pnode.properties.at(3).getString();

				if (value.stype == "bool") {
					value.value.set(!! pnode.properties.at(4).getInt32());
				} else if
					(  value.stype == "Vector3D"
					|| value.stype == "Lcl Translation"
					|| value.stype == "Lcl Rotation"
					|| value.stype == "Lcl Scaling") {
					value.value.set(Vector3D(pnode.properties.at(4).getDouble(), pnode.properties.at(5).getDouble(), pnode.properties.at(6).getDouble()));
				} else if (value.stype == "ColorRGB") {
					value.value.set(ColorRGB(pnode.properties.at(4).getDouble(), pnode.properties.at(5).getDouble(), pnode.properties.at(6).getDouble()));
				} else if (value.stype == "object") {
					// well... no value.
				} else if (pnode.properties.size() != 5) {
					throw Exception("cannot handle unknown Properties70 multi-valued/value-less entry");
				} else {
					value.value = pnode.properties.at(4).value();
				}

				properties.emplace(key, value);
			}
		}

		void Properties70::show(std::ostream& os, const std::string &indent) const {
			for (const std::pair<std::string, Parser::PropertyValue> &prop: properties) {
				os << indent << "+ " << prop.first << "(" + prop.second.stype + "): " << prop.second.value << "\n";
			}
		}
	}
}
