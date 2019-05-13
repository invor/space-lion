#include "Controls.hpp"

#include "GlobalEngineCore.hpp"
#include "DeferredRenderingPipeline.hpp"

#include "GlobalRenderingComponents.hpp"
#include "PickingComponent.hpp"

#include "GlobalCoreComponents.hpp"
#include "AirplanePhysicsComponent.hpp"
#include "CameraComponent.hpp"
#include "TransformComponent.hpp"

#include "GlobalTools.hpp"
#include "EditorUI.hpp"
#include "EditorSelectComponent.hpp"
#include "TransformTool.hpp"

#include "GLFW/glfw3.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw_gl3.h"

#include <iostream>

namespace Controls
{
	/* Hidden namespace to store some control related variables and functions */
	namespace
	{

#ifdef EDITOR_MODE
		ControlState current_state = EDITOR;
#else
		ControlState current_state = CAMERA;
#endif

		Entity* _active_aircraft = nullptr;

		int _active_joystick = 0;

		double last_xpos, last_ypos;

		float _movement_speed = 1.0f;

		int _selected_id = -1;
		int _lastClick_id = -1;

		// Editor mode internal callback functions
		std::function<void(double, double)> _mouse_ctrl_callback = [](auto a, auto b) {};
		std::function<void(double,double,double,double)> _mouse_move_callback = [](auto a, auto b, auto c, auto d) {};
		std::function<void()> _mouse_release_callback = []() {};


		/*
		* Mouse and keyboard callback for controlling the currently active camera.
		*/
		void cameraCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
		{
			double dx = xpos - last_xpos;
			double dy = ypos - last_ypos;

			//std::cout<<"Mouse dx: "<<dx<<" dy: "<<dy<<std::endl;

			last_xpos = xpos;
			last_ypos = ypos;

			Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
			uint idx = GCoreComponents::transformManager().getIndex(active_camera);
			Quat orientation = GCoreComponents::transformManager().getOrientation(idx);
			Vec3 worldUp = glm::rotate(glm::conjugate(orientation),Vec3(0.0,1.0,0.0));
			GCoreComponents::transformManager().rotateLocal(idx,glm::rotate(Quat(),-0.001f*(float)dx,worldUp));
			GCoreComponents::transformManager().rotateLocal(idx,glm::rotate(Quat(),-0.001f*(float)dy,Vec3(1.0,0.0,0.0)));
		}

		void cameraMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
		}

		void cameraKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if(key == GLFW_KEY_ESCAPE)
			{
				glfwSetWindowShouldClose(window,true);
			}
		}

		/*
		 * Mouse and keyboard callbacks for very simple editor controls
		 */
		void editorCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
		{
			if (ImGui::GetIO().WantCaptureMouse)
				return;

			double dx = xpos - last_xpos;
			double dy = ypos - last_ypos;

			if( glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				//std::cout<<"Mouse dx: "<<dx<<" dy: "<<dy<<std::endl;

				Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
				uint idx = GCoreComponents::transformManager().getIndex(active_camera);
				Quat orientation = GCoreComponents::transformManager().getOrientation(idx);
				Vec3 worldUp = glm::rotate(glm::conjugate(orientation),Vec3(0.0,1.0,0.0));
				GCoreComponents::transformManager().rotateLocal(idx,glm::rotate(Quat(),-0.001f*(float)dx,worldUp));
				GCoreComponents::transformManager().rotateLocal(idx,glm::rotate(Quat(),-0.001f*(float)dy,Vec3(1.0,0.0,0.0)));
			}
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				int w_width, w_height;
				glfwGetWindowSize(window, &w_width, &w_height);
				double dxn = dx / static_cast<double>(w_width);
				double dyn = -dy / static_cast<double>(w_height);

				double xn = xpos / static_cast<double>(w_width);
				double yn = (static_cast<double>(w_height) - ypos) / static_cast<double>(w_height);

				//	if (_selected_id >= 0 && _lastClick_id >= 0)
				//	{
				//		_transform_tool->transform(_selected_id, dxn, dyn);
				//		GLandscapeComponents::featureCurveManager().updateCorrespondingCurve(_selected_id);
				//		if(GLandscapeComponents::landscapeManager().isReady(0))
				//			GLandscapeComponents::landscapeManager().updateBricks(0);
				//	}

				_mouse_move_callback(dxn, dyn, xn, yn);
			}

			last_xpos = xpos;
			last_ypos = ypos;
		}

		void editorMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
			ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);

			if (ImGui::GetIO().WantCaptureMouse)
				return;


			//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
			//{
			//	double xpos, ypos;
			//	glfwGetCursorPos(window, &xpos, &ypos);
			//
			//	int w_width, w_height;
			//	glfwGetWindowSize(window, &w_width, &w_height);
			//	double xn = xpos / static_cast<double>(w_width);
			//	double yn = (static_cast<double>(w_height) - ypos) / static_cast<double>(w_height);
			//
			//	_mouse_ctrl_callback(xn, yn);
			//}
			if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			{
				int width, height;
				double pos_x, pos_y;
				glfwGetWindowSize(window, &width, &height);
				glfwGetCursorPos(window, &pos_x, &pos_y);
				double pos_x_normalized = (pos_x / (double)width);
				double pos_y_normalized = (pos_y / (double)height);

				GEngineCore::renderingPipeline().getGBuffer()->bindToRead(4);
				width = GEngineCore::renderingPipeline().getGBuffer()->getWidth();
				height = GEngineCore::renderingPipeline().getGBuffer()->getHeight();

				GLint data[1] = { -1 };
				glReadPixels((GLint)(pos_x_normalized*width), (GLint)(height - pos_y_normalized*height), 1, 1, GL_RED_INTEGER, GL_INT, data);

				std::cout << "pixel id: " << data[0] << std::endl;

				_lastClick_id = data[0];

				// TODO move to editor code
				if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				{
					if (GTools::selectManager().isSelected(_lastClick_id))
					{
						GTools::selectManager().removeFromSelection(_lastClick_id);
					}
					else
					{
						GTools::selectManager().addToSelection(_lastClick_id);
					}

					GRenderingComponents::pickingManager().getOnPickFunction(_lastClick_id)();
				}
				else if (mods == GLFW_MOD_CONTROL)
				{
					_mouse_ctrl_callback(pos_x_normalized, 1.0 - pos_y_normalized);
				}
				else
				{
					if (_lastClick_id == -1)
						GTools::selectManager().resetSelection();

					GRenderingComponents::pickingManager().getOnPickFunction(_lastClick_id)();
					GTools::selectManager().select(_lastClick_id);
				}
			}
			else if (action == GLFW_RELEASE)
			{
				_mouse_release_callback();
				_mouse_release_callback = []() {};
			}
			//	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			//	{
			//		if (_selected_id >= 0 && _lastClick_id >= 0 && _selected_id != _lastClick_id)
			//		{
			//			GLandscapeComponents::landscapeManager().updateBricks(0);
			//		}
			//	}
		}

		void editorKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
				return; 

			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window,true);
				break;
			case GLFW_KEY_W:
				//TODO select move tool
				GTools::transformTool().setToolMode(Editor::TransformTool::TRANSLATE);
				break;
			case GLFW_KEY_E:
				//TODO select rotate tool
				GTools::transformTool().setToolMode(Editor::TransformTool::ROTATE);
				break;
			case GLFW_KEY_R:
				//TODO select scale tool
				break;
			case GLFW_KEY_H:
				if(action == GLFW_PRESS && mods == GLFW_MOD_ALT)
					GEngineCore::renderingPipeline().invertInterfaceHidden();
				break;
			/*
			case GLFW_KEY_F:
				if (action == GLFW_PRESS && mods == GLFW_MOD_ALT)
				{
					Entity new_curve = GLandscapeComponents::landscapeManager().addFeatureCurve(true);
					GLandscapeComponents::featureCurveManager().addConstraintPoint(GLandscapeComponents::featureCurveManager().getIndex(new_curve), 0.33);
					GLandscapeComponents::featureCurveManager().addConstraintPoint(GLandscapeComponents::featureCurveManager().getIndex(new_curve), 0.66);
				}
				break;
			case GLFW_KEY_G:
				if (action == GLFW_PRESS && mods == GLFW_MOD_ALT)
				{
					Entity new_curve = GLandscapeComponents::landscapeManager().addFeatureCurve(false);
					GLandscapeComponents::featureCurveManager().addConstraintPoint(GLandscapeComponents::featureCurveManager().getIndex(new_curve), 0.33);
					GLandscapeComponents::featureCurveManager().addConstraintPoint(GLandscapeComponents::featureCurveManager().getIndex(new_curve), 0.66);
				}
				break;
			case GLFW_KEY_1:
				if (action == GLFW_PRESS)
					GTools::constraintPointTool().setNoiseAmplitude(0.01f);
				break;
			case GLFW_KEY_2:
				if (action == GLFW_PRESS)
					GTools::constraintPointTool().setNoiseAmplitude(0.33f);
				break;
			case GLFW_KEY_3:
				if (action == GLFW_PRESS)
					GTools::constraintPointTool().setNoiseAmplitude(0.66f);
				break;
			case GLFW_KEY_4:
				if (action == GLFW_PRESS)
					GTools::constraintPointTool().setNoiseAmplitude(1.0f);
				break;
			case GLFW_KEY_5:
				if (action == GLFW_PRESS)
					GTools::constraintPointTool().setNoiseRoughness(5.0f);
				break;
			case GLFW_KEY_6:
				if (action == GLFW_PRESS)
					GTools::constraintPointTool().setNoiseRoughness(10.0f);
				break;
			case GLFW_KEY_7:
				if (action == GLFW_PRESS)
					GTools::constraintPointTool().setNoiseRoughness(15.0f);
				break;
			case GLFW_KEY_8:
				if (action == GLFW_PRESS)
					GTools::constraintPointTool().setNoiseRoughness(20.0f);
				break;
				*/
			default:
				break;
			}
		}

		void editorScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
		{
			ImGui_ImplGlfwGL3_ScrollCallback(window, xoffset, yoffset);

			if (ImGui::GetIO().WantCaptureMouse)
				return;

			_movement_speed = std::max(1.0f, _movement_speed + static_cast<float>(yoffset));
		}
	}

	void checkKeyStatus(GLFWwindow* active_window, double dt)
	{
		if (!glfwJoystickPresent(GLFW_JOYSTICK_1) && _active_aircraft != nullptr)
		{
			uint aircraft_idx = GCoreComponents::airplanePhysicsManager().getIndex(*_active_aircraft);
			if (glfwGetKey(active_window, GLFW_KEY_UP) == GLFW_PRESS)
				GCoreComponents::airplanePhysicsManager().setElevatorAngle(aircraft_idx, -1.0);
			else if (glfwGetKey(active_window, GLFW_KEY_DOWN) == GLFW_PRESS)
				GCoreComponents::airplanePhysicsManager().setElevatorAngle(aircraft_idx, 1.0);
			else
				GCoreComponents::airplanePhysicsManager().setElevatorAngle(aircraft_idx, 0.0);

			if (glfwGetKey(active_window, GLFW_KEY_LEFT) == GLFW_PRESS)
				GCoreComponents::airplanePhysicsManager().setAileronAngle(aircraft_idx, -1.0);
			else if (glfwGetKey(active_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				GCoreComponents::airplanePhysicsManager().setAileronAngle(aircraft_idx, 1.0);
			else
				GCoreComponents::airplanePhysicsManager().setAileronAngle(aircraft_idx, 0.0);
		}


		if (current_state==EDITOR && glfwGetMouseButton(active_window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
			return;

		float boost = 5.0f;

		if (glfwGetKey(active_window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			boost = 50.0f;

		Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
		uint idx = GCoreComponents::transformManager().getIndex(active_camera);
		Quat orientation = GCoreComponents::transformManager().getOrientation(idx);

		Quat qFront = glm::cross(glm::cross(orientation,glm::quat(0.0,0.0,0.0,-1.0)),glm::conjugate(orientation));
		Quat qUp =	glm::cross(glm::cross(orientation,glm::quat(0.0,0.0,1.0,0.0)),glm::conjugate(orientation));
		Quat qRighthand = glm::cross(glm::cross(orientation,glm::quat(0.0,1.0,0.0,0.0)),glm::conjugate(orientation));

		Vec3 vfront = glm::normalize(glm::vec3(qFront.x,qFront.y,qFront.z)) * boost * _movement_speed * (float)dt;
		Vec3 vUp = glm::normalize(glm::vec3(qUp.x,qUp.y,qUp.z));
		Vec3 vRighthand = glm::normalize(glm::vec3(qRighthand.x,qRighthand.y,qRighthand.z)) * boost * _movement_speed * (float)dt;

		if (glfwGetKey(active_window,GLFW_KEY_W) == GLFW_PRESS)
			GCoreComponents::transformManager().translate(idx,vfront);

		if (glfwGetKey(active_window,GLFW_KEY_A) == GLFW_PRESS)
			GCoreComponents::transformManager().translate(idx,-vRighthand);

		if (glfwGetKey(active_window,GLFW_KEY_S) == GLFW_PRESS)
			GCoreComponents::transformManager().translate(idx,-vfront);

		if (glfwGetKey(active_window,GLFW_KEY_D) == GLFW_PRESS)
			GCoreComponents::transformManager().translate(idx,vRighthand);
	}

	void checkJoystickStatus(GLFWwindow* active_window, double dt)
	{
		if (!glfwJoystickPresent(GLFW_JOYSTICK_1))
			return;

		//std::cout << "Joystick connected" << std::endl;

		if (_active_aircraft == nullptr)
		{
			int count;
			const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

			float trans_dx = axes[0];
			float trans_dy = axes[1];

			if (abs(trans_dx) < 0.05f)
				trans_dx = 0.0f;

			if (abs(trans_dy) < 0.05f)
				trans_dy = 0.0f;

			Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
			uint camera_idx = GCoreComponents::transformManager().getIndex(active_camera);
			glm::quat orientation = GCoreComponents::transformManager().getOrientation(camera_idx);
			glm::vec3 position = GCoreComponents::transformManager().getWorldPosition(camera_idx);

			glm::quat qFront = glm::cross(glm::cross(orientation, glm::quat(0.0, 0.0, 0.0, -1.0)), glm::conjugate(orientation));
			glm::quat qUp = glm::cross(glm::cross(orientation, glm::quat(0.0, 0.0, 1.0, 0.0)), glm::conjugate(orientation));
			glm::quat qRighthand = glm::cross(glm::cross(orientation, glm::quat(0.0, 1.0, 0.0, 0.0)), glm::conjugate(orientation));

			glm::vec3 vfront = glm::normalize(glm::vec3(qFront.x, qFront.y, qFront.z)) * (float)dt;
			glm::vec3 vUp = glm::normalize(glm::vec3(qUp.x, qUp.y, qUp.z));
			glm::vec3 vRighthand = glm::normalize(glm::vec3(qRighthand.x, qRighthand.y, qRighthand.z)) * (float)dt;

			GCoreComponents::transformManager().translate(camera_idx, trans_dy * 100.0f * vfront + trans_dx * 100.0f * vRighthand);

			float rot_dx = axes[2];
			float rot_dy = axes[3];

			if (abs(rot_dx) < 0.025f)
				rot_dx = 0.0f;

			if (abs(rot_dy) < 0.025f)
				rot_dy = 0.0f;

			rot_dx = std::abs(rot_dx) * rot_dx * 1.5f;
			rot_dy = std::abs(rot_dy) * rot_dy * 1.5f;

			glm::quat cam_orientation = glm::rotate(glm::quat(), 3.14f, Vec3(0.0, 1.0, 0.0));
			glm::vec3 worldUp = glm::rotate(glm::conjugate(orientation), glm::vec3(0.0, 1.0, 0.0));

			orientation *= glm::rotate(glm::quat(), rot_dx * -(float)dt, worldUp);
			orientation *= glm::rotate(glm::quat(), rot_dy * (float)dt, glm::vec3(1.0, 0.0, 0.0));

			GCoreComponents::transformManager().setOrientation(camera_idx,orientation);

			return;
		}

		int axes_count;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);

		int button_count;
		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &button_count);

		//for (int i = 0; i < button_count; i++)
		//{
		//	if (buttons[i] == GLFW_PRESS)
		//		std::cout << "Button " << i << " pressed." << std::endl;
		//}

		float left_dx = axes[0];
		float left_dy = axes[1];
		float right_dx = axes[2];
		float right_dy = axes[3];
		float trig_left = axes[5];
		float trig_right = axes[4];

		if (abs(left_dx) < 0.05f) left_dx = 0.0f;
		if (abs(left_dy) < 0.05f) left_dy = 0.0f;

		if (abs(right_dx) < 0.05f) right_dx = 0.0f;
		if (abs(right_dy) < 0.05f) right_dy = 0.0f;

		std::cout << "rigth dx: " << right_dx << std::endl;

		uint aircraft_idx = GCoreComponents::airplanePhysicsManager().getIndex(*_active_aircraft);
		GCoreComponents::airplanePhysicsManager().setElevatorAngle(aircraft_idx, -left_dy);
		GCoreComponents::airplanePhysicsManager().setRudderAngle(aircraft_idx, -(trig_left*0.5f+0.5f) + (trig_right*0.5f + 0.5f) );
		GCoreComponents::airplanePhysicsManager().setAileronAngle(aircraft_idx, left_dx);

		if (buttons[4] == GLFW_PRESS)
		{
			float current_thrust = GCoreComponents::airplanePhysicsManager().getEngineThrust(aircraft_idx);
			GCoreComponents::airplanePhysicsManager().setEngineThrust(aircraft_idx, current_thrust - 100.0f);
		}
		else if (buttons[5] == GLFW_PRESS)
		{
			float current_thrust = GCoreComponents::airplanePhysicsManager().getEngineThrust(aircraft_idx);
			GCoreComponents::airplanePhysicsManager().setEngineThrust(aircraft_idx,current_thrust + 100.0f);
		}

		Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
		uint camera_idx = GCoreComponents::transformManager().getIndex(active_camera);
		glm::quat cam_orientation = glm::rotate(glm::quat(), 3.14f, Vec3(0.0, 1.0, 0.0));
		cam_orientation = glm::rotate(cam_orientation, -0.5f * right_dx, Vec3(0.0, 1.0, 0.0));
		cam_orientation = glm::rotate(cam_orientation, -0.5f * right_dy, Vec3(1.0, 0.0, 0.0));
		GCoreComponents::transformManager().setOrientation(camera_idx,cam_orientation);
	}

	void setActiveAircraft(Entity* aircraft)
	{
		_active_aircraft = aircraft;
	}

	void setControlCallbacks(GLFWwindow* active_window)
	{
		for (int i = 0; i < 16; i++)
		{
			if (!glfwJoystickPresent(GLFW_JOYSTICK_1 + i))
			{
				_active_joystick = GLFW_JOYSTICK_1 + i;
				break;
			}
		}

		switch (current_state)
		{
			case CAMERA:
				//glfwSetInputMode(active_window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
				glfwSetCursorPosCallback(active_window,cameraCursorPosCallback);
				glfwSetMouseButtonCallback(active_window,cameraMouseButtonCallback);
				glfwSetKeyCallback(active_window,cameraKeyCallback);

				glfwSetScrollCallback(active_window, ImGui_ImplGlfwGL3_ScrollCallback);
				glfwSetCharCallback(active_window, ImGui_ImplGlfwGL3_CharCallback);
				break;
			case EDITOR:
				glfwSetCursorPosCallback(active_window,editorCursorPosCallback);
				glfwSetMouseButtonCallback(active_window,editorMouseButtonCallback);
				glfwSetKeyCallback(active_window,editorKeyCallback);

				glfwSetScrollCallback(active_window, editorScrollCallback);
				glfwSetCharCallback(active_window, ImGui_ImplGlfwGL3_CharCallback);
				break;
		default:
			break;
		}
	}

	void setEditorMouseCtrlCallback(std::function<void(double,double)> f)
	{
		_mouse_ctrl_callback = f;
	}

	void setEditorMouseMoveCallback(std::function<void(double, double, double, double)> f)
	{
		_mouse_move_callback = f;
	}

	void setEditorMouseReleaseCallback(std::function<void()> f)
	{
		_mouse_release_callback = f;
	}
}