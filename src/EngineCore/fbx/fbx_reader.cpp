
#include "fbx_reader.hpp"
#include "fbx_unzip.hpp"

extern "C" {
	#include <string.h>
}

namespace FBX {
	Reader::Reader(const std::string& filename)
	: m_file(filename.c_str(), std::ios_base::in | std::ios_base::binary) {
	}

	static uint64_t fromLE64(uint64_t value) {
		uint16_t endian_test = 0x0201;
		if (1 != *(char*) &endian_test) {
			/* big endian */
			uint64_t newvalue;
			for (unsigned int i = 0; i < sizeof(value); ++i) ((char*)&newvalue)[i] = ((char*)&value)[sizeof(value)-1-i];
			return newvalue;
		}
		return value;
	}

	static uint32_t fromLE32(uint32_t value) {
		uint16_t endian_test = 0x1;
		if (1 != *(char*) &endian_test) {
			/* big endian */
			uint32_t newvalue;
			for (unsigned int i = 0; i < sizeof(value); ++i) ((char*)&newvalue)[i] = ((char*)&value)[sizeof(value)-1-i];
			return newvalue;
		}
		return value;
	}

	static uint16_t fromLE16(uint16_t value) {
		uint16_t endian_test = 0x1;
		if (1 != *(char*) &endian_test) {
			/* big endian */
			uint16_t newvalue;
			for (unsigned int i = 0; i < sizeof(value); ++i) ((char*)&newvalue)[i] = ((char*)&value)[sizeof(value)-1-i];
			return newvalue;
		}
		return value;
	}

	static float fromLEFloat(float value) {
		static const float endian_test = 0x1;
		if (0x80 != ((unsigned char*)&endian_test)[2]) {
			/* big endian */
			float newvalue;
			for (unsigned int i = 0; i < sizeof(value); ++i) ((char*)&newvalue)[i] = ((char*)&value)[sizeof(value)-1-i];
			return newvalue;
		}
		return value;
	}

	static double fromLEDouble(double value) {
		static const double endian_test = 0x1;
		if (0xf0 != ((unsigned char*)&endian_test)[6]) {
			/* big endian */
			double newvalue;
			for (unsigned int i = 0; i < sizeof(value); ++i) ((char*)&newvalue)[i] = ((char*)&value)[sizeof(value)-1-i];
			return newvalue;
		}
		return value;
	}

	static int64_t readInt64(uint8_t *ptr) {
		int64_t i;
		memcpy(&i, ptr, 8);
		return fromLE64(i);
	}
	static int32_t readInt32(uint8_t *ptr) {
		int32_t i;
		memcpy(&i, ptr, 4);
		return fromLE32(i);
	}
	static int16_t readInt16(uint8_t *ptr) {
		int16_t i;
		memcpy(&i, ptr, 2);
		return fromLE16(i);
	}
	static bool readBool(uint8_t *ptr) {
		uint8_t u = *ptr;
		if (u == 0x0) return false;
		if (u == 0x1) return true;
		throw ReaderException("not a boolean value");
		return true;
	}
	static float readFloat(uint8_t *ptr) {
		float f;
		memcpy(&f, ptr, 4);
		return fromLEFloat(f);
	}
	static double readDouble(uint8_t *ptr) {
		double d;
		memcpy(&d, ptr, 8);
		return fromLEDouble(d);
	}

	bool Reader::_fill_buffer(size_t bytes) {
		if (m_buffer.size() < bytes) {
			/* lame overflow protection.. read multiples of 4k for small sizes */
			size_t want = bytes; // (bytes < 1024*1024) ? (bytes + 4095) & ~4095 : bytes;
			size_t have = m_buffer.size();
			m_buffer.resize(want);
			m_file.read((char*) m_buffer.data() + have, want - have);
			m_buffer.resize(have + (size_t) m_file.gcount());
		}
		return m_buffer.size() >= bytes;
	}

	void Reader::_require_bytes(size_t bytes) {
		if (!_fill_buffer(bytes)) throw ReaderException("Unexpected end of file");
	}

	void Reader::seekg(std::streamoff pos) {
		m_buffer.clear();
		m_file.clear();
		m_file.seekg(pos);
	}

	std::streamoff Reader::tellg() {
		return (std::streamoff) m_file.tellg() - (std::streamoff) m_buffer.size();
	}

	std::streamoff Reader::size() {
		std::streamoff cur = m_file.tellg();
		m_file.seekg(0, std::ifstream::end);
		std::streamoff s = m_file.tellg();
		m_file.seekg(cur);
		return s;
	}

	void Reader::skip(size_t bytes) {
		if (bytes >= m_buffer.size()) {
			m_buffer.clear();
		} else {
			m_buffer.erase(m_buffer.begin(),m_buffer.begin()+bytes);
		}
	}

	ByteVector Reader::getMemory(size_t bytes) {
		_require_bytes(bytes);
		ByteVector buf(m_buffer.begin(),m_buffer.begin()+bytes);
		skip(bytes);
		return buf;
	}

	float Reader::getFloat() {
		_require_bytes(4);
		float f = readFloat(m_buffer.data());
		skip(4);
		return f;
	}
	double Reader::getDouble() {
		_require_bytes(8);
		double d = readDouble(m_buffer.data());
		skip(8);
		return d;
	}
	uint64_t Reader::getUInt64() {
		return (uint64_t) getInt64();
	}
	int64_t Reader::getInt64() {
		_require_bytes(8);
		int64_t i = readInt64(m_buffer.data());
		skip(8);
		return i;
	}
	uint32_t Reader::getUInt32() {
		return (uint32_t) getInt32();
	}
	int32_t Reader::getInt32() {
		_require_bytes(4);
		int32_t i = readInt32(m_buffer.data());
		skip(4);
		return i;
	}
	uint16_t Reader::getUInt16() {
		return (uint16_t) getInt16();
	}
	int16_t Reader::getInt16() {
		_require_bytes(2);
		int16_t i = readInt16(m_buffer.data());
		skip(2);
		return i;
	}
	uint8_t Reader::getUInt8() {
		_require_bytes(1);
		uint8_t u = m_buffer[0];
		skip(1);
		return u;
	}
	bool Reader::getBool() {
		uint8_t u = getUInt8();
		if (u == 0x0) return false;
		if (u == 0x1) return true;
		// ? throw ReaderException("not a boolean value");
		return true;
	}
	ByteVector Reader::getRaw() {
		uint32_t len = getUInt32();
		return getMemory(len);
	}
	std::string Reader::getString() {
		uint32_t len = getUInt32();
		_require_bytes(len);
		std::string str((const char*) m_buffer.data(), len);
		skip(len);
		return str;
	}
	std::string Reader::getName() {
		uint8_t len = getUInt8();
		_require_bytes(len);
		std::string name((const char*) m_buffer.data(), len);
		skip(len);
		return name;
	}

	ByteVector Reader::read_property_array(size_t expected_size) {
		uint32_t encoding = getUInt32();
		ByteVector data = getRaw();
		if (0 == encoding) return data;
		if (1 == encoding) return inflate(data, expected_size);
		throw ReaderException("can't decompress array data yet");
	}

	#define READ_ARRAY_PROPERTY(type, reader, elementsize) {\
			uint32_t entries = getUInt32(); \
			ByteVector data = read_property_array(entries * elementsize); \
			if (data.size() != entries * elementsize) throw ReaderException("array data size mismatch"); \
			uint8_t *ptr = data.data(); \
			std::vector<type> list; \
			for (uint32_t ndx = 0; ndx < entries; ++ndx, ptr += elementsize) list.push_back(reader(ptr)); \
			return NodeProperty(list); \
		}

	NodeProperty Reader::getNodeProperty() {
		uint8_t code = getUInt8();
		switch (code) {
		case 'F': return NodeProperty(getFloat());
		case 'D': return NodeProperty(getDouble());
		case 'L': return NodeProperty(getInt64());
		case 'I': return NodeProperty(getInt32());
		case 'Y': return NodeProperty(getInt16());
		case 'C': return NodeProperty(getBool());
		case 'f': READ_ARRAY_PROPERTY(float, readFloat, 4);
		case 'd': READ_ARRAY_PROPERTY(double, readDouble, 8);
		case 'l': READ_ARRAY_PROPERTY(int64_t, readInt64, 8);
		case 'i': READ_ARRAY_PROPERTY(int32_t, readInt32, 4);
		case 'b': READ_ARRAY_PROPERTY(bool, readBool, 1);
		case 'S': return NodeProperty(getString());
		case 'R': return NodeProperty(getRaw());
			break;
		}
		throw ReaderException(std::string("couldn't handle property code ") + std::string(1, (char) code));
	}

	Reader::NodeChildren Reader::load() {
		uint32_t v;
		return load(v);
	}
	Reader::NodeChildren Reader::load(uint32_t& version) {
		seekg(0);
		ByteVector magic = getMemory(23);
		if (0 != memcmp(magic.data(), "Kaydara FBX Binary  \x00\x1a\x00", 23)) throw Exception("Wrong file type");
		version = getUInt32();
		return NodeChildren(27u, size());
	}

	bool Reader::next(Node &child, NodeChildren &children) {
		if (children.pos >= children.end) return false;
		seekg(children.pos);

		std::streamoff node_start = tellg();
		uint32_t end = getUInt32();
		uint32_t n_props = getUInt32();
		uint32_t len_props = getUInt32();
		std::string name = getName();
		child.name = NodeName(name);
		child.properties.clear();
		for (uint32_t i = 0; i < n_props; ++i) child.properties.push_back(getNodeProperty());
		if (tellg() != (std::streamoff) (node_start + len_props + 13 + name.length())) throw ReaderException("property length mismatch");

		if (end == 0 || children.pos + 13 == end) { children.pos = children.end; return false; } /* end = 0 or null node ends list */
		child.children.pos = tellg();
		child.children.end = end;
		children.pos = end;

		return true;
	}

	bool Reader::next(Node &child, NodeChildren &children, NodeName name) {
		Node c;
		while (next(c, children)) {
			if (c.name == name) {
				child = c;
				return true;
			}
		}
		return false;
	}
	bool Reader::find(Node &child, NodeChildren list, NodeName name) {
		Node c;
		while (next(c, list)) {
			if (c.name == name) {
				child = c;
				return true;
			}
		}
		return false;
	}

	bool Reader::find(Node &child, Node parent, NodeName name) {
		return find(child, parent.children, name);
	}

}
