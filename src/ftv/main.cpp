#include <thread>
#include <vector>
#include "ftv_renderHub.h"

using namespace std;

int main(){

	
	Ftv_RenderHub testRenderer;
	testRenderer.init();
	testRenderer.runFtvVolumeTest();
}
