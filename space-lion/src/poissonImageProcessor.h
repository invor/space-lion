#ifndef poissonImageEditing_h
#define poissonImageEditing_h

#include "abstractPostProcessor.h"

class poissonImageProcessor : public abstractPostProcessor
{
public:
	poissonImageProcessor() : abstractPostProcessor(), B(0,0,false,false,false) {}
	~poissonImageProcessor() {}

	poissonImageProcessor(int w, int h) : abstractPostProcessor(), B(w,h,true,true,false) {}

	void render(GLuint inputImage);
	void render(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, glm::vec2 lowerBound, glm::vec2 upperBound);
private:
	framebufferObject B;
protected:
	bool initShaderProgram();
};

#endif