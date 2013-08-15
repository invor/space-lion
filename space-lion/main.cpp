#include "src/renderHub.h"


int main( void )
{
	//Let's do some tests
	RenderHub testRenderer;
	if(testRenderer.init())
	{
		//testRenderer.addScene();
		//testRenderer.setActiveScene(0);
		testRenderer.run();
		//testRenderer.runVolumeTest();
		//testRenderer.runFtvVolumeTest();
		//testRenderer.runInpaintingTest();
		//testRenderer.runFtvGuidanceFieldTest();
	}


	return 0;
}