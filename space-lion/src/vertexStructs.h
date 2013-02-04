#ifndef vertexStructs_h
#define vertexStructs_h

//freeglut and glew
#include <GL/glew.h>

//openGL Math Lib
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//pragma seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

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
	vertex6() : vertex3(), nx(0.0), ny(0.0), nz(0.0) {}
	vertex6(float tx, float ty, float tz, float tnx, float tny, float tnz) :
		vertex3(tx,ty,tz), nx(tnx), ny(tny), nz(tnz) {}
	float nx;
	float ny;
	float nz;
};

struct vertex8 : public vertex6
{
	vertex8() : vertex6(), u(0.0), v(0.0) {}
	vertex8(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, float tu, float tv) :
		vertex6(tx,ty,tz,tnx,tny,tnz), u(tu), v(tv) {}
	float u;
	float v;
};

struct vertex9 : public vertex6
{
	vertex9() : vertex6(), tx(0.0), ty(0.0), tz(0.0) {}
	vertex9(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz) :
		vertex6(tx,ty,tz,tnx,tny,tnz), tx(ttx), ty(tty), tz(ttz) {}
	float tx;
	float ty;
	float tz;
};

struct vertex13 : public vertex9
{
	vertex13() : vertex9(), r(0), g(0), b(0), a(0) {}
	vertex13(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, GLubyte tr, GLubyte tg, GLubyte tb, GLubyte ta) :
		vertex9(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz), r(tr), g(tg), b(tb), a(ta) {}
	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;
};

struct vertex15 : public vertex13
{
	vertex15() : vertex13(), u(0.0), v(0.0) {}
	vertex15(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, GLubyte tr, GLubyte tg, GLubyte tb, GLubyte ta, float tu, float tv) :
		vertex13(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz,tr,tg,tb,ta), u(tu), v(tv) {}
	float u;
	float v;
};

#endif