#ifndef _FBX_PROPERTY_H
#define _FBX_PROPERTY_H _FBX_PROPERTY_H

#include <unordered_map>

#include "fbx_types.hpp"
#include "fbx_generic_parser.hpp"
#include "fbx_value.hpp"

namespace FBX {
	namespace Parser {
		class PropertyValue {
		public:
			std::string stype;
			std::string constructor;
			std::string flags;
			Value value;
		};

		class Properties70 {
		public:
			std::unordered_map<std::string, PropertyValue> properties;
			typedef std::unordered_map<std::string, PropertyValue>::const_iterator const_iterator;

			static const NodeName name;
			void parse(Reader &reader, const Reader::Node &node);

			void show(std::ostream& os, const std::string &indent) const;
		};

		typedef FBX::ChildParser<Properties70> Properties70_Parser;
	}
}


#endif
