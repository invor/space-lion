#ifndef TransformTools_hpp
#define TransformTools_hpp

#include "EntityManager.hpp"
#include "TransformComponent.hpp"

namespace Editor
{
	class TransformTool
	{
	public:
		enum Mode { TRANSLATE, ROTATE, SCALE  };

	private:
		Mode m_active_tool_mode;

		Entity m_tgizmo_xaxis;
		Entity m_tgizmo_yaxis;
		Entity m_tgizmo_zaxis;

		Entity m_sgizmo_xaxis;
		Entity m_sgizmo_yaxis;
		Entity m_sgizmo_zaxis;

		EntityManager* m_entity_mngr;
		TransformComponentManager* m_transform_mngr;
	public:
		TransformTool(EntityManager* entity_mngr, TransformComponentManager* transform_mngr);
		~TransformTool();

		void setToolMode(Mode tool_mode);

		/**
		 * Select entity belonging to the transform tool
		 * \param e Selected entity
		 * \return Returns false it given entity is not part of the transform tool, true otherwise
		 */
		bool selectToolEntity(Entity e);

		/**
		 * Applies a transformation to a target entity using the current tool mode and select tool entity
		 * \param target Entity that the transformation is applied to
		 * \param dx User input in x-direction (assumes screen-space mouse input)
		 * \param dy User input in y-direction (assumes screen-space mouse input)
		 */
		void transform(Entity target, double dx, double dy);
	};
}

#endif