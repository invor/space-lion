#include "src/renderHub.h"


int main( void )
{
	//Let's do some tests
	renderHub testRenderer;
	if(testRenderer.init())
	{
		std::cout<<"Oh my god, why?";
		//testRenderer.addScene();
		//testRenderer.setActiveScene(0);
		//testRenderer.run();
		//testRenderer.runPoissonImageEditing();
	}

	return 0;
}