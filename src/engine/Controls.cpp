#include "Controls.hpp"

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

		Entity* _active_camera;

		double last_xpos, last_ypos;

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

			uint idx = GCoreComponents::transformManager().getIndex(*_active_camera);
			Quat orientation = GCoreComponents::transformManager().getOrientation(idx);
			Vec3 worldUp = glm::rotate(glm::conjugate(orientation),Vec3(0.0,1.0,0.0));
			GCoreComponents::transformManager().rotate(idx,glm::rotate(Quat(),-0.001f*(float)dx,worldUp));
			GCoreComponents::transformManager().rotate(idx,glm::rotate(Quat(),-0.001f*(float)dy,Vec3(1.0,0.0,0.0)));
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
			if( glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				double dx = xpos - last_xpos;
				double dy = ypos - last_ypos;

				//std::cout<<"Mouse dx: "<<dx<<" dy: "<<dy<<std::endl;

				last_xpos = xpos;
				last_ypos = ypos;

				uint idx = GCoreComponents::transformManager().getIndex(*_active_camera);
				Quat orientation = GCoreComponents::transformManager().getOrientation(idx);
				Vec3 worldUp = glm::rotate(glm::conjugate(orientation),Vec3(0.0,1.0,0.0));
				GCoreComponents::transformManager().rotate(idx,glm::rotate(Quat(),-0.001f*(float)dx,worldUp));
				GCoreComponents::transformManager().rotate(idx,glm::rotate(Quat(),-0.001f*(float)dy,Vec3(1.0,0.0,0.0)));
			}
		}

		void editorMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
			if( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
				glfwGetCursorPos(window,&last_xpos,&last_ypos);
		}

		void editorKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window,true);
				break;
			case GLFW_KEY_W:
				//TODO select move tool
				break;
			case GLFW_KEY_E:
				//TODO select rotate tool
				break;
			case GLFW_KEY_R:
				//TODO select scale tool
				break;
			default:
				break;
			}
		}
	}

	void init(Entity* active_camera)
	{
		_active_camera = active_camera;
	}

	void checkKeyStatus(GLFWwindow* active_window, double dt)
	{
		float boost = 5.0f;

		if (glfwGetKey(active_window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			boost = 50.0f;

		uint idx = GCoreComponents::transformManager().getIndex(*_active_camera);
		Quat orientation = GCoreComponents::transformManager().getOrientation(idx);

		Quat qFront = glm::cross(glm::cross(orientation,glm::quat(0.0,0.0,0.0,-1.0)),glm::conjugate(orientation));
		Quat qUp =	glm::cross(glm::cross(orientation,glm::quat(0.0,0.0,1.0,0.0)),glm::conjugate(orientation));
		Quat qRighthand = glm::cross(glm::cross(orientation,glm::quat(0.0,1.0,0.0,0.0)),glm::conjugate(orientation));

		Vec3 vfront = glm::normalize(glm::vec3(qFront.x,qFront.y,qFront.z)) * boost * (float)dt;
		Vec3 vUp = glm::normalize(glm::vec3(qUp.x,qUp.y,qUp.z));
		Vec3 vRighthand = glm::normalize(glm::vec3(qRighthand.x,qRighthand.y,qRighthand.z)) * boost * (float)dt;

		if (glfwGetKey(active_window,GLFW_KEY_W) == GLFW_PRESS)
			GCoreComponents::transformManager().translate(idx,vfront);

		if (glfwGetKey(active_window,GLFW_KEY_A) == GLFW_PRESS)
			GCoreComponents::transformManager().translate(idx,-vRighthand);

		if (glfwGetKey(active_window,GLFW_KEY_S) == GLFW_PRESS)
			GCoreComponents::transformManager().translate(idx,-vfront);

		if (glfwGetKey(active_window,GLFW_KEY_D) == GLFW_PRESS)
			GCoreComponents::transformManager().translate(idx,vRighthand);
	}

	void setControlCallbacks(GLFWwindow* active_window)
	{
		switch (current_state)
		{
			case CAMERA:
				glfwSetInputMode(active_window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
				glfwSetCursorPosCallback(active_window,cameraCursorPosCallback);
				glfwSetMouseButtonCallback(active_window,cameraMouseButtonCallback);
				glfwSetKeyCallback(active_window,cameraKeyCallback);
				break;
			case EDITOR:
				glfwSetCursorPosCallback(active_window,editorCursorPosCallback);
				glfwSetMouseButtonCallback(active_window,editorMouseButtonCallback);
				glfwSetKeyCallback(active_window,editorKeyCallback);
				break;
		default:
			break;
		}
	}
}