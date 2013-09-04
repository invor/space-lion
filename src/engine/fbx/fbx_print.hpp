#ifndef _FBX_PRINT_H
#define _FBX_PRINT_H _FBX_PRINT_H

#include <iostream>
#include <typeinfo>

namespace FBX {
	void showString(std::ostream& os, const uint8_t* ptr, size_t len);

	template<typename T>
	struct Print {
		static void print(std::ostream &os, const T& data, uint32_t limit) {
			throw Exception("print not implemented for this type");
		}
		static const char* name() {
			return typeid(T).name();
		}
	};

#define PRINT_VECTOR_NAME(X, NAME) \
	template<> struct Print< std::vector<X> > { \
		static void print(std::ostream &os, const std::vector<X>& data, uint32_t limit) { \
			os << data.size() << " * ["; \
			for (size_t ndx = 0, l = data.size(); ndx < l; ++ndx) { \
				if (ndx >= limit) { os << ", ..."; break; } \
				if (ndx != 0) os << ", "; \
				Print< X >::print(os, data[ndx], limit); \
			} \
			os << "]"; \
		} \
		static const char* name() { return NAME; } \
	};

#define PRINT_NAME(X, NAME) \
	template<> struct Print< X > { \
		static void print(std::ostream &os, const X& data, uint32_t limit) { os << data; } \
		static const char* name() { return NAME; } \
	}; \
	PRINT_VECTOR_NAME(X, "[" NAME "]")

PRINT_NAME(float, "Float")
PRINT_NAME(double, "Double")
PRINT_NAME(int64_t, "Int64")
PRINT_NAME(int32_t, "Int32")
PRINT_NAME(int16_t, "Int16")

	template<>
	struct Print<bool> {
		static void print(std::ostream &os, const bool& data, uint32_t limit) {
			os << (data ? "true" : "false");
		}
		static const char* name() { return "Boolean"; }
	};
PRINT_VECTOR_NAME(bool, "Boolean")

	template<>
	struct Print<Vector3D> {
		static void print(std::ostream &os, const Vector3D& data, uint32_t limit) {
			os << "(" << data.x << ", " << data.y << ", " << data.z << ")";
		}
		static const char* name() { return "Vector3D"; }
	};

	template<>
	struct Print<ColorRGB> {
		static void print(std::ostream &os, const ColorRGB& data, uint32_t limit) {
			os << "(" << data.red << ", " << data.green << ", " << data.blue << ")";
		}
		static const char* name() { return "ColorRGB"; }
	};

	template<>
	struct Print<std::string> {
		static void print(std::ostream &os, const std::string& data, uint32_t limit) {
			showString(os, (const uint8_t*) data.c_str(), data.length());
		}
		static const char* name() { return "String"; }
	};

	template<>
	struct Print<ByteVector> {
		static void print(std::ostream &os, const ByteVector& data, uint32_t limit) {
			showString(os, data.data(), data.size());
		}
		static const char* name() { return "Raw"; }
	};

#undef PRINT_VECTOR_NAME
#undef PRINT_NAME

}

#endif
