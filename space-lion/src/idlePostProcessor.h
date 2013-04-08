#ifndef idlePostProcessor_h
#define idlePostProcessor_h

#include "abstractPostProcessor.h"

class idlePostProcessor : public abstractPostProcessor
{
public:
	idlePostProcessor() : abstractPostProcessor() {}
	~idlePostProcessor() {}

	void render(GLuint inputImage);
	void render(framebufferObject *inputFBO);
protected:
	bool initShaderProgram();
};

#endif