#ifndef poissonImageEditing_h
#define poissonImageEditing_h

#include "abstractPostProcessor.h"

class poissonImageProcessor : public abstractPostProcessor
{
public:
	poissonImageProcessor() : abstractPostProcessor() {}
	~poissonImageProcessor() {}

	void render(GLuint inputImage);
	void render(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations);
private:
	framebufferObject B;
protected:
	bool initShaderProgram();
};

#endif