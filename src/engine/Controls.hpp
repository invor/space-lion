#ifndef Controls_hpp
#define Controls_hpp

#include "GLFW/glfw3.h"
#include "TransformComponent.hpp"

#include <iostream>

namespace Controls
{
	enum ControlState { CAMERA };

	void init(TransformComponentManager* transform_mngr, Entity* active_camera);

	void checkKeyStatus(GLFWwindow* active_window, double dt);

	void setControlState(ControlState new_state);

	void setControlCallbacks(GLFWwindow* active_window);
}

#endif