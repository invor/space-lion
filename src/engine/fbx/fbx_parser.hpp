#ifndef _FBX_PARSER_H
#define _FBX_PARSER_H _FBX_PARSER_H

#include "fbx_types.hpp"
#include "fbx_generic_parser.hpp"
#include "fbx_node_name.hpp"

namespace FBX {
	namespace ParserResults {
		class P {
		public:
			static const FBX::NodeName name;
			void parse(Reader &reader, const Reader::Node &node) {
			}
		};
	}

	namespace Parser {
		typedef FBX::ChildParser<FBX::ParserResults::P> P;
	}
}

#endif
