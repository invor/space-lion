#ifndef types_hpp
#define types_hpp

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_AVX
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

typedef glm::mat4 Mat4x4;
typedef glm::vec4 Vec4;
typedef glm::vec3 Vec3;
typedef glm::vec2 Vec2;
typedef glm::quat Quat;

typedef unsigned int uint;

#endif