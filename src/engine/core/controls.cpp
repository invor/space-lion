#include "controls.h"

Controls* Controls::activeInstance;

void Controls::updateCamera(GLFWwindow *window, Scene *scene)
{
	/*	Get current cursor position on the screen */
	double pos_x, pos_y;
	glfwGetCursorPos(window,&pos_x,&pos_y);
	glm::vec2 currentCursorPosition(pos_x,pos_y);

	/*	Get pointer to active camera */
	SceneCamera* scene_camera_ptr = scene->getActiveCamera();
	glm::vec3 look_at_point = scene_camera_ptr->getLookAt();

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
	{
		glm::vec2 cursor_movement = (latest_cursor_position - currentCursorPosition)*0.1f;

		/*	Update camera position */
		glm::vec4 cam_position = glm::vec4(scene_camera_ptr->getPosition(), 1.0);
		/*	To that end, translate look-at-point to origin */
		glm::vec4 new_position = cam_position - glm::vec4(look_at_point, 0.0);
		/*	Calculate new oribtal position */
		new_position = glm::rotate(glm::mat4(1.0), cursor_movement.x, y_axis)
			* glm::rotate(glm::mat4(1.0), cursor_movement.y, scene_camera_ptr->computeRightHandVector())
			* new_position;
		/*	Reverse translation */
		new_position += glm::vec4(look_at_point, 0.0);
		scene_camera_ptr->setPosition(glm::vec3(new_position));

		/*	Update camera oritentation */
		glm::quat cam_orientation = scene_camera_ptr->getOrientation();
		/*	First, global rotation around y-axis */
		glm::quat new_orientation = glm::normalize(glm::rotate(glm::quat(), cursor_movement.x, y_axis)) * cam_orientation;
		scene_camera_ptr->setOrientation(new_orientation);
		/*	Followed by a local rotation around x-axis*/
		scene_camera_ptr->rotate(cursor_movement.y, x_axis);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS)
	{
		glm::vec2 cursor_movement = (latest_cursor_position - currentCursorPosition)*0.1f;
		glm::vec3 right_hand_vec = scene_camera_ptr->computeRightHandVector();
		glm::vec3 up_vec = scene_camera_ptr->computeUpVector();

		glm::vec3 translation_vec = (right_hand_vec * cursor_movement.x) + (up_vec * -cursor_movement.y);

		scene_camera_ptr->setLookAt(look_at_point + translation_vec);
		scene_camera_ptr->setPosition(scene_camera_ptr->getPosition() + translation_vec);
	}

	/*	Implement "zoom" function */
	float distance_from_center = glm::length(scene_camera_ptr->getPosition() - scene_camera_ptr->getLookAt());
	scene_camera_ptr->translate( latest_scroll_input * (distance_from_center/8.0f) * scene_camera_ptr->computeFrontVector());
	latest_scroll_input = 0.0;

	/*	Update latest cursor position */
	latest_cursor_position = currentCursorPosition;
}

void Controls::mouseScrollFeedback(GLFWwindow *window, double x_offset, double y_offset)
{
	activeInstance->latest_scroll_input += 0.5f * (float) y_offset;
}
