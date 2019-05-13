#ifndef TransformTools_hpp
#define TransformTools_hpp

#include "EntityManager.hpp"

namespace Editor
{
	class TransformTool
	{
	public:
		enum Mode { TRANSLATE, ROTATE, SCALE  };

	private:
		Mode m_active_tool_mode;
		int m_active_axis;

		const Vec3 m_axis[3] = { Vec3(1.0,0.0,0.0), Vec3(0.0,1.0,0.0), Vec3(0.0,0.0,1.0) };

		int m_activation_counter;

		Entity m_tgizmo_xaxis;
		Entity m_tgizmo_yaxis;
		Entity m_tgizmo_zaxis;

		Entity m_rgizmo_xaxis;
		Entity m_rgizmo_yaxis;
		Entity m_rgizmo_zaxis;

		Entity m_sgizmo_xaxis;
		Entity m_sgizmo_yaxis;
		Entity m_sgizmo_zaxis;

	public:
		TransformTool();
		~TransformTool();

		void activate();

		void deactivate();

		void setToolMode(Mode tool_mode);

		/**
		 * Select entity belonging to the transform tool
		 * \param e Selected entity
		 * \return Returns false it given entity is not part of the transform tool, true otherwise
		 */
		bool selectToolEntity(uint entity_id);

		void setGizmoVisible(bool visible);

		void setGizmoPosition(Vec3 new_position);

		void moveGizmo(Vec3 translation);

		/**
		 * Applies a transformation to a target entity using the current tool mode and select tool entity
		 * \param target Entity that the transformation is applied to
		 * \param dx User input in x-direction (assumes screen-space mouse input)
		 * \param dy User input in y-direction (assumes screen-space mouse input)
		 */
		void translate(double dx, double dy, double x, double y);

		void rotate(double dx, double dy, double x, double y);
	};
}

#endif