#ifndef EditorUI_hpp
#define EditorUI_hpp

#include "types.hpp"

struct Frame;

namespace Editor
{

	class EditorUI
	{
	public:
		EditorUI();
		~EditorUI();

	private:
		bool m_show_componentProperties;
		bool m_show_outliner;

		bool m_show_landscapeImportPopup;
		bool m_show_landscapeExportPopup;
		bool m_show_landscapeExportMeshPopup;

		bool m_show_landscapeImportMeshPopup;
		bool m_show_landscapeImportHeightMapPopup;

		bool m_show_featureCurves;
		bool m_show_heightmapInterfaceMeshes;
		bool m_show_featureMeshes;
		bool m_show_landscapeBrickBBs;
		bool m_show_landscapeMeshes;

		bool m_show_pointlights;
		bool m_show_decals;

		bool m_landscape_realtimeUpdate;

		bool m_landscape_voxFeatureCurves;
		bool m_landscape_voxFeatureMeshes;
		bool m_landscape_voxHeightmaps;

		// The main draw functions of the editor UI
		void drawMainMenuBar(const Frame& frame);
		void drawPopups(const Frame& frame);
		void drawWindows(const Frame& frame);

		// The sub-functions called by main draw functions
		void drawFileMenu();
		void drawCreateMenu();
		void drawWindowMenu();
		void drawShowHideMenu();
		void drawLandscapeMenu();

		void drawOutliner();

		void drawComponentPropertyWindow();
		void drawAirplanePhysicsComponentProperties();
		void drawCameraComponentProperties();
		void drawLandscapeComponentProperties();
		void drawTransformComponentProperties();
		void drawOceanComponentProperties();
		void drawPointlightComponentProperties();
		void drawSunlightComponentProperties();
		void drawDecalComponentProperties();
		void drawRearSteerBicycleComponentProperties();
	};

}

#endif // !EditorUI_hpp
