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

	/*
	/	Static array size, since we know how many images we will load.
	*/
	GLuint textures_f[51];
	GLuint textures_b[51];

	int currentFrame;

	/*
	/	Masks for the different test configurations.
	*/
	GLuint maskConfigB;

	/*
	/	Methods for reading ppm image files.
	/	Courtesy to the computer vision lecture I attended.
	*/
	bool readPpmHeader(const char* filename, long& headerEndPos, int& imgDimX, int& imgDimY);
	bool readPpmData(const char* filename, char* imageData, long dataBegin, int imageSize);
public:
	/*
	/	Creating a postProcessor with a set framebuffer resolution should be encouraged for long term use.
	/	This needs a better solution, possibly requiring changes to the framebuffer and postProcessor class.
	*/
	ftvTestbench() : imageProcessor(400,400), currentFrame(0) {imageProcessor.init();}
	~ftvTestbench() {}

	bool loadImageSequence();

	/*
	/	Reset test instances to inital state.
	*/
	void reset() {currentFrame = 0;}

	void initMasks();

	void getTexture(GLuint& handle, int index);

	void getFrameConfigA(framebufferObject* maskFbo, framebufferObject* imgFbo);

	void getFrameConfigB(framebufferObject* maskFbo, framebufferObject* imgFbo);
};

#endif
