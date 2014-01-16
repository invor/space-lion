#include <thread>
#include <vector>

#include "fapra_renderHub.h"

using namespace std;

int main(){

	FapraRenderHub renderHub;
	renderHub.init();
	renderHub.renderActiveScene();

	return 0;
}
