#ifndef _FBX_TYPES_H
#define _FBX_TYPES_H _FBX_TYPES_H

#include <vector>
#include <cstdint>
#include <cmath>
#include <limits>

namespace FBX {
	typedef std::vector<uint8_t> ByteVector;

	class Vector3D {
	public:
		double x, y, z;

		Vector3D() : x(0), y(0), z(0) { }
		Vector3D(double x, double y, double z) : x(x), y(y), z(z) { }
	};

	class ColorRGB {
	public:
		double red, green, blue;

		ColorRGB() : red(0), green(0), blue(0) { }
		ColorRGB(double red, double green, double blue) : red(red), green(green), blue(blue) { }
	};

	namespace OpenGL {
		inline bool nearlyEqual(float a, float b) {
			const float epsilon = std::numeric_limits<float>::epsilon();
			float absA = fabs(a);
			float absB = fabs(b);
			float diff = fabs(a - b);

			if (a == b) { // shortcut, handles infinities
				return true;
			} else if (a == 0 || b == 0 || diff < epsilon) {
				// a or b is zero or both are extremely close to it
				// relative error is less meaningful here
				return diff < epsilon;
			} else { // use relative error
				return diff / (absA + absB) < epsilon;
			}
		}

		struct ColorRGBA {
			uint8_t r, g, b, a;
			static uint8_t unnormalize(double d) { return (uint8_t) round(d*255); }
			ColorRGBA(double r, double g, double b) : r(unnormalize(r)), g(unnormalize(g)), b(unnormalize(b)), a(0) { }
			ColorRGBA(double r, double g, double b, double a) : r(unnormalize(r)), g(unnormalize(g)), b(unnormalize(b)), a(unnormalize(a)) { }
			ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) { }
			ColorRGBA() : r(0), g(0), b(0), a(0) { }
			void set(double r, double g, double b) { this->r = unnormalize(r); this->g = unnormalize(g); this->b = unnormalize(b); this->a = 0; }
			void set(double r, double g, double b, double a) { this->r = unnormalize(r); this->g = unnormalize(g); this->b = unnormalize(b); this->a = unnormalize(a); }
			void set(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { this->r = r; this->g = g; this->b = b; this->a = a; }

			bool operator==(const ColorRGBA &other) const {
				return r == other.r && g == other.g && b == other.b && a == other.a;
			}
			bool operator!=(const ColorRGBA &other) const { return !(*this == other); }
		};

		struct UVCoord {
			float u, v;
			UVCoord(float u, float v) : u(u), v(v) { }
			UVCoord() : u(0), v(0) { }
			void set(float u, float v) { this->u = u; this->v = v; }
			bool operator==(const UVCoord &other) const {
				return nearlyEqual(u, other.u) && nearlyEqual(v, other.v);
			}
			bool operator!=(const UVCoord &other) const { return !(*this == other); }
		};

		struct Vector3 {
			float x, y, z;
			Vector3(float x, float y, float z) : x(x), y(y), z(z) { }
			Vector3() : x(0), y(0), z(0) { }
			void set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
			bool operator==(const Vector3 &other) const {
				return nearlyEqual(x, other.x) && nearlyEqual(y, other.y) && nearlyEqual(z, other.z);
			}
			bool operator!=(const Vector3 &other) const { return !(*this == other); }
		};

		struct FullVertex {
			Vector3 position;
			Vector3 normal;
			Vector3 binormal;
			Vector3 tangent;
			ColorRGBA color;
			UVCoord uvcoord;
			bool operator==(const FullVertex &other) const {
				return position == other.position
				    && normal   == other.normal
				    && binormal == other.binormal
				    && tangent  == other.tangent
				    && color    == other.color
				    && uvcoord  == other.uvcoord;
			}
			bool operator!=(const FullVertex &other) const { return !(*this == other); }
		};
	}
}

#include "fbx_exceptions.hpp"
#include "fbx_print.hpp"

#endif
