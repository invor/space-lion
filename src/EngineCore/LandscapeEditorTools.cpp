#include "LandscapeEditorTools.hpp"

#include "Frame.hpp"

#include "GlobalEngineCore.hpp"
#include "DeferredRenderingPipeline.hpp"

#include "GlobalRenderingComponents.hpp"
#include "StaticMeshComponent.hpp"
#include "PickingComponent.hpp"

#include "GlobalCoreComponents.hpp"
#include "CameraComponent.hpp"
#include "TransformComponent.hpp"

#include "Controls.hpp"

Editor::ControlVertexTools::ControlVertexTools()
	: m_target(GEngineCore::entityManager().create())	// create "dummy" entity
{
}

Editor::ControlVertexTools::~ControlVertexTools()
{

}

void Editor::ControlVertexTools::activate(Entity target)
{
	m_target = target;

	GTools::transformTool().activate();

	Controls::setEditorMouseCtrlCallback(std::bind(&Editor::ControlVertexTools::insert,this,std::placeholders::_1, std::placeholders::_2));
}

void Editor::ControlVertexTools::deactivate()
{
	GTools::transformTool().deactivate();

	Controls::setEditorMouseCtrlCallback([](auto a, auto b) {});
}

void Editor::ControlVertexTools::setRealtimeUpdate(bool rt_update)
{
	m_realtime_update = rt_update;
}

void Editor::ControlVertexTools::update()
{
	GLandscapeComponents::featureCurveManager().updateCorrespondingCurve(m_target.id());

	if (this->m_realtime_update)
		GLandscapeComponents::landscapeManager().updateBricks(0);
}

void Editor::ControlVertexTools::insert(double x, double y)
{
	//TODO compute insertion point based on (normalized) x, y, camera_fov and camera position

	uint cam_idx = GCoreComponents::cameraManager().getActiveCameraIndex();
	Entity active_cam = GCoreComponents::cameraManager().getEntity(cam_idx);
	float aspect_ratio = GCoreComponents::cameraManager().getAspectRatio(cam_idx);
	float fovy = GCoreComponents::cameraManager().getFovy(cam_idx);
	Vec3 cam_pos = GCoreComponents::transformManager().getWorldPosition(active_cam);
	Vec3 cam_dir = glm::normalize(Vec3(Vec2(tan(fovy / 2.0f) * aspect_ratio, tan(fovy / 2.0f)) * ((Vec2(x, y)*2.0f) - 1.0f), -1.0f));
	Mat4x4 cam_model_mx = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(active_cam));
	cam_dir = glm::transpose(glm::inverse(glm::mat3(cam_model_mx))) * cam_dir;

	// compute insertion point
	Vec3 plane_pos = GCoreComponents::transformManager().getWorldPosition(m_target);
	Vec3 plane_normal = glm::transpose(glm::inverse(glm::mat3(cam_model_mx))) * Vec3(0.0, 0.0, 1.0);

	// choose coordiante plane that aligns with camera view plane the most
	if (abs(plane_normal.x) > abs(plane_normal.y) && abs(plane_normal.x) > abs(plane_normal.z))
		plane_normal = Vec3(1.0, 0.0, 0.0);
	else if(abs(plane_normal.y) > abs(plane_normal.x) && abs(plane_normal.y) > abs(plane_normal.z) )
		plane_normal = Vec3(0.0, 1.0, 0.0);
	else if(abs(plane_normal.z) > abs(plane_normal.x) && abs(plane_normal.z) > abs(plane_normal.y) )
		plane_normal = Vec3(0.0, 0.0, 1.0);


	float d = glm::dot((plane_pos - cam_pos), plane_normal) / glm::dot(cam_dir, plane_normal);
	Vec3 intersection = cam_pos + d * cam_dir;

	Entity curve_entity = GLandscapeComponents::featureCurveManager().getCurveFromControlVertex(m_target);

	Vec3 insert_pos = Vec3( glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(curve_entity))) * Vec4(intersection,1.0) );

	//TODO check if insertion is between target and next oder previous and target
	Vec3 target_pos = plane_pos;
	Entity previous_cv = GLandscapeComponents::featureCurveManager().getPreviousCV(m_target);
	Entity next_cv = GLandscapeComponents::featureCurveManager().getNextCV(m_target);

	// select previous or next cv as comparison
	Vec3 compare_pos = (previous_cv == m_target) ? GCoreComponents::transformManager().getWorldPosition(next_cv) : GCoreComponents::transformManager().getWorldPosition(previous_cv);

	bool insert_behind = (glm::length(compare_pos - insert_pos) > glm::length(target_pos - compare_pos));
	
	//flip bool if no previous cv
	insert_behind = (previous_cv == m_target) ? !insert_behind : insert_behind;

	Entity new_cv = GLandscapeComponents::featureCurveManager().insertControlVertex(curve_entity, m_target, insert_pos, insert_behind);

	if (m_realtime_update)
		GLandscapeComponents::landscapeManager().updateAll();

	//TODO set target to newly inserted vertex

	m_target = new_cv;
}


Editor::ConstraintPointTools::ConstraintPointTools()
	: m_target(GEngineCore::entityManager().create()),	// create "dummy" entity
	m_isActive(false)
{
	// TODO add interface draw job to render pipeline ?
	// (and only draw windows when tool is active)

	//GEngineCore::renderingPipeline()).addPerFrameInterfaceGpuTask(std::bind(&Editor::ConstraintPointTools::drawInterface, this));
}

Editor::ConstraintPointTools::~ConstraintPointTools()
{
}

void Editor::ConstraintPointTools::activate(Entity target)
{
	m_isActive = true;

	m_target = target;

	//GTools::featureCurveGradientManipulator().activate(target, []() {});

	//Controls::setEditorMouseCtrlCallback(std::bind(&Editor::ConstraintPointTools::insert, this, std::placeholders::_1, std::placeholders::_2));
}

void Editor::ConstraintPointTools::deactivate()
{
	m_isActive = false;

	//GTools::featureCurveGradientManipulator().deactivate();

	//Controls::setEditorMouseCtrlCallback([](auto a, auto b) {});
}

void Editor::ConstraintPointTools::insert(double x, double y)
{
	//GLandscapeComponents::featureCurveManager().addConstraintPoint()
	float tgt_curve_position = GLandscapeComponents::featureCurveManager().getConstraintPointCurvePosition(m_target);

	GLandscapeComponents::featureCurveManager().addConstraintPoint(m_target, tgt_curve_position);
}

void Editor::ConstraintPointTools::setNoiseRoughness(float roughness)
{
	if (!m_isActive)
		return;

	Entity cp = GLandscapeComponents::featureCurveManager().getConstraintPointFromGradient(m_target);
	GLandscapeComponents::featureCurveManager().setConstraintPointNoiseRoughness(cp, roughness);
}

void Editor::ConstraintPointTools::setNoiseAmplitude(float amplitude)
{
	if (!m_isActive)
		return;

	Entity cp = GLandscapeComponents::featureCurveManager().getConstraintPointFromGradient(m_target);

	GLandscapeComponents::featureCurveManager().setConstraintPointNoiseAmplitude(cp, amplitude);
}

void Editor::ConstraintPointTools::setRealtimeUpdate(bool rt_update)
{
	m_realtime_update = rt_update;
}

void Editor::ConstraintPointTools::drawInterface()
{
	if (m_isActive)
	{
		//TODO get position of Constraint Point in Screenspace
		uint cam_idx = GCoreComponents::cameraManager().getActiveCameraIndex();
		Mat4x4 cam_model_mx = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex( GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex()) ));
		Mat4x4 view_mx = glm::inverse(cam_model_mx);
		Mat4x4 proj_mx = GCoreComponents::cameraManager().getProjectionMatrix(cam_idx);
		Vec3 cp_w_position = GCoreComponents::transformManager().getWorldPosition(m_target);
		Vec4 cp_ss_position = (proj_mx * view_mx * Vec4(cp_w_position, 1.0f));
		cp_ss_position /= cp_ss_position.w;
		cp_ss_position += Vec4(1.0f, 1.0f, 0.0f, 0.0f);
		cp_ss_position /= Vec4(2.0f,2.0f,0.0f,0.0f);
		cp_ss_position.y = 1.0f - cp_ss_position.y;

		ImVec2 window_pos = ImVec2(cp_ss_position.x * GEngineCore::renderingPipeline().getGBuffer()->getWidth() + 150.0f, cp_ss_position.y * GEngineCore::renderingPipeline().getGBuffer()->getHeight());

		//	ImVec2 move_vec = ImVec2(previous_window_pos.x - window_pos.x, previous_window_pos.y - window_pos.y);
		//	int x = ImGui::GetCursorPosX();
		//	int y = ImGui::GetCursorPosY();
		//	ImGui::SetCursorPosX(x + move_vec.x);
		//	ImGui::SetCursorPosY(y + move_vec.y);
		//	
		//	previous_window_pos = window_pos;
		//	
		//	ImGui::SetNextWindowPos(window_pos);

		bool p_open = true;
		if (!ImGui::Begin("Constraint Point Tools", &p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		Entity cp = GLandscapeComponents::featureCurveManager().getConstraintPointFromGradient(m_target);
		float roughness = GLandscapeComponents::featureCurveManager().getConstraintPointNoiseRoughness(cp);
		float amplitude = GLandscapeComponents::featureCurveManager().getConstraintPointNoiseAmplitude(cp);
		std::pair<int,int> matIDs = GLandscapeComponents::featureCurveManager().getConstraintPointMaterialIDs(cp);
		float curve_position = GLandscapeComponents::featureCurveManager().getConstraintPointCurvePosition(cp);

		bool update = false;

		update |= (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 40.0f));

		update |= (ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 10.0f));

		update |= (ImGui::SliderInt("MaterialID_0", &matIDs.first, 0, 3));

		update |= (ImGui::SliderInt("MaterialID_1", &matIDs.second, 0, 3));
 
		if (curve_position > 0.0 && curve_position < 1.0)
		{
			ImGui::Separator();
			update |= (ImGui::SliderFloat("Curve Position", &curve_position, 0.01f, 0.99f));
		}

		if (ImGui::Button("Insert CP", ImVec2(120, 0)))
		{
			Entity cp = GLandscapeComponents::featureCurveManager().getConstraintPointFromGradient(m_target);
			Entity curve = GLandscapeComponents::featureCurveManager().getCurveFromConstraintPoint(cp);

			float insert_position = curve_position;
			insert_position = (insert_position < 0.01f) ? insert_position + 0.1f : insert_position;
			insert_position = (insert_position > 0.99f) ? insert_position - 0.1f : insert_position;

			GLandscapeComponents::featureCurveManager().addConstraintPoint(curve, insert_position );
		}

		if (update)
		{
			GLandscapeComponents::featureCurveManager().setConstraintPointNoiseRoughness(cp, roughness);
			GLandscapeComponents::featureCurveManager().setConstraintPointNoiseAmplitude(cp, amplitude);
			GLandscapeComponents::featureCurveManager().setConstraintPointCurvePosition(cp, curve_position);
			GLandscapeComponents::featureCurveManager().setConstrainPointMaterialIDs(cp, matIDs.first, matIDs.second);

			//bit complicated...
			//GTools::featureCurveGradientManipulator().setPosition(GCoreComponents::transformManager().getWorldPosition(m_target));
			//GTools::featureCurveGradientManipulator().setRotationalAxis(GLandscapeComponents::featureCurveManager().getConstraintPointTangent(cp));
			//GTools::featureCurveGradientManipulator().deactivate();
			GTools::featureCurveGradientManipulator().activate(m_target, []() {});

			if (m_realtime_update)
				GLandscapeComponents::landscapeManager().updateAll();
		}

		ImGui::End();

	}
}


Editor::FeatureCurveGradientManipulator::FeatureCurveGradientManipulator()
	: m_gizmo_circle(GEngineCore::entityManager().create()),
		m_target(m_gizmo_circle), m_rotational_axis(Vec3(0.0,0.0,1.0))
{
	GCoreComponents::transformManager().addComponent(m_gizmo_circle, Vec3(), Quat(), Vec3(0.5));

	GRenderingComponents::interfaceMeshManager().addComponent(m_gizmo_circle, "../resources/materials/editor/interface_gradient_gizmo.slmtl", "../resources/meshes/editor/gradient_gizmo.fbx",false);

	GRenderingComponents::pickingManager().addComponent(m_gizmo_circle, "../resources/materials/editor/gradient_gizmo_picking.slmtl", "../resources/meshes/editor/gradient_gizmo.fbx",
		[this]() { Controls::setEditorMouseMoveCallback(std::bind(&FeatureCurveGradientManipulator::transform, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
					this->setVisible(true);
					Controls::setEditorMouseReleaseCallback([]() {Controls::setEditorMouseMoveCallback([](double,double,double,double) {}); });
					});

	setVisible(false);
}

Editor::FeatureCurveGradientManipulator::~FeatureCurveGradientManipulator()
{

}

void Editor::FeatureCurveGradientManipulator::activate(Entity target, std::function<void()> update)
{
	m_target = target;
	m_target_update = update;

	setVisible(true);
	setPosition(GCoreComponents::transformManager().getWorldPosition(m_target));
	Entity cp_entity = GLandscapeComponents::featureCurveManager().getConstraintPointFromGradient(target);
	setRotationalAxis( GLandscapeComponents::featureCurveManager().getConstraintPointTangent(cp_entity) );
}

void Editor::FeatureCurveGradientManipulator::deactivate()
{
	setVisible(false);
}

void Editor::FeatureCurveGradientManipulator::setPosition(Vec3 new_position)
{
	GCoreComponents::transformManager().setPosition(m_gizmo_circle, new_position);
}

void Editor::FeatureCurveGradientManipulator::setRotationalAxis(Vec3 rot_axis)
{
	float angle = std::acos(glm::dot(glm::normalize(rot_axis), Vec3(0.0,0.0,1.0)));

	if (angle < 0.01)
		return;

	Vec3 axis = glm::normalize( glm::cross(Vec3(0.0, 0.0, 1.0), rot_axis) );

	GCoreComponents::transformManager().setOrientation(GCoreComponents::transformManager().getIndex(m_gizmo_circle),glm::angleAxis(angle, axis));
	m_rotational_axis = glm::normalize(rot_axis);


	//	Vec3 v0 = GCoreComponents::transformManager().getWorldPosition(m_gizmo_circle);
	//	Vec3 v1 = v0 + 1.5f * rot_axis;
	//	std::vector<float> cv_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
	//		v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0});
	//	std::vector<uint> cv_interface_indices({ 0,1 });
	//	
	//	VertexDescriptor vertex_description(28, { VertexDescriptor::Attribute(GL_FLOAT,3,GL_FALSE,0),
	//		VertexDescriptor::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });
	//	
	//	Entity dummy = GEngineCore::entityManager().create();
	//	GCoreComponents::transformManager().addComponent(dummy);
	//	GRenderingComponents::interfaceMeshManager().addComponent(dummy,
	//		"dummy" + std::to_string(dummy.id()),
	//		"../resources/materials/editor/interface_cv.slmtl",
	//		cv_interface_vertices,
	//		cv_interface_indices,
	//		vertex_description,
	//		GL_LINES);
}

void Editor::FeatureCurveGradientManipulator::setVisible(bool visible)
{
	GRenderingComponents::interfaceMeshManager().setVisibility(m_gizmo_circle, visible);
	GRenderingComponents::pickingManager().setPickable(m_gizmo_circle, visible);
}

void Editor::FeatureCurveGradientManipulator::setRealtimeUpdate(bool rt_update)
{
	m_realtime_update = rt_update;
}

void Editor::FeatureCurveGradientManipulator::transform(double dx, double dy, double x, double y)
{
	//  compute intersection with tangent plane
	uint cam_idx = GCoreComponents::cameraManager().getActiveCameraIndex();
	Entity active_cam = GCoreComponents::cameraManager().getEntity(cam_idx);
	float aspect_ratio = GCoreComponents::cameraManager().getAspectRatio( cam_idx );
	float fovy = GCoreComponents::cameraManager().getFovy(cam_idx);
	Vec3 cam_pos = GCoreComponents::transformManager().getWorldPosition(active_cam);
	Vec3 cam_dir = glm::normalize(Vec3( Vec2(tan(fovy / 2.0f) * aspect_ratio, tan(fovy/2.0f)) * ((Vec2(x,y)*2.0f) - 1.0f), -1.0f));

	Mat4x4 cam_model_mx = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(active_cam));
	cam_dir = glm::transpose(glm::inverse(glm::mat3(cam_model_mx))) * cam_dir;

	Vec3 plane_pos = GCoreComponents::transformManager().getWorldPosition(m_target);
	Entity cp_entity = GLandscapeComponents::featureCurveManager().getConstraintPointFromGradient(m_target);
	Vec3 plane_normal = GLandscapeComponents::featureCurveManager().getConstraintPointTangent(cp_entity);

	float d = glm::dot((plane_pos - cam_pos), plane_normal) / glm::dot(cam_dir, plane_normal);

	Vec3 intersection = cam_pos + d * cam_dir;

	// compute new constraint gradient
	Mat4x4 trans_model = glm::transpose(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(m_target)));
	glm::mat3 vector_transform(trans_model);

	Vec3 new_gradient = vector_transform * glm::normalize(intersection - plane_pos);
	
	GLandscapeComponents::featureCurveManager().setConstraintPointGradient(m_target, new_gradient);

	if (m_realtime_update)
		GLandscapeComponents::landscapeManager().updateAll();
}


Editor::FeatureCurveTools::FeatureCurveTools()
	: m_target(GEngineCore::entityManager().create()) //create dummy entity...TODO: think about better solution
{

}

void Editor::FeatureCurveTools::activate(Entity target)
{
	m_target = target;

	GTools::transformTool().activate();
}

void Editor::FeatureCurveTools::deactivate()
{
	GTools::transformTool().deactivate();
}

void Editor::FeatureCurveTools::update()
{
	if (m_realtime_update)
		GLandscapeComponents::landscapeManager().updateCorrespondingLandscape(m_target);
}

void Editor::FeatureCurveTools::setRealtimeUpdate(bool rt_update)
{
	m_realtime_update = rt_update;
}


Editor::FeatureMeshTools::FeatureMeshTools()
	: m_target(GEngineCore::entityManager().create()) //create dummy entity...TODO: think about better solution
{

}

void Editor::FeatureMeshTools::activate(Entity target)
{
	m_target = target;

	GTools::transformTool().activate();
}

void Editor::FeatureMeshTools::deactivate()
{
	GTools::transformTool().deactivate();
}

void Editor::FeatureMeshTools::update()
{
	if (m_realtime_update)
		GLandscapeComponents::landscapeManager().updateCorrespondingLandscape(m_target);
}

void Editor::FeatureMeshTools::setRealtimeUpdate(bool rt_update)
{
	m_realtime_update = rt_update;
}


Editor::LandscapeTools::LandscapeTools()
{
	GEngineCore::renderingPipeline().addPerFrameGpuTask(std::bind(&Editor::LandscapeTools::drawInterface, this, std::placeholders::_1),DeferredRenderingPipeline::RenderPass::INTERFACE_PASS);
}

Editor::LandscapeTools::~LandscapeTools()
{

}

void Editor::LandscapeTools::activateControlVertexTools(Entity target)
{
	m_cv_tools.activate(target);
}

void Editor::LandscapeTools::activateConstraintPointTools(Entity target)
{
	m_cp_tools.activate(target);
	m_fc_gradient_tool.activate(target, []() {});
}

void Editor::LandscapeTools::deactivateControlVertexTools()
{
	m_cv_tools.deactivate();
}

void Editor::LandscapeTools::deactivateConstraintPointTools()
{
	m_cp_tools.deactivate();
	m_fc_gradient_tool.deactivate();
}

void Editor::LandscapeTools::activateFeatureCurveTools(Entity target)
{
	m_fc_tools.activate(target);
}

void Editor::LandscapeTools::deactivateFeatureCurveTools()
{
	m_fc_tools.deactivate();
}


void Editor::LandscapeTools::activateFeatureMeshTool(Entity target)
{
	m_fm_tools.activate(target);
}

void Editor::LandscapeTools::deactivateFeatureMeshTool()
{
	m_fm_tools.deactivate();
}


void Editor::LandscapeTools::setRealtimeUpdate(bool rt_update)
{
	m_cv_tools.setRealtimeUpdate(rt_update);
	m_cp_tools.setRealtimeUpdate(rt_update);
	m_fc_gradient_tool.setRealtimeUpdate(rt_update);
	m_fc_tools.setRealtimeUpdate(rt_update);
	m_fm_tools.setRealtimeUpdate(rt_update);
}

void Editor::LandscapeTools::drawInterface(const Frame& frame)
{
	m_cp_tools.drawInterface();
}