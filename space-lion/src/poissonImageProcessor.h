#ifndef poissonImageEditing_h
#define poissonImageEditing_h

#include "abstractPostProcessor.h"

class poissonImageProcessor : public abstractPostProcessor
{
public:
	poissonImageProcessor() : abstractPostProcessor() {}
	~poissonImageProcessor() {}

	void render(GLuint inputImage);
protected:
	bool initShaderProgram();
};

#endif