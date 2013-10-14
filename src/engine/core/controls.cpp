#include "controls.h"

namespace Controls {

	namespace
	{
		float latest_scroll_input = 0.0;
		glm::vec2 latest_cursor_position = glm::vec2(0.0,0.0);
		glm::vec3 x_axis = glm::vec3(1.0, 0.0, 0.0);
		glm::vec3 y_axis = glm::vec3(0.0, 1.0, 0.0);

		void mouseScrollFeedback(GLFWwindow *window, double x_offset, double y_offset)
		{
			latest_scroll_input += 0.5f * (float) y_offset;
		}
	}

	void updateCamera(GLFWwindow *window, SceneCamera *camera)
	{
		/*	Get current cursor position on the screen */
		double pos_x, pos_y;
		glfwGetCursorPos(window, &pos_x, &pos_y);
		glm::vec2 currentCursorPosition(pos_x, pos_y);

		/*	Get pointer to active camera */
		glm::vec3 look_at_point = camera->getLookAt();

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			glm::vec2 cursor_movement = (latest_cursor_position - currentCursorPosition)*0.1f;

			/*	Update camera position */
			glm::vec4 cam_position = glm::vec4(camera->getPosition(), 1.0);
			/*	To that end, translate look-at-point to origin */
			glm::vec4 new_position = cam_position - glm::vec4(look_at_point, 0.0);
			/*	Calculate new oribtal position */
			new_position = glm::rotate(glm::mat4(1.0), cursor_movement.x, y_axis)
				* glm::rotate(glm::mat4(1.0), cursor_movement.y, camera->computeRightHandVector())
				* new_position;
			/*	Reverse translation */
			new_position += glm::vec4(look_at_point, 0.0);
			camera->setPosition(glm::vec3(new_position));

			/*	Update camera oritentation */
			glm::quat cam_orientation = camera->getOrientation();
			/*	First, global rotation around y-axis */
			glm::quat new_orientation = glm::normalize(glm::rotate(glm::quat(), cursor_movement.x, y_axis)) * cam_orientation;
			camera->setOrientation(new_orientation);
			/*	Followed by a local rotation around x-axis*/
			camera->rotate(cursor_movement.y, x_axis);
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS)
		{
			glm::vec2 cursor_movement = (latest_cursor_position - currentCursorPosition)*0.1f;
			glm::vec3 right_hand_vec = camera->computeRightHandVector();
			glm::vec3 up_vec = camera->computeUpVector();

			glm::vec3 translation_vec = (right_hand_vec * cursor_movement.x) + (up_vec * -cursor_movement.y);

			camera->setLookAt(look_at_point + translation_vec);
			camera->setPosition(camera->getPosition() + translation_vec);
		}

		/*	Implement "zoom" function */
		float distance_from_center = glm::length(camera->getPosition() - camera->getLookAt());
		camera->translate(latest_scroll_input * (distance_from_center / 8.0f) * camera->computeFrontVector());
		latest_scroll_input = 0.0;

		/*	Update latest cursor position */
		latest_cursor_position = currentCursorPosition;
	}

	void setControlCallbacks(GLFWwindow *active_window)
	{
		glfwSetScrollCallback(active_window, mouseScrollFeedback);
	}
}
