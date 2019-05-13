#include "TransformTool.hpp"

#include <algorithm>

#include "GlobalEngineCore.hpp"

#include "GlobalCoreComponents.hpp"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"

#include "GlobalRenderingComponents.hpp"
#include "StaticMeshComponent.hpp"
#include "PickingComponent.hpp"

#include "GlobalTools.hpp"
#include "EditorSelectComponent.hpp"

#include "Controls.hpp"

namespace Editor
{
	TransformTool::TransformTool() :
		m_active_axis(0),
		m_activation_counter(0),
		m_tgizmo_xaxis(GEngineCore::entityManager().create()),
		m_tgizmo_yaxis(GEngineCore::entityManager().create()),
		m_tgizmo_zaxis(GEngineCore::entityManager().create()),
		m_rgizmo_xaxis(GEngineCore::entityManager().create()),
		m_rgizmo_yaxis(GEngineCore::entityManager().create()),
		m_rgizmo_zaxis(GEngineCore::entityManager().create()),
		m_sgizmo_xaxis(GEngineCore::entityManager().create()),
		m_sgizmo_yaxis(GEngineCore::entityManager().create()),
		m_sgizmo_zaxis(GEngineCore::entityManager().create())
	{
		GCoreComponents::transformManager().addComponent(m_tgizmo_xaxis,Vec3(),Quat(),Vec3(0.5f));
		GCoreComponents::transformManager().addComponent(m_tgizmo_yaxis,Vec3(),Quat(),Vec3(0.5f));
		GCoreComponents::transformManager().addComponent(m_tgizmo_zaxis,Vec3(),Quat(),Vec3(0.5f));

		GCoreComponents::transformManager().addComponent(m_rgizmo_xaxis, Vec3(), Quat(), Vec3(3.0f));
		GCoreComponents::transformManager().addComponent(m_rgizmo_yaxis, Vec3(), Quat(), Vec3(3.0f));
		GCoreComponents::transformManager().addComponent(m_rgizmo_zaxis, Vec3(), Quat(), Vec3(3.0f));

		GCoreComponents::transformManager().addComponent(m_sgizmo_xaxis,Vec3(),Quat(),Vec3(0.5f));
		GCoreComponents::transformManager().addComponent(m_sgizmo_yaxis,Vec3(),Quat(),Vec3(0.5f));
		GCoreComponents::transformManager().addComponent(m_sgizmo_zaxis,Vec3(),Quat(),Vec3(0.5f));


		GRenderingComponents::interfaceMeshManager().addComponent(m_tgizmo_xaxis, "../resources/materials/editor/interface_gizmo.slmtl", "../resources/meshes/editor/move_gizmoX.fbx", false);
		GRenderingComponents::interfaceMeshManager().addComponent(m_tgizmo_yaxis, "../resources/materials/editor/interface_gizmo.slmtl", "../resources/meshes/editor/move_gizmoY.fbx", false);
		GRenderingComponents::interfaceMeshManager().addComponent(m_tgizmo_zaxis, "../resources/materials/editor/interface_gizmo.slmtl", "../resources/meshes/editor/move_gizmoZ.fbx", false);

		GRenderingComponents::interfaceMeshManager().addComponent(m_rgizmo_xaxis, "../resources/materials/editor/interface_gizmo.slmtl", "../resources/meshes/editor/rotate_gizmoX.fbx", false);
		GRenderingComponents::interfaceMeshManager().addComponent(m_rgizmo_yaxis, "../resources/materials/editor/interface_gizmo.slmtl", "../resources/meshes/editor/rotate_gizmoY.fbx", false);
		GRenderingComponents::interfaceMeshManager().addComponent(m_rgizmo_zaxis, "../resources/materials/editor/interface_gizmo.slmtl", "../resources/meshes/editor/rotate_gizmoZ.fbx", false);


		GRenderingComponents::pickingManager().addComponent(m_tgizmo_xaxis, "../resources/materials/editor/gizmo_picking.slmtl", "../resources/meshes/editor/move_gizmoX.fbx",
			[this]() { 	this->selectToolEntity(m_tgizmo_xaxis.id());
						this->setGizmoVisible(true);
						Controls::setEditorMouseMoveCallback(std::bind(&TransformTool::translate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
						Controls::setEditorMouseReleaseCallback([]() {
							Controls::setEditorMouseMoveCallback([](double, double, double, double) {});
							Controls::setEditorMouseReleaseCallback([]() {});
							});
						});
		GRenderingComponents::pickingManager().addComponent(m_tgizmo_yaxis, "../resources/materials/editor/gizmo_picking.slmtl", "../resources/meshes/editor/move_gizmoY.fbx",
			[this]() { 	this->selectToolEntity(m_tgizmo_yaxis.id());
						this->setGizmoVisible(true);
						Controls::setEditorMouseMoveCallback(std::bind(&TransformTool::translate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
						Controls::setEditorMouseReleaseCallback([]() {
							Controls::setEditorMouseMoveCallback([](double, double, double, double) {});
							Controls::setEditorMouseReleaseCallback([]() {});
							});
						});
		GRenderingComponents::pickingManager().addComponent(m_tgizmo_zaxis, "../resources/materials/editor/gizmo_picking.slmtl", "../resources/meshes/editor/move_gizmoZ.fbx",
			[this]() { 	this->selectToolEntity(m_tgizmo_zaxis.id());
						this->setGizmoVisible(true);
						Controls::setEditorMouseMoveCallback(std::bind(&TransformTool::translate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
						Controls::setEditorMouseReleaseCallback([]() {
							Controls::setEditorMouseMoveCallback([](double, double, double, double) {});
							Controls::setEditorMouseReleaseCallback([]() {});
							});
						});
		GRenderingComponents::pickingManager().setPickable(m_tgizmo_xaxis, false);
		GRenderingComponents::pickingManager().setPickable(m_tgizmo_yaxis, false);
		GRenderingComponents::pickingManager().setPickable(m_tgizmo_zaxis, false);

		GRenderingComponents::pickingManager().addComponent(m_rgizmo_xaxis, "../resources/materials/editor/gizmo_picking.slmtl", "../resources/meshes/editor/rotate_gizmoX.fbx",
			[this]() {	this->selectToolEntity(m_rgizmo_xaxis.id());
						this->setGizmoVisible(true);
						Controls::setEditorMouseMoveCallback(std::bind(&TransformTool::rotate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
						Controls::setEditorMouseReleaseCallback([]() {
							Controls::setEditorMouseMoveCallback([](double, double, double, double) {});
							Controls::setEditorMouseReleaseCallback([]() {});
							});
						});
		GRenderingComponents::pickingManager().addComponent(m_rgizmo_yaxis, "../resources/materials/editor/gizmo_picking.slmtl", "../resources/meshes/editor/rotate_gizmoY.fbx",
			[this]() { 	this->selectToolEntity(m_rgizmo_yaxis.id());
						this->setGizmoVisible(true);
						Controls::setEditorMouseMoveCallback(std::bind(&TransformTool::rotate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
						Controls::setEditorMouseReleaseCallback([]() {
							Controls::setEditorMouseMoveCallback([](double, double, double, double) {});
							Controls::setEditorMouseReleaseCallback([]() {});
							});
						});
		GRenderingComponents::pickingManager().addComponent(m_rgizmo_zaxis, "../resources/materials/editor/gizmo_picking.slmtl", "../resources/meshes/editor/rotate_gizmoZ.fbx",
			[this]() { 	this->selectToolEntity(m_rgizmo_zaxis.id());
						this->setGizmoVisible(true);
						Controls::setEditorMouseMoveCallback(std::bind(&TransformTool::rotate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
						Controls::setEditorMouseReleaseCallback([]() {
							Controls::setEditorMouseMoveCallback([](double, double, double, double) {});
							Controls::setEditorMouseReleaseCallback([]() {});
							});
						});

		GRenderingComponents::pickingManager().setPickable(m_rgizmo_xaxis, false);
		GRenderingComponents::pickingManager().setPickable(m_rgizmo_yaxis, false);
		GRenderingComponents::pickingManager().setPickable(m_rgizmo_zaxis, false);

		//TODO rotate mode picking
	}

	TransformTool::~TransformTool()
	{
	}

	void TransformTool::activate()
	{
		m_activation_counter++;

		auto selected_entitis = GTools::selectManager().getAllSelectedEntities();

		setGizmoVisible(true);
		Vec3 avg_positiong = Vec3(0.0f);
		float normalization = 0.0;
		for (auto target : selected_entitis)
		{
			avg_positiong += GCoreComponents::transformManager().getWorldPosition(target);
			normalization += 1.0f;
		}
		setGizmoPosition(avg_positiong/normalization);
	}

	void TransformTool::deactivate()
	{
		m_activation_counter--;

		if (m_activation_counter == 0)
		{
			setGizmoVisible(false);
		}
		else
		{
			auto selected_entities = GTools::selectManager().getAllSelectedEntities();

			setGizmoVisible(true);
			Vec3 avg_positiong = Vec3(0.0f);
			float normalization = 0.0;
			for (auto target : selected_entities)
			{
				avg_positiong += GCoreComponents::transformManager().getWorldPosition(target);
				normalization += 1.0f;
			}
			setGizmoPosition(avg_positiong / normalization);
		}
	}

	void TransformTool::setToolMode(Mode tool_mode)
	{
		m_active_tool_mode = tool_mode;

		if (m_activation_counter > 0)
			setGizmoVisible(true);
	}

	bool TransformTool::selectToolEntity(uint entity_id)
	{
		if (entity_id == m_tgizmo_xaxis.id() || entity_id == m_rgizmo_xaxis.id() || entity_id == m_sgizmo_xaxis.id() )
		{
			m_active_axis = 0;

			return true;
		}
		else if (entity_id == m_tgizmo_yaxis.id() || entity_id == m_rgizmo_yaxis.id() || entity_id == m_sgizmo_yaxis.id() )
		{
			m_active_axis = 1;
			return true;
		}
		else if (entity_id == m_tgizmo_zaxis.id() || entity_id == m_rgizmo_zaxis.id() || entity_id == m_sgizmo_zaxis.id() )
		{
			m_active_axis = 2;
			return true;
		}

		return false;
	}

	void TransformTool::setGizmoVisible(bool visible)
	{
		switch (m_active_tool_mode)
		{
		case Editor::TransformTool::TRANSLATE:
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_xaxis, visible);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_yaxis, visible);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_zaxis, visible);
			GRenderingComponents::pickingManager().setPickable(m_tgizmo_xaxis, visible);
			GRenderingComponents::pickingManager().setPickable(m_tgizmo_yaxis, visible);
			GRenderingComponents::pickingManager().setPickable(m_tgizmo_zaxis, visible);


			GRenderingComponents::interfaceMeshManager().setVisibility(m_rgizmo_xaxis, false);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_rgizmo_yaxis, false);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_rgizmo_zaxis, false);
			GRenderingComponents::pickingManager().setPickable(m_rgizmo_xaxis, false);
			GRenderingComponents::pickingManager().setPickable(m_rgizmo_yaxis, false);
			GRenderingComponents::pickingManager().setPickable(m_rgizmo_zaxis, false);
			break;
		case Editor::TransformTool::ROTATE:
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_xaxis, false);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_yaxis, false);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_zaxis, false);
			GRenderingComponents::pickingManager().setPickable(m_tgizmo_xaxis, false);
			GRenderingComponents::pickingManager().setPickable(m_tgizmo_yaxis, false);
			GRenderingComponents::pickingManager().setPickable(m_tgizmo_zaxis, false);

			GRenderingComponents::interfaceMeshManager().setVisibility(m_rgizmo_xaxis, visible);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_rgizmo_yaxis, visible);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_rgizmo_zaxis, visible);
			GRenderingComponents::pickingManager().setPickable(m_rgizmo_xaxis, visible);
			GRenderingComponents::pickingManager().setPickable(m_rgizmo_yaxis, visible);
			GRenderingComponents::pickingManager().setPickable(m_rgizmo_zaxis, visible);
			break;
		case Editor::TransformTool::SCALE:
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_xaxis, false);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_yaxis, false);
			GRenderingComponents::interfaceMeshManager().setVisibility(m_tgizmo_zaxis, false);
			break;
		default:
			break;
		}
	}

	void TransformTool::setGizmoPosition(Vec3 new_position)
	{
		GCoreComponents::transformManager().setPosition(m_tgizmo_xaxis, new_position);
		GCoreComponents::transformManager().setPosition(m_tgizmo_yaxis, new_position);
		GCoreComponents::transformManager().setPosition(m_tgizmo_zaxis, new_position);

		GCoreComponents::transformManager().setPosition(m_rgizmo_xaxis, new_position);
		GCoreComponents::transformManager().setPosition(m_rgizmo_yaxis, new_position);
		GCoreComponents::transformManager().setPosition(m_rgizmo_zaxis, new_position);

		GCoreComponents::transformManager().setPosition(m_sgizmo_xaxis, new_position);
		GCoreComponents::transformManager().setPosition(m_sgizmo_yaxis, new_position);
		GCoreComponents::transformManager().setPosition(m_sgizmo_zaxis, new_position);
	}

	void TransformTool::moveGizmo(Vec3 translation)
	{
		GCoreComponents::transformManager().translate(m_tgizmo_xaxis, translation);
		GCoreComponents::transformManager().translate(m_tgizmo_yaxis, translation);
		GCoreComponents::transformManager().translate(m_tgizmo_zaxis, translation);
		GCoreComponents::transformManager().translate(m_rgizmo_xaxis, translation);
		GCoreComponents::transformManager().translate(m_rgizmo_yaxis, translation);
		GCoreComponents::transformManager().translate(m_rgizmo_zaxis, translation);
		GCoreComponents::transformManager().translate(m_sgizmo_xaxis, translation);
		GCoreComponents::transformManager().translate(m_sgizmo_yaxis, translation);
		GCoreComponents::transformManager().translate(m_sgizmo_zaxis, translation);
	}

	void TransformTool::translate(double dx, double dy, double x, double y)
	{
		Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());

		auto selected_entities = GTools::selectManager().getAllSelectedEntities();

		Vec3 avg_positiong = Vec3(0.0f);
		float normalization = 0.0;
		for (auto target : selected_entities)
		{
			avg_positiong += GCoreComponents::transformManager().getWorldPosition(target);
			normalization += 1.0f;
		}
		avg_positiong /= normalization;

		Vec3 tgt_pos = avg_positiong;
		Vec3 cam_pos = GCoreComponents::transformManager().getWorldPosition(active_camera);

		// Compute tgt pos and tgt + transform axisvector to screenspace
		Mat4x4 view_mx = glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(active_camera)));
		Mat4x4 proj_mx = GCoreComponents::cameraManager().getProjectionMatrix(GCoreComponents::cameraManager().getIndex(active_camera));
		Vec4 obj_ss = proj_mx * view_mx * Vec4(tgt_pos, 1.0f);
		obj_ss /= obj_ss.w;

		Vec4 transform_tgt = Vec4(tgt_pos + m_axis[m_active_axis], 1.0);
		Vec4 transform_tgt_ss = proj_mx * view_mx * transform_tgt;
		transform_tgt_ss /= transform_tgt_ss.w;
		
		Vec2 transform_axis_ss = Vec2(transform_tgt_ss.x, transform_tgt_ss.y) - Vec2(obj_ss.x, obj_ss.y);

		Vec2 mouse_move = Vec2(static_cast<float>(dx), static_cast<float>(dy)) * 2.0f;

		float scale = 0.0f;

		if( glm::length(transform_axis_ss) > 0.0 )
			scale = dot(glm::normalize(transform_axis_ss), glm::normalize(mouse_move)) * (glm::length(mouse_move) / glm::length(transform_axis_ss));

		moveGizmo(scale *  m_axis[m_active_axis]);

		// transform all targets
		for (auto target : selected_entities)
		{
			glm::mat3 inv_vector_model_mx = transpose(glm::mat3(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(target))));

			//GCoreComponents::transformManager().translate(target, inv_vector_model_mx * (scale * m_axis[m_active_axis]));
			GCoreComponents::transformManager().translate(target, (scale * m_axis[m_active_axis]));

			// update targets as they asked us to
			GTools::selectManager().getUpdateFunction(target)();
		}
	}

	void TransformTool::rotate(double dx, double dy, double x, double y)
	{
		auto selected_entities = GTools::selectManager().getAllSelectedEntities();

		Vec3 avg_positiong = Vec3(0.0f);
		float normalization = 0.0;
		for (auto target : selected_entities)
		{
			avg_positiong += GCoreComponents::transformManager().getWorldPosition(target);
			normalization += 1.0f;
		}
		avg_positiong /= normalization;

		//  compute intersection with axis plane
		uint cam_idx = GCoreComponents::cameraManager().getActiveCameraIndex();
		Entity active_cam = GCoreComponents::cameraManager().getEntity(cam_idx);
		float aspect_ratio = GCoreComponents::cameraManager().getAspectRatio(cam_idx);
		float fovy = GCoreComponents::cameraManager().getFovy(cam_idx);
		Vec3 cam_pos = GCoreComponents::transformManager().getWorldPosition(active_cam);
		Vec3 cam_dir_stop = glm::normalize(Vec3(Vec2(tan(fovy / 2.0f) * aspect_ratio, tan(fovy / 2.0f)) * ((Vec2(x, y)*2.0f) - 1.0f), -1.0f));
		Vec3 cam_dir_start = glm::normalize(Vec3(Vec2(tan(fovy / 2.0f) * aspect_ratio, tan(fovy / 2.0f)) * (((Vec2(x, y) - Vec2(dx, dy))*2.0f) - 1.0f), -1.0f));

		Mat4x4 cam_model_mx = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(active_cam));
		cam_dir_stop = glm::transpose(glm::inverse(glm::mat3(cam_model_mx))) * cam_dir_stop;
		cam_dir_start = glm::transpose(glm::inverse(glm::mat3(cam_model_mx))) * cam_dir_start;

		Vec3 plane_pos = avg_positiong;
		Vec3 plane_normal = m_axis[m_active_axis];

		float d_stop = glm::dot((plane_pos - cam_pos), plane_normal) / glm::dot(cam_dir_stop, plane_normal);
		float d_start = glm::dot((plane_pos - cam_pos), plane_normal) / glm::dot(cam_dir_start, plane_normal);

		Vec3 intersection_stop = cam_pos + d_stop * cam_dir_stop;
		Vec3 intersection_start = cam_pos + d_start * cam_dir_start;

		float angle = std::acos( std::min(std::max( dot( glm::normalize(intersection_stop-plane_pos), glm::normalize(intersection_start-plane_pos)), -1.0f), 1.0f) );

		float sign = dot(glm::normalize(intersection_stop - plane_pos), glm::cross(glm::normalize(intersection_start - plane_pos), plane_normal)) > 0.0f ? -1.0f : 1.0f ;

		glm::quat rotation = glm::rotate(glm::quat(), sign*angle, m_axis[m_active_axis]);
		

		std::cout << "Rotation Angle: " << sign*angle << std::endl;

		// transform all targets
		for (auto target : selected_entities)
		{
			uint transform_idx = GCoreComponents::transformManager().getIndex(target);

			glm::mat3 inv_vector_model_mx = transpose(glm::mat3(GCoreComponents::transformManager().getWorldTransformation(transform_idx)));

			//GCoreComponents::transformManager().translate(target, inv_vector_model_mx * (scale * m_axis[m_active_axis]));
			GCoreComponents::transformManager().rotate(transform_idx, rotation);

			// update targets as they asked us to
			GTools::selectManager().getUpdateFunction(target)();
		}
	}
}