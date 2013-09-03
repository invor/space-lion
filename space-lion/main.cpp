#include "src/renderHub.h"
#include "src/ftv_renderHub.h"


int main( void )
{
	//Let's do some tests
	Ftv_RenderHub testRenderer;
	if(testRenderer.init())
	{
		//testRenderer.addScene();
		//testRenderer.setActiveScene(0);
		//testRenderer.run();
		//testRenderer.runVolumeTest();
		//testRenderer.runFtvVolumeTest();
		testRenderer.runTextureAdvectionTest();
		//testRenderer.runInpaintingTest();
		//testRenderer.runFtvGuidanceFieldTest();
	}


	return 0;
}