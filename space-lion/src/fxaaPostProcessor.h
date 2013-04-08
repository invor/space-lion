#ifndef fxaaPostProcessor_h
#define fxaaPostProcessor_h

#include "abstractPostProcessor.h"

class fxaaPostProcessor : public abstractPostProcessor
{
public:
	fxaaPostProcessor() : abstractPostProcessor() {}
	~fxaaPostProcessor() {}

	void render(GLuint inputImage);
	void render(framebufferObject *currentFrame);
protected:
	bool initShaderProgram();
};

#endif