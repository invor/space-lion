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

//	Basic Vertex with x,y and z component
struct vertex_p
{
	vertex_p() : x(0.0), y(0.0), z(0.0) {}
	vertex_p(float tx, float ty, float tz) : 
		x(tx), y(ty), z(tz) {}
	float x;
	float y;
	float z;
};

struct vertex_pu : public vertex_p
{
	vertex_pu() : vertex_p(), u(0.0), v(0.0) {}
	vertex_pu(float tx, float ty, float tz, float tu, float tv) :
		vertex_p(tx,ty,tz), u(tu), v(tv) {}
	float u;
	float v;
};

struct vertex_pn : public vertex_p
{
	vertex_pn() : vertex_p(), nx(0.0), ny(0.0), nz(0.0) {}
	vertex_pn(float tx, float ty, float tz, float tnx, float tny, float tnz) :
		vertex_p(tx,ty,tz), nx(tnx), ny(tny), nz(tnz) {}
	float nx;
	float ny;
	float nz;
};

struct vertex_pnu : public vertex_pn
{
	vertex_pnu() : vertex_pn(), u(0.0), v(0.0) {}
	vertex_pnu(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, float tu, float tv) :
		vertex_pn(tx,ty,tz,tnx,tny,tnz), u(tu), v(tv) {}
	float u;
	float v;
};

struct vertex_pnt : public vertex_pn
{
	vertex_pnt() : vertex_pn(), tx(0.0), ty(0.0), tz(0.0) {}
	vertex_pnt(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz) :
		vertex_pn(tx,ty,tz,tnx,tny,tnz), tx(ttx), ty(tty), tz(ttz) {}
	float tx;
	float ty;
	float tz;
};

struct vertex_pntu : public vertex_pnt
{
	vertex_pntu() : vertex_pnt(), u(0.0), v(0.0) {}
	vertex_pntu(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, float tu, float tv) :
		vertex_pnt(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz), u(tu), v(tv) {}
	float u;
	float v;
};


struct vertex_pntc : public vertex_pnt
{
	vertex_pntc() : vertex_pnt(), r(0), g(0), b(0), a(0) {}
	vertex_pntc(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, GLubyte tr, GLubyte tg, GLubyte tb, GLubyte ta) :
		vertex_pnt(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz), r(tr), g(tg), b(tb), a(ta) {}
	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;
};

struct vertex_pntcu : public vertex_pntc
{
	vertex_pntcu() : vertex_pntc(), u(0.0), v(0.0) {}
	vertex_pntcu(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, GLubyte tr, GLubyte tg, GLubyte tb, GLubyte ta, float tu, float tv) :
		vertex_pntc(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz,tr,tg,tb,ta), u(tu), v(tv) {}
	float u;
	float v;
};

struct vertex_pntcub : public vertex_pntcu
{
	vertex_pntcub() : vertex_pntcu(), bx(0.0), by(0.0), bz(0.0) {}
	vertex_pntcub(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, GLubyte tr, GLubyte tg, GLubyte tb, GLubyte ta, float tu, float tv, float tbx, float tby, float tbz) :
		vertex_pntcu(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz,tr,tg,tb,ta,tu,tv), bx(tbx), by(tby), bz(tbz) {}
	float bx;
	float by;
	float bz;
};

#endif
