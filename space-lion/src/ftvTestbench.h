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
	/*
	/	This needs a better solution, possibly requiring changes to the framebuffer and
	/	postProcessor class.
	*/
	postProcessor* imageProcessor;

	GLuint images_f[50];
	GLuint images_b[50];

	/*
	/	Methods for reading ppm image files.
	/	Courtesy to the computer vision lecture I attended.
	*/
	bool readPpmHeader(char* filename, long& headerEndPos, int& imgDimX, int& imgDimY);
	bool readPpmData(char* filename, char* imageData, long dataBegin, int imageSize);
public:
	ftvTestbench() {}
	~ftvTestbench() {}

	bool loadImageSequence();

	/*
	/	Reset test instances to inital state.
	*/
	void reset();

	void getImage(GLuint& image, int index);

	void getFrameConfigA(framebufferObject* maskFbo, framebufferObject* imgFbo);

	void getFrameConfigB(framebufferObject* maskFbo, framebufferObject* imgFbo);
};

#endif
