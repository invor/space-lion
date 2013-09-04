#include "core/renderHub.h"

#include <thread>
#include <vector>

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
	std::thread renderThread(&RenderHub::init,&testRenderer);

	renderThread.join();
}
