#include "controls.h"

Controls* Controls::activeInstance;

void Controls::updateCamera(GLFWwindow *window, Scene *scene)
{
	double pos_x, pos_y;
	glfwGetCursorPos(window,&pos_x,&pos_y);
	glm::vec2 currentCursorPosition(pos_x,pos_y);

	SceneCamera* scene_camera_ptr = scene->getActiveCamera();

	if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
	{
		glm::vec2 cursor_movement = (latest_cursor_position-currentCursorPosition);
		latest_y_rotation += cursor_movement.x*0.1f;
		latest_x_rotation += cursor_movement.y*0.1f;
	}

	glm::mat4 translation_helper = glm::rotate(glm::mat4(1.0), latest_y_rotation ,y_axis);
	translation_helper = glm::rotate(translation_helper, latest_x_rotation ,x_axis);
		
	glm::vec4 new_position_h = translation_helper * glm::vec4(0.0,0.0,latest_radius,1.0);
	glm::vec3 new_position = glm::vec3(new_position_h);

	scene_camera_ptr->setPosition(new_position);

	scene_camera_ptr->setOrientation(glm::normalize(glm::rotate(glm::quat(),latest_y_rotation,y_axis)) *
										glm::normalize(glm::rotate(glm::quat(),latest_x_rotation,x_axis)));

	latest_cursor_position = currentCursorPosition;
}

void Controls::mouseScrollFeedback(GLFWwindow *window, double x_offset, double y_offset)
{
	float new_radius = activeInstance->latest_radius - 0.5f * (float)y_offset * activeInstance->latest_radius/8.0;

	if(new_radius > 0.0) {activeInstance->latest_radius = new_radius;}
}
