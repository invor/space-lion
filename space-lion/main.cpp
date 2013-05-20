#include "src/renderHub.h"


int main( void )
{
	//Let's do some tests
	renderHub testRenderer;
	if(testRenderer.init())
	{
		testRenderer.addScene();
		testRenderer.setActiveScene(0);
		testRenderer.run();
		//testRenderer.runFtv();
	}


	return 0;
}