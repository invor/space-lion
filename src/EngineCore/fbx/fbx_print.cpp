#include "fbx_types.hpp"

namespace FBX {
	void showString(std::ostream& os, const uint8_t* ptr, size_t len) {
		os << '"' << std::hex;
		for (uint32_t i = 0; i < len; ++i) {
			if (ptr[i] >= 32 && ptr[i] < 127) {
				if (ptr[i] != '\\' && ptr[i] != '"') os << (char) ptr[i];
				else os << '\\' << (char) ptr[i];
			}
			else if (ptr[i] < 16) os << "\\x0" << (uint32_t) ptr[i];
			else os << "\\x" << (uint32_t) ptr[i];
		}
		os << '"' << std::dec;
	}
}