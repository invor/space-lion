#ifndef vertexStructs_h
#define vertexStructs_h

//freeglut and glew
#include <GL/glew.h>

//openGL Math Lib
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//	Basic Vertex with x,y and z component
struct Vertex_p
{
	Vertex_p() : x(0.0), y(0.0), z(0.0) {}
	Vertex_p(float tx, float ty, float tz) : 
		x(tx), y(ty), z(tz) {}
	float x;
	float y;
	float z;
};

struct Vertex_pu : public Vertex_p
{
	Vertex_pu() : Vertex_p(), u(0.0), v(0.0) {}
	Vertex_pu(float tx, float ty, float tz, float tu, float tv) :
		Vertex_p(tx,ty,tz), u(tu), v(tv) {}
	float u;
	float v;
};

struct Vertex_pn : public Vertex_p
{
	Vertex_pn() : Vertex_p(), nx(0.0), ny(0.0), nz(0.0) {}
	Vertex_pn(float tx, float ty, float tz, float tnx, float tny, float tnz) :
		Vertex_p(tx,ty,tz), nx(tnx), ny(tny), nz(tnz) {}
	float nx;
	float ny;
	float nz;
};

struct Vertex_pnu : public Vertex_pn
{
	Vertex_pnu() : Vertex_pn(), u(0.0), v(0.0) {}
	Vertex_pnu(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, float tu, float tv) :
		Vertex_pn(tx,ty,tz,tnx,tny,tnz), u(tu), v(tv) {}
	float u;
	float v;
};

struct Vertex_pnt : public Vertex_pn
{
	Vertex_pnt() : Vertex_pn(), tx(0.0), ty(0.0), tz(0.0) {}
	Vertex_pnt(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz) :
		Vertex_pn(tx,ty,tz,tnx,tny,tnz), tx(ttx), ty(tty), tz(ttz) {}
	float tx;
	float ty;
	float tz;
};

struct Vertex_pntu : public Vertex_pnt
{
	Vertex_pntu() : Vertex_pnt(), u(0.0), v(0.0) {}
	Vertex_pntu(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, float tu, float tv) :
		Vertex_pnt(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz), u(tu), v(tv) {}
	float u;
	float v;
};


struct Vertex_pntc : public Vertex_pnt
{
	Vertex_pntc() : Vertex_pnt(), r(0), g(0), b(0), a(0) {}
	Vertex_pntc(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, GLubyte tr, GLubyte tg, GLubyte tb, GLubyte ta) :
		Vertex_pnt(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz), r(tr), g(tg), b(tb), a(ta) {}
	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;
};

struct Vertex_pntcu : public Vertex_pntc
{
	Vertex_pntcu() : Vertex_pntc(), u(0.0), v(0.0) {}
	Vertex_pntcu(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, GLubyte tr, GLubyte tg, GLubyte tb, GLubyte ta, float tu, float tv) :
		Vertex_pntc(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz,tr,tg,tb,ta), u(tu), v(tv) {}
	float u;
	float v;
};

struct Vertex_pntcub : public Vertex_pntcu
{
	Vertex_pntcub() : Vertex_pntcu(), bx(0.0), by(0.0), bz(0.0) {}
	Vertex_pntcub(float tx, float ty, float tz, float tnx, float tny, float tnz, float ttx, float tty, float ttz, GLubyte tr, GLubyte tg, GLubyte tb, GLubyte ta, float tu, float tv, float tbx, float tby, float tbz) :
		Vertex_pntcu(tx,ty,tz,tnx,tny,tnz,ttx,tty,ttz,tr,tg,tb,ta,tu,tv), bx(tbx), by(tby), bz(tbz) {}
	float bx;
	float by;
	float bz;
};

#endif
