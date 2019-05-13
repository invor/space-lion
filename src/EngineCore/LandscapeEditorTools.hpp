
#include "GlobalLandscapeComponents.hpp"

#ifndef LandscapeEditorTools_hpp
#define LandscapeEditorTools_hpp

#include "TerrainLoader.hpp"
#include "TransformTool.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))

namespace Editor
{
	class ControlVertexTools
	{
	private:
		Entity m_target;

		bool m_realtime_update;
	public:
		ControlVertexTools();
		~ControlVertexTools();

		void activate(Entity target);
		void deactivate();
		void update();

		void setRealtimeUpdate(bool rt_update);

		void insert(double x, double y);
	};

	class ConstraintPointTools
	{
	private:
		Entity m_target;

		bool m_isActive;

		bool m_realtime_update;

		ImVec2 previous_window_pos;

	public:
		ConstraintPointTools();
		~ConstraintPointTools();

		void activate(Entity target);
		void deactivate();

		void insert(double x, double y);

		void setNoiseRoughness(float roughness);

		void setNoiseAmplitude(float amplitude);

		void setRealtimeUpdate(bool rt_update);

		void drawInterface();
	};

	class FeatureCurveGradientManipulator
	{
	private:
		Entity m_gizmo_circle;

		Vec3 m_rotational_axis;

		Entity m_target;
		std::function<void()> m_target_update;

		bool m_realtime_update;

	public:
		FeatureCurveGradientManipulator();
		~FeatureCurveGradientManipulator();

		void activate(Entity target, std::function<void()> update);
		void deactivate();

		void setPosition(Vec3 new_position);

		void setRotationalAxis(Vec3 rot_axis);

		void setVisible(bool visible);

		void setRealtimeUpdate(bool rt_update);

		void transform(double dx, double dy, double x, double y);
	};

	class FeatureCurveTools
	{
	private:
		Entity m_target;

		bool m_realtime_update;
	public:
		FeatureCurveTools();

		void activate(Entity target);
		void deactivate();
		void update();

		void setRealtimeUpdate(bool rt_update);
	};

	class FeatureMeshTools
	{
	private:
		Entity m_target;

		bool m_realtime_update;
	public:
		FeatureMeshTools();

		void activate(Entity target);
		void deactivate();
		void update();

		void setRealtimeUpdate(bool rt_update);
	};

	class LandscapeTools
	{
	private:
		ControlVertexTools m_cv_tools;

		ConstraintPointTools m_cp_tools;

		FeatureCurveGradientManipulator m_fc_gradient_tool;

		FeatureCurveTools m_fc_tools;

		FeatureMeshTools m_fm_tools;

		bool m_realtime_update;

	public:
		LandscapeTools();
		~LandscapeTools();

		void activateControlVertexTools(Entity target);
		void activateConstraintPointTools(Entity target);
		void activateFeatureCurveTools(Entity target);
		void activateFeatureMeshTool(Entity target);

		void deactivateControlVertexTools();
		void deactivateConstraintPointTools();
		void deactivateFeatureCurveTools();
		void deactivateFeatureMeshTool();

		void setRealtimeUpdate(bool rt_update);

		void drawInterface(const Frame&);

		ControlVertexTools& getControlVertexTool() { return m_cv_tools; }
		ConstraintPointTools& getConstraintPointTool() { return m_cp_tools; }
		FeatureCurveGradientManipulator& getFeatureCurveGradientManipulator() { return m_fc_gradient_tool; }
	};
}

#endif // !LandscapeEditorTools_hpp