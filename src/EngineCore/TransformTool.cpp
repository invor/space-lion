#include "TransformTool.hpp"

namespace Editor
{
	TransformTool::TransformTool(EntityManager* entity_mngr, TransformComponentManager* transform_mngr)
		: m_entity_mngr(entity_mngr), m_transform_mngr(transform_mngr),
		m_tgizmo_xaxis(m_entity_mngr->create()),
		m_tgizmo_yaxis(m_entity_mngr->create()),
		m_tgizmo_zaxis(m_entity_mngr->create()),
		m_sgizmo_xaxis(m_entity_mngr->create()),
		m_sgizmo_yaxis(m_entity_mngr->create()),
		m_sgizmo_zaxis(m_entity_mngr->create())
	{
	}

	TransformTool::~TransformTool()
	{
	}

	void TransformTool::setToolMode(Mode tool_mode)
	{
	}

	bool TransformTool::selectToolEntity(Entity e)
	{
		return false;
	}

	void TransformTool::transform(Entity target, double dx, double dy)
	{
	}
}