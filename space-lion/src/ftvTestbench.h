#ifndef ftvTestbench_h
#define ftvTestbench_h

#include <list>

#include "GLSLProgram.h"
#include "postProcessor.h"
#include "GL/glfw.h"

//pragmas seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"opengl32.lib")
#endif

class ftvTestbench
{
private:
	postProcessor imageProcessor;

	GLuint textures_f[50];
	GLuint textures_b[50];

	/*
	/	Methods for reading ppm image files.
	/	Courtesy to the computer vision lecture I attended.
	*/
	bool readPpmHeader(char* filename, long& headerEndPos, int& imgDimX, int& imgDimY);
	bool readPpmData(char* filename, char* imageData, long dataBegin, int imageSize);
public:
	/*
	/	Creating a postProcessor with a set framebuffer resolution should be encouraged for long term use.
	/	This needs a better solution, possibly requiring changes to the framebuffer and postProcessor class.
	*/
	ftvTestbench() : imageProcessor(400,400) {imageProcessor.init();}
	~ftvTestbench() {}

	bool loadImageSequence();

	/*
	/	Reset test instances to inital state.
	*/
	void reset();

	void getTexture(GLuint& handle, int index);

	void getFrameConfigA(framebufferObject* maskFbo, framebufferObject* imgFbo);

	void getFrameConfigB(framebufferObject* maskFbo, framebufferObject* imgFbo);
};

#endif
