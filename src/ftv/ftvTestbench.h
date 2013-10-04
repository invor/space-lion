#ifndef ftvTestbench_h
#define ftvTestbench_h

#include <list>
#include <fstream>

#include "../engine/core/GLSLProgram.h"
#include "ftv_postProcessor.h"
#include "GLFW/glfw3.h"

#define _CRT_SECURE_NO_WARNINGS

class FtvTestbench
{
private:
	/*	Since it's only a testbench, it has it's own postProcessor and resourceManager */
	Ftv_PostProcessor imageProcessor;
	Ftv_ResourceManager resourceMngr;

	/*
	/	Static array size, since we know how many images we will load.
	*/
	GLuint textures_f[51];
	GLuint textures_b[51];
	GLuint textures_v[51];

	/*	Single 3D texture to store all vector fields */
	GLuint vector_fields;

	int currentFrame;

	/*
	/	Masks for the different test configurations.
	*/
	GLuint maskConfigB;
	GLuint maskConfigC_1;
	GLuint maskConfigC_2;
	GLuint maskConfigC_3;

	/*
	/	Methods for reading ppm image files.
	/	Courtesy to the computer vision lecture I attended.
	*/
	bool readPpmHeader(const char* filename, long& headerEndPos, int& imgDimX, int& imgDimY);
	bool readPpmData(const char* filename, char* imageData, long dataBegin, int imgDimX, int imgDimY);

	/**
	 * \brief Reads float-valued image data from a raw file
	 * \param filename Location of the raw file
	 * \param imageData Array the image data will be stored in
	 * \param size Length of the imageData array and size of the file 
	 * \return Returns true if file was succesfully read, false otherwise
	 */
	bool readRawImageF(const char* filename, float* imageData, int size);
public:
	/*
	/	Creating a postProcessor with a set framebuffer resolution shouldn't be encouraged for long term use.
	/	This needs a better solution, possibly requiring changes to the framebuffer and postProcessor class.
	*/
	FtvTestbench() : imageProcessor(400,400), currentFrame(0) {imageProcessor.ftv_init(&resourceMngr);}
	~FtvTestbench() {}

	bool loadImageSequence();

	bool loadVectorFieldSequence();

	bool loadVectorFieldSequenceTo3D(int from, int to);

	/*
	/	Reset test instances to inital state.
	*/
	void reset() {currentFrame = 0;}

	void initMasks();

	void getForwardTexture(GLuint& handle, int index);
	void getBackwardTexture(GLuint& handle, int index);
	void getVectorTexture(GLuint& handle, int index);
	void getVectorTexture3D(GLuint& handle);

	/*
	/
	*/
	void getFrameConfigA(FramebufferObject* maskFbo, FramebufferObject* imgFbo);

	/*
	/	Return (with successive method calls) all 51 images (from the f batch).
	/	Inpainting region is static. (Kind of a worst case scenario for possion...)
	*/
	void getFrameConfigB(FramebufferObject* maskFbo, FramebufferObject* imgFbo);

	/*
	/	Return (with successive method calls) all 51 images (from the f batch).
	/	Inpainting region alternates between 3 different masks.
	*/
	void getFrameConfigC(FramebufferObject* maskFbo, FramebufferObject* imgFbo);
};

#endif
