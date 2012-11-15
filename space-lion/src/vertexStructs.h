#ifndef vertexStructs_h
#define vertexStructs_h

//freeglut and glew
#include <GL/glew.h>

//openGL Math Lib
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Basic Vertex with x,y and z component
struct vertex3
{
	vertex3() : x(0.0), y(0.0), z(0.0) {}
	vertex3(float tx, float ty, float tz) : 
		x(tx), y(ty), z(tz) {}
	float x;
	float y;
	float z;
};

struct vertex5 : public vertex3
{
	vertex5() : vertex3(), u(0.0), v(0.0) {}
	vertex5(float tx, float ty, float tz, float tu, float tv) :
		vertex3(tx,ty,tz), u(tu), v(tv) {}
	float u;
	float v;
};

struct vertex6 : public vertex3
{
	vertex6() : vertex3(), a1(0.0), a2(0.0), a3(0.0) {}
	vertex6(float tx, float ty, float tz, float ta1, float ta2, float ta3) :
		vertex3(tx,ty,tz), a1(ta1), a2(ta2), a3(ta3) {}
	float a1;
	float a2;
	float a3;
};

struct vertex8 : public vertex6
{
	vertex8() : vertex6(), u(0.0), v(0.0) {}
	vertex8(float tx, float ty, float tz, float ta1, float ta2, float ta3, float tu, float tv) :
		vertex6(tx,ty,tz,ta1,ta2,ta3), u(tu), v(tv) {}
	float u;
	float v;
};

struct vertex9 : public vertex6
{
	vertex9() : vertex6(), b1(0.0), b2(0.0), b3(0.0) {}
	vertex9(float tx, float ty, float tz, float ta1, float ta2, float ta3, float tb1, float tb2, float tb3) :
		vertex6(tx,ty,tz,ta1,ta2,ta3), b1(tb1), b2(tb2), b3(tb3) {}
	float b1;
	float b2;
	float b3;
};

struct vertex11 : public vertex9
{
	vertex11() : vertex9(), u(0.0), v(0.0) {}
	vertex11(float tx, float ty, float tz, float ta1, float ta2, float ta3, float tb1, float tb2, float tb3, float tu, float tv) :
		vertex9(tx,ty,tz,ta1,ta2,ta3,tb1,tb2,tb3), u(tu), v(tv) {}
	float u;
	float v;
};

#endif