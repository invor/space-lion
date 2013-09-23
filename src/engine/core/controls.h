#ifndef controls_h
#define controls_h

#include "scene.h"

class Controls
{
public:
	Controls() : x_axis(1.0,0.0,0.0), y_axis(0.0,1.0,0.0), latest_cursor_position(0.0,0.0),latest_scroll_input(0.0) {}
	~Controls() {}

	void updateCamera(GLFWwindow *window, Scene *scene);

	/*	Callback handling */
	static Controls *activeInstance;
	static void setActive(Controls *controlsPtr) {activeInstance=controlsPtr;}
	static void mouseScrollFeedback(GLFWwindow *window, double x_offset, double y_offset);
private:
	const glm::vec3 x_axis;
	const glm::vec3 y_axis;

	glm::vec2 latest_cursor_position;
	float latest_scroll_input;
};

#endif
