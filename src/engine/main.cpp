#include <thread>
#include <vector>
#include "core/renderHub.h"

using namespace std;

int main(){

	
	RenderHub testRenderer;
	//if(testRenderer.init())
	//{
	//	testRenderer.addScene();
	//	testRenderer.setActiveScene(0);
	//	testReceiver->pushLoadSceneMessages();
	//	std::thread renderThread(&RenderHub::run,&testRenderer);
	//}
	testRenderer.init();
	testRenderer.run();
	//std::thread renderThread(&RenderHub::run,&testRenderer);

	//renderThread.join();
}
