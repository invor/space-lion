#include <thread>
#include <future>

#include "EngineFrontend.hpp"

#include "..\Editor\CameraController.hpp"

int main() {

    EngineCore::Common::EngineFrontend engine_frontend;

    std::thread engine_thread(&(EngineCore::Common::EngineFrontend::startEngine), &engine_frontend);

    engine_frontend.waitForEngineStarted();

    Editor::Controls::CameraController cam_ctrl(engine_frontend.accessWorldState(),engine_frontend.accessFrameManager());

    engine_frontend.addInputActionContext(cam_ctrl.getInputActionContext());

    engine_thread.join();

    return 0;
}