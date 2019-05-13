#ifndef Controls_hpp
#define Controls_hpp

struct Entity;
struct GLFWwindow;

#include <functional>

namespace Controls
{
	enum ControlState { CAMERA, EDITOR };

	void init(Entity* active_camera);

	void checkKeyStatus(GLFWwindow* active_window, double dt);

	void checkJoystickStatus(GLFWwindow* active_window, double dt);

	void setControlState(ControlState new_state);

	void setActiveAircraft(Entity* aircraft);

	void setControlCallbacks(GLFWwindow* active_window);

	void setEditorMouseCtrlCallback(std::function<void(double, double)> f);

	void setEditorMouseMoveCallback(std::function<void(double, double, double, double)> f);

	void setEditorMouseReleaseCallback(std::function<void()> f);
}

#endif