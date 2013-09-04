#ifndef _FBX_READER_H
#define _FBX_READER_H _FBX_READER_H

#include <fstream>

#include "fbx_types.hpp"
#include "fbx_node_property.hpp"
#include "fbx_node_name.hpp"

namespace FBX {

	/* not thread-safe! */
	class Reader {
	public:
		class NodeChildren {
		private:
			std::streamoff pos, end;
			friend class Reader;
			NodeChildren(std::streamoff pos, std::streamoff end) : pos(pos), end(end) { }
		public:
			NodeChildren() : pos(0), end(0) {}
		};
		class Node {
		public:
			NodeName name;
			std::vector<NodeProperty> properties;
			NodeChildren children;
		};

		explicit Reader(const std::string& filename);

		NodeChildren load();
		NodeChildren load(uint32_t& version);

		/* returns next child node in children list */
		bool next(Node &child, NodeChildren &children);
		bool next(Node &child, NodeChildren &children, NodeName name);
		bool find(Node &child, NodeChildren list, NodeName name);
		bool find(Node &child, Node parent, NodeName name);

	private:
		explicit Reader(const Reader &other);
		Reader& operator =(const Reader &other);

		std::ifstream m_file;
		ByteVector m_buffer;

		bool _fill_buffer(size_t bytes);
		void _require_bytes(size_t bytes);

		ByteVector read_property_array(size_t expected_size);

		void seekg(std::streamoff pos);
		std::streamoff tellg();
		std::streamoff size();
		void skip(size_t bytes);

		ByteVector getMemory(size_t bytes);

		float getFloat();
		double getDouble();
		uint64_t getUInt64();
		int64_t getInt64();
		uint32_t getUInt32();
		int32_t getInt32();
		uint16_t getUInt16();
		int16_t getInt16();
		uint8_t getUInt8();
		bool getBool();
		ByteVector getRaw(); /* uint32 length + data */
		std::string getString(); /* uint32 length + data */
		std::string getName(); /* uint8 length + data */

		NodeProperty getNodeProperty();
	};
}

#endif
