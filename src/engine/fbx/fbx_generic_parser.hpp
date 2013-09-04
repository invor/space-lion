#ifndef _FBX_GENERIC_PARSER_H
#define _FBX_GENERIC_PARSER_H _FBX_GENERIC_PARSER_H

#include "fbx_types.hpp"
#include "fbx_reader.hpp"

namespace FBX {
	template<typename Storage>
	class ChildParser {
	private:
		bool m_required;
		Storage& m_storage;
	public:
		explicit ChildParser(Storage& storage, bool required = true) : m_required(required), m_storage(storage) { };

		void parse(Reader &reader, const Reader::Node &node) {
			if (node.name == Storage::name) {
				m_required = false;
				m_storage.parse(reader, node);
			}
		}
		void check() {
			if (m_required) throw Exception("missing required child node " + Storage::name.toString());
		}
	};

	namespace internal {
		static inline void parseChildrenNode(Reader &reader, const Reader::Node &node) {
		}

		template<typename Parser, typename... Parsers>
		static inline void parseChildrenNode(Reader &reader, const Reader::Node &node, Parser parser, Parsers... parsers) {
			parser.parse(reader, node);
			parseChildrenNode(reader, node, parsers...);
		}

		static inline void parseChildrenCheck() {
		}

		template<typename Parser, typename... Parsers>
		static inline void parseChildrenCheck(Parser parser, Parsers... parsers) {
			parser.check();
			parseChildrenCheck(parsers...);
		}
	}

	template<typename... Parsers>
	static inline void parseChildren(Reader &reader, Reader::NodeChildren children, Parsers... parsers) {
		Reader::Node node;
		while (reader.next(node, children)) {
			internal::parseChildrenNode(reader, node, parsers...);
		}
		internal::parseChildrenCheck(parsers...);
	}
}

#endif
