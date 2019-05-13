#include <thread>
#include <future>

#include "EngineFrontend.hpp"

int main() {
	
	EngineCore::Common::EngineFrontend engine_frontend;

	std::thread engine_thread(&(EngineCore::Common::EngineFrontend::startEngine), &engine_frontend);

	engine_thread.join();
	
	return 0;
}