#include "EditorUI.hpp"

#include "ResourceLoading.hpp"

#include "Frame.hpp"
#include "TaskSchedueler.hpp"

#include "GlobalRenderingComponents.hpp"
#include "PickingComponent.hpp"
#include "StaticMeshComponent.hpp"
#include "OceanComponent.hpp"
#include "VolumeComponent.hpp"
#include "DecalComponent.hpp"

#include "GlobalCoreComponents.hpp"
#include "AirplanePhysicsComponent.hpp"
#include "CameraComponent.hpp"
#include "PointlightComponent.hpp"
#include "SunlightComponent.hpp"
#include "TransformComponent.hpp"
#include "DebugNameComponent.hpp"
#include "RearSteerBicycleComponent.hpp"

#include "GlobalLandscapeComponents.hpp"
#include "Terrain.hpp"
#include "PointlightComponent.hpp"

#include "EditorSelectComponent.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), static_cast<int>(values.size()));
	}

	bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), static_cast<int>(values.size()));
	}

}

namespace Editor
{

	EditorUI::EditorUI()
		: m_show_featureCurves(true),
		m_show_heightmapInterfaceMeshes(true),
		m_show_featureMeshes(true),
		m_show_landscapeBrickBBs(true),
		m_show_landscapeMeshes(true),
		m_show_pointlights(true),
		m_show_decals(true),
		m_landscape_voxFeatureCurves(true),
		m_landscape_voxFeatureMeshes(true),
		m_landscape_voxHeightmaps(true)
	{
		GEngineCore::renderingPipeline().addPerFrameGpuTask(std::bind(&Editor::EditorUI::drawMainMenuBar, this, std::placeholders::_1), DeferredRenderingPipeline::RenderPass::INTERFACE_PASS);
		GEngineCore::renderingPipeline().addPerFrameGpuTask(std::bind(&Editor::EditorUI::drawPopups, this, std::placeholders::_1), DeferredRenderingPipeline::RenderPass::INTERFACE_PASS);
		GEngineCore::renderingPipeline().addPerFrameGpuTask(std::bind(&Editor::EditorUI::drawWindows, this, std::placeholders::_1), DeferredRenderingPipeline::RenderPass::INTERFACE_PASS);
	}

	EditorUI::~EditorUI()
	{

	}

	void EditorUI::drawMainMenuBar(const Frame& frame)
	{
		if (ImGui::BeginMainMenuBar())
		{
			drawFileMenu();
			drawCreateMenu();
			drawWindowMenu();
			drawShowHideMenu();
			drawLandscapeMenu();

			ImGui::EndMainMenuBar();
		}
	}

	void EditorUI::drawPopups(const Frame& frame)
	{
		if (m_show_landscapeImportPopup)  ImGui::OpenPopup("Import");
		if (ImGui::BeginPopupModal("Import", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Import exsting landscape from file");
			ImGui::Separator();

			static char import_filepath[32] = "";

			if (ImGui::InputText("##import_filepath", import_filepath, IM_ARRAYSIZE(import_filepath), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				GEngineCore::taskSchedueler().submitTask(std::bind(Landscape::loadLandscape, import_filepath));
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Button("Import", ImVec2(120, 0)))
			{
				Landscape::loadLandscape(import_filepath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
		m_show_landscapeImportPopup = false;

		if (m_show_landscapeExportPopup)  ImGui::OpenPopup("Export");
		if (ImGui::BeginPopupModal("Export", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Export landscape to file");
			ImGui::Separator();

			auto landscape_list = GLandscapeComponents::landscapeManager().getListofLandscapeEntities();

			std::vector<std::string> items;
			int counter = 0;
			for (auto& lcsp : landscape_list)
				items.push_back(std::to_string(counter++));

			static int item = 0;
			ImGui::Combo("Select Landscape to export", &item, items);

			static char export_filepath[32] = "";

			if (ImGui::InputText("Enter filename", export_filepath, 32, ImGuiInputTextFlags_EnterReturnsTrue))
				Landscape::loadLandscape(export_filepath);

			if (ImGui::Button("Export", ImVec2(120, 0)))
			{
				Landscape::storeLandscape(landscape_list[item], export_filepath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
		m_show_landscapeExportPopup = false;

		if (m_show_landscapeExportMeshPopup)  ImGui::OpenPopup("Export Terrain Mesh");
		if (ImGui::BeginPopupModal("Export Terrain Mesh", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Export landscape mesh to file");
			ImGui::Separator();

			auto landscape_list = GLandscapeComponents::landscapeManager().getListofLandscapeEntities();

			std::vector<std::string> items;
			int counter = 0;
			for (auto& lcsp : landscape_list)
				items.push_back(std::to_string(counter++));

			static int item = 0;
			ImGui::Combo("Select Landscape to export", &item, items);

			static char export_filepath[32] = "";

			if (ImGui::InputText("Enter filename", export_filepath, 32, ImGuiInputTextFlags_EnterReturnsTrue))
				Landscape::loadLandscape(export_filepath);

			if (ImGui::Button("Export", ImVec2(120, 0)))
			{
				GLandscapeComponents::landscapeManager().exportLandscapeMesh(landscape_list[item], export_filepath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
		m_show_landscapeExportMeshPopup = false;


		if (m_show_landscapeImportMeshPopup)  ImGui::OpenPopup("Import mesh");
		if (ImGui::BeginPopupModal("Import mesh", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Import exsting landscape mesh from file");
			ImGui::Separator();

			static char import_filepath[32] = "";

			if (ImGui::InputText("##import_filepath", import_filepath, IM_ARRAYSIZE(import_filepath), ImGuiInputTextFlags_EnterReturnsTrue))
				//Landscape::loadLandscape(import_filepath);
				GLandscapeComponents::landscapeManager().addFeatureMesh(import_filepath);

			if (ImGui::Button("Import", ImVec2(120, 0)))
			{
				//Landscape::loadLandscape(import_filepath);
				GLandscapeComponents::landscapeManager().addFeatureMesh(import_filepath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
		m_show_landscapeImportMeshPopup = false;

		if (m_show_landscapeImportHeightMapPopup) ImGui::OpenPopup("Import heightmap");
		if (ImGui::BeginPopupModal("Import heightmap", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Import heightmap file");
			ImGui::Separator();

			static char import_filepath[32] = ""; //static string size...so sloppy

			if (ImGui::InputText("##import_filepath", import_filepath, IM_ARRAYSIZE(import_filepath), ImGuiInputTextFlags_EnterReturnsTrue))
				//Landscape::loadLandscape(import_filepath);
				GLandscapeComponents::landscapeManager().importHeightmap(import_filepath);

			if (ImGui::Button("Import", ImVec2(120, 0)))
			{
				//Landscape::loadLandscape(import_filepath);
				GLandscapeComponents::landscapeManager().importHeightmap(import_filepath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
		m_show_landscapeImportHeightMapPopup = false;
	}

	void EditorUI::drawWindows(const Frame& frame)
	{
		//TODO use show/hide flags

		drawOutliner();

		drawComponentPropertyWindow();
	}

	void EditorUI::drawFileMenu()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
			}

			ImGui::EndMenu();
		}
	}

	void EditorUI::drawCreateMenu()
	{
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Pointlight"))
			{
				Entity light = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(light, Vec3(0.0, 0.0, 0.0));
				GCoreComponents::pointlightManager().addComponent(light, Vec3(1.0), 100000000, 1000);
				GRenderingComponents::interfaceMeshManager().addComponent(light, "../resources/materials/editor/interface_pointlight.slmtl", "../resources/meshes/editor/pointlight.fbx");
				GRenderingComponents::pickingManager().addComponent(light,
					"../resources/materials/editor/picking.slmtl",
					"../resources/meshes/editor/pointlight.fbx");
				GTools::selectManager().addComponent(light, [light]() { GTools::transformTool().activate(); }, [light]() { GTools::transformTool().deactivate(); });
			}

			if (ImGui::MenuItem("Ocean"))
			{
				Entity ocean = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(ocean, Vec3(0.0, 0.0, 0.0));
				GRenderingComponents::oceanManager().addComponent(ocean, 50.0f, 512.0f, 256);
				GTools::selectManager().addComponent(ocean, []() {}, []() {});
			}

			if (ImGui::MenuItem("Decal"))
			{
				Entity decal = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(decal, Vec3(0.0f, 0.0f, 0.0f));
				GRenderingComponents::decalManager().addComponent(decal, "../resources/materials/decals/slatecliffrock_large.dmtl", 2.0f, 2.0f);
			}
			if (ImGui::MenuItem("Koenigstiescher"))
			{
				Entity tiger_select = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(tiger_select, Vec3(), Quat(), Vec3(1.0f));
				GRenderingComponents::pickingManager().addComponent(tiger_select, "../resources/materials/editor/picking.slmtl", "../resources/meshes/tiger/select.fbx");
				GTools::selectManager().addComponent(tiger_select, []() { GTools::transformTool().activate(); }, []() { GTools::transformTool().deactivate(); });
				//GRenderingComponents::staticMeshManager().addComponent(tiger_select, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/tiger/select.fbx");

				Entity tiger_body_1 = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(tiger_body_1, Vec3(), Quat(), Vec3(1.0f));
				GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(tiger_body_1), tiger_select);
				GRenderingComponents::staticMeshManager().addComponent(tiger_body_1, "../resources/materials/tiger/body_1.slmtl", "../resources/meshes/tiger/body_1.fbx");

				Entity tiger_body_2 = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(tiger_body_2, Vec3(), Quat(), Vec3(1.0f));
				GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(tiger_body_2), tiger_select);
				GRenderingComponents::staticMeshManager().addComponent(tiger_body_2, "../resources/materials/tiger/body_2.slmtl", "../resources/meshes/tiger/body_2.fbx");

				Entity tiger_tracks = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(tiger_tracks, Vec3(), Quat(), Vec3(1.0f));
				GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(tiger_tracks), tiger_select);
				GRenderingComponents::staticMeshManager().addComponent(tiger_tracks, "../resources/materials/tiger/tracks.slmtl", "../resources/meshes/tiger/tracks.fbx");

				Entity tiger_turret_1 = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(tiger_turret_1, Vec3(), Quat(), Vec3(1.0f));
				GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(tiger_turret_1), tiger_select);
				GRenderingComponents::staticMeshManager().addComponent(tiger_turret_1, "../resources/materials/tiger/turret_1.slmtl", "../resources/meshes/tiger/turret_1.fbx");

				Entity tiger_turret_2 = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(tiger_turret_2, Vec3(), Quat(), Vec3(1.0f));
				GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(tiger_turret_2), tiger_select);
				GRenderingComponents::staticMeshManager().addComponent(tiger_turret_2, "../resources/materials/tiger/turret_2.slmtl", "../resources/meshes/tiger/turret_2.fbx");

				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([tiger_body_1, tiger_body_2, tiger_tracks, tiger_turret_1, tiger_turret_2]() {
					std::vector<uint8_t> vertex_data;
					std::vector<uint32_t> index_data;
					VertexLayout vertex_layout;
					ResourceLoading::loadFbxGeometrySlim("../resources/meshes/tiger/body_1.fbx", vertex_data, index_data, vertex_layout);
					Mesh* mesh = GEngineCore::resourceManager().createMesh("tiger_body_1_slim",
						vertex_data,
						index_data,
						vertex_layout,
						GL_TRIANGLES).resource;
					GRenderingComponents::shadowCastMeshManager().addComponent(tiger_body_1, "../resources/materials/shadowCaster.slmtl", mesh);

					ResourceLoading::loadFbxGeometrySlim("../resources/meshes/tiger/body_2.fbx", vertex_data, index_data, vertex_layout);
					mesh = GEngineCore::resourceManager().createMesh("tiger_body_2_slim",
						vertex_data,
						index_data,
						vertex_layout,
						GL_TRIANGLES).resource;
					GRenderingComponents::shadowCastMeshManager().addComponent(tiger_body_2, "../resources/materials/shadowCaster.slmtl", mesh);

					ResourceLoading::loadFbxGeometrySlim("../resources/meshes/tiger/tracks.fbx", vertex_data, index_data, vertex_layout);
					mesh = GEngineCore::resourceManager().createMesh("tiger_tracks_slim",
						vertex_data,
						index_data,
						vertex_layout,
						GL_TRIANGLES).resource;
					GRenderingComponents::shadowCastMeshManager().addComponent(tiger_tracks, "../resources/materials/shadowCaster.slmtl", mesh);

					ResourceLoading::loadFbxGeometrySlim("../resources/meshes/tiger/turret_1.fbx", vertex_data, index_data, vertex_layout);
					mesh = GEngineCore::resourceManager().createMesh("tiger_turret_1_slim",
						vertex_data,
						index_data,
						vertex_layout,
						GL_TRIANGLES).resource;
					GRenderingComponents::shadowCastMeshManager().addComponent(tiger_turret_1, "../resources/materials/shadowCaster.slmtl", mesh);

					ResourceLoading::loadFbxGeometrySlim("../resources/meshes/tiger/turret_2.fbx", vertex_data, index_data, vertex_layout);
					mesh = GEngineCore::resourceManager().createMesh("tiger_turret_2_slim",
						vertex_data,
						index_data,
						vertex_layout,
						GL_TRIANGLES).resource;
					GRenderingComponents::shadowCastMeshManager().addComponent(tiger_turret_2, "../resources/materials/shadowCaster.slmtl", mesh);
				}
				);
			}
			
			if (ImGui::BeginMenu("Bike"))
			{
				ImGui::Text("Simulation file");
				static char file[64] = "../resources/simulationData/data2.txt";
				ImGui::InputText("##import_filepath", file, IM_ARRAYSIZE(file), ImGuiInputTextFlags_EnterReturnsTrue);
				//if (ImGui::InputText("##import_filepath", file, IM_ARRAYSIZE(file), ImGuiInputTextFlags_EnterReturnsTrue))
				//{
				//	Entity bike = GEngineCore::entityManager().create();
				//	GCoreComponents::transformManager().addComponent(bike, Vec3(0.0, 0.0, 0.0));
				//	GCoreComponents::rearSteerBicycleManager().addComponent(bike, file);
				//	GTools::selectManager().addComponent(bike);
				//	GCoreComponents::debugNameManager().addComponent(bike, "RearSteerBicycle");
				//}

				if (ImGui::Button("Create"))
				{
					Entity bike = GEngineCore::entityManager().create();
					GCoreComponents::transformManager().addComponent(bike, Vec3(0.0, 0.0, 0.0));
					GCoreComponents::rearSteerBicycleManager().addComponent(bike, file);
					GTools::selectManager().addComponent(bike);
					GCoreComponents::debugNameManager().addComponent(bike, "RearSteerBicycle");

					Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
					uint idx = GCoreComponents::transformManager().getIndex(active_camera);
					//GCoreComponents::transformManager().setParent(idx, bike);
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
	}

	void EditorUI::drawShowHideMenu()
	{
		if (ImGui::BeginMenu("Show/Hide"))
		{
			if (ImGui::MenuItem("Feature Curves", NULL, m_show_featureCurves))
			{
				m_show_featureCurves = !m_show_featureCurves;

				auto fc_entites = GLandscapeComponents::landscapeManager().getListofFeatureCurveEntities();

				for (auto entity : fc_entites)
				{
					auto cvs = GLandscapeComponents::featureCurveManager().getControlVertices(entity);
					auto cps = GLandscapeComponents::featureCurveManager().getConstraintPoints(entity);

					for (auto cv : cvs)
					{
						GRenderingComponents::interfaceMeshManager().setVisibility(cv.m_entity, m_show_featureCurves);
						GRenderingComponents::pickingManager().setPickable(cv.m_entity, m_show_featureCurves);
					}
					for (auto cp : cps)
					{
						GRenderingComponents::interfaceMeshManager().setVisibility(cp.m_lefthand_gradient, m_show_featureCurves);
						GRenderingComponents::pickingManager().setPickable(cp.m_lefthand_gradient, m_show_featureCurves);

						GRenderingComponents::interfaceMeshManager().setVisibility(cp.m_righthand_gradient, m_show_featureCurves);
						GRenderingComponents::pickingManager().setPickable(cp.m_righthand_gradient, m_show_featureCurves);
					}

					GRenderingComponents::interfaceMeshManager().setVisibility(entity, m_show_featureCurves);
					GRenderingComponents::pickingManager().setPickable(entity, m_show_featureCurves);
				}
			}

			if (ImGui::MenuItem("Heightmap proxy meshes", NULL, m_show_heightmapInterfaceMeshes))
			{
				m_show_heightmapInterfaceMeshes = !m_show_heightmapInterfaceMeshes;

				auto heightmap_entities = GLandscapeComponents::landscapeManager().getListofHeightmapEntities();

				for (auto entity : heightmap_entities)
				{
					GRenderingComponents::interfaceMeshManager().setVisibility(entity, m_show_heightmapInterfaceMeshes);
					GRenderingComponents::pickingManager().setPickable(entity, m_show_heightmapInterfaceMeshes);
				}
			}

			if (ImGui::MenuItem("Feature meshes", NULL, m_show_featureMeshes))
			{
				m_show_featureMeshes = !m_show_featureMeshes;

				auto fm_entities = GLandscapeComponents::landscapeManager().getListofFeatureMeshEntities();

				for (auto entity : fm_entities)
				{
					GRenderingComponents::interfaceMeshManager().setVisibility(entity, m_show_featureMeshes);
					GRenderingComponents::pickingManager().setPickable(entity, m_show_featureMeshes);
				}
			}

			if (ImGui::MenuItem("Brick Bounding Boxes", NULL, m_show_landscapeBrickBBs))
			{
				m_show_landscapeBrickBBs = !m_show_landscapeBrickBBs;

				auto fm_entities = GLandscapeComponents::landscapeManager().getListofBrickEntities();

				for (auto entity : fm_entities)
				{
					GRenderingComponents::interfaceMeshManager().setVisibility(entity, m_show_landscapeBrickBBs);
					GRenderingComponents::pickingManager().setPickable(entity, m_show_landscapeBrickBBs);
				}
			}

			if (ImGui::MenuItem("Landscape Meshes", NULL, m_show_landscapeMeshes))
			{
				m_show_landscapeMeshes = !m_show_landscapeMeshes;

				auto fm_entities = GLandscapeComponents::landscapeManager().getListofBrickEntities();

				for (auto entity : fm_entities)
				{
					GRenderingComponents::staticMeshManager().setVisibility(entity, m_show_landscapeMeshes);
				}
			}

			if (ImGui::MenuItem("Pointlights", NULL, m_show_pointlights))
			{
				m_show_pointlights = !m_show_pointlights;

				auto pl_entities = GCoreComponents::pointlightManager().getListOfEntities();

				for (auto entity : pl_entities)
				{
					GRenderingComponents::interfaceMeshManager().setVisibility(entity, m_show_pointlights);
					GRenderingComponents::pickingManager().setPickable(entity, m_show_pointlights);
				}
			}

			if (ImGui::MenuItem("Decals", NULL, m_show_decals))
			{
				m_show_decals = !m_show_decals;

				auto decal_entities = GRenderingComponents::decalManager().getListOfEntities();

				for (auto entity : decal_entities)
				{
					GRenderingComponents::interfaceMeshManager().setVisibility(entity, m_show_decals);
					GRenderingComponents::pickingManager().setPickable(entity, m_show_decals);
				}
			}

			ImGui::EndMenu();
		}
	}

	void EditorUI::drawWindowMenu()
	{
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Component Properties", NULL, m_show_componentProperties))
			{
				m_show_componentProperties = !m_show_componentProperties;
			}
			if (ImGui::MenuItem("Outliner", NULL, m_show_outliner))
			{
				m_show_outliner = !m_show_outliner;
			}

			ImGui::EndMenu();
		}
	}

	void EditorUI::drawLandscapeMenu()
	{
		if (ImGui::BeginMenu("Landscape"))
		{

			if (ImGui::BeginMenu("New"))
			{
				ImGui::Text("Position");
				static float x, y, z = 0.0f;
				ImGui::InputFloat("##x", &x, 0.1f); ImGui::SameLine();
				ImGui::InputFloat("##y", &y, 0.1f); ImGui::SameLine();
				ImGui::InputFloat("##z", &z, 0.1f);

				ImGui::Text("Dimension");
				static float r = 128.0f, s = 64.0f, t = 128.0f;
				ImGui::InputFloat("##r", &r, 1.0f); ImGui::SameLine();
				ImGui::InputFloat("##s", &s, 1.0f); ImGui::SameLine();
				ImGui::InputFloat("##t", &t, 1.0f);

				ImGui::Text("Dimension");
				static int subdivX = 1, subdivY = 1, subdivZ = 1;
				ImGui::InputInt("##subdivX", &subdivX, 1); ImGui::SameLine();
				ImGui::InputInt("##subdivY", &subdivY, 1); ImGui::SameLine();
				ImGui::InputInt("##subdivZ", &subdivZ, 1);

				ImGui::Text("Brick resolution");
				static int resX = 128, resY = 64, resZ = 128;
				ImGui::InputInt("##resX", &resX, 2); ImGui::SameLine();
				ImGui::InputInt("##resY", &resY, 2); ImGui::SameLine();
				ImGui::InputInt("##resZ", &resZ, 2);

				if (ImGui::Button("Create"))
				{
					Entity new_landscape = GEngineCore::entityManager().create();
					GLandscapeComponents::landscapeManager().addComponent(new_landscape, Vec3(x, y, z), Vec3(r, s, t), subdivX, subdivY, subdivZ, resX, resY, resZ);
					GTools::selectManager().addComponent(new_landscape);
					GCoreComponents::debugNameManager().addComponent(new_landscape, "landscape");
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Import")) { m_show_landscapeImportPopup = true; }
			if (ImGui::MenuItem("Export")) { m_show_landscapeExportPopup = true; }
			if (ImGui::MenuItem("Export mesh")) { m_show_landscapeExportMeshPopup = true; }

			ImGui::Separator();

			if (ImGui::MenuItem("Import mesh")) { m_show_landscapeImportMeshPopup = true; }
			if (ImGui::MenuItem("Import heightmap")) { m_show_landscapeImportHeightMapPopup = true; }

			ImGui::Separator();

			if (ImGui::MenuItem("Add Feature Curve", "CTRL+F")) { Entity new_curve = GLandscapeComponents::landscapeManager().addFeatureCurve(true); }

			if (ImGui::BeginMenu("Feature Curves"))
			{
				ImGui::Text("Ribbon width");
				static float rb_width = 1.0f;
				if (ImGui::InputFloat("##rb_width", &rb_width, 0.1f))
					GLandscapeComponents::featureCurveManager().setRibbonWidth(rb_width);
				ImGui::EndMenu();
			}


			ImGui::Separator();

			if (ImGui::MenuItem("Update Shader Programs", "u")) { GLandscapeComponents::brickManager().updateShaderPrograms(); }

			ImGui::Separator();

			if (ImGui::MenuItem("Realtime Update Enabled", NULL, m_landscape_realtimeUpdate))
			{
				m_landscape_realtimeUpdate = !m_landscape_realtimeUpdate;

				GTools::landscapeTool().setRealtimeUpdate(m_landscape_realtimeUpdate);
			}

			if (ImGui::MenuItem("Update", "u")) { GLandscapeComponents::landscapeManager().updateAll(); }

			if (ImGui::MenuItem("Debug texture baking")) { GLandscapeComponents::brickManager().updateTextureBaking(0); }

			ImGui::Separator();

			if (ImGui::BeginMenu("Voxelization"))
			{
				if (ImGui::MenuItem("Feature Curves", NULL, m_landscape_voxFeatureCurves))
				{
					m_landscape_voxFeatureCurves = !m_landscape_voxFeatureCurves;

					GLandscapeComponents::brickManager().setVoxelizeFeatureCurves(m_landscape_voxFeatureCurves);
				}

				if (ImGui::MenuItem("Feature Meshes", NULL, m_landscape_voxFeatureMeshes))
				{
					m_landscape_voxFeatureMeshes = !m_landscape_voxFeatureMeshes;

					GLandscapeComponents::brickManager().setVoxelizeFeatureMeshes(m_landscape_voxFeatureMeshes);
				}

				if (ImGui::MenuItem("Heightmaps", NULL, m_landscape_voxHeightmaps))
				{
					m_landscape_voxHeightmaps = !m_landscape_voxHeightmaps;

					GLandscapeComponents::brickManager().setVoxelizeHeightmaps(m_landscape_voxHeightmaps);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
	}

	void EditorUI::drawOutliner()
	{
		ImGuiWindowFlags window_flags = 0;
		bool p_open = true;
		if (!ImGui::Begin("Outliner", &p_open, window_flags))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// list all selectable entities, display debug name if available
		auto entities = GTools::selectManager().getAllEntities();
		uint entity_cnt = entities.size();

		ImGui::Columns(2, "mycolumns"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("Entity ID"); ImGui::NextColumn();
		ImGui::Text("Debug Name"); ImGui::NextColumn();
		ImGui::Separator();
		std::vector<Entity> add_to_selection;

		for (auto entity : entities)
		{
			if (ImGui::Selectable(std::to_string(entity.id()).c_str(), GTools::selectManager().isSelected(entity), ImGuiSelectableFlags_SpanAllColumns))
			{
				add_to_selection.push_back(entity);
			}
			ImGui::NextColumn();
			//ImGui::Text( std::to_string(query.second.id()).c_str() ); ImGui::NextColumn();
			std::string debug_name = GCoreComponents::debugNameManager().getDebugName(entity);
			ImGui::Text(debug_name.c_str() );
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		// TODO select new selection in SelectManager
		if (ImGui::IsKeyDown(341))
		{
			for (auto entity : add_to_selection)
				GTools::selectManager().addToSelection(entity.id());
		}
		else
		{
			if(!add_to_selection.empty())
				GTools::selectManager().select(add_to_selection.back().id());
		}

		ImGui::End();
	}

	void EditorUI::drawComponentPropertyWindow()
	{
		ImGuiWindowFlags window_flags = 0;
		bool p_open = true;
		if (!ImGui::Begin("Component Properties", &p_open, window_flags))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		if (last_selection_eID == -1)
		{
			ImGui::End();
			return;
		}

		// for last selected entity, check for all components belonging to it

		if (GCoreComponents::transformManager().checkComponent(last_selection_eID))
			drawTransformComponentProperties();

		if (GCoreComponents::cameraManager().checkComponent(last_selection_eID))
			drawCameraComponentProperties();

		drawPointlightComponentProperties();

		drawSunlightComponentProperties();

		drawLandscapeComponentProperties();

		drawDecalComponentProperties();

		drawRearSteerBicycleComponentProperties();

		ImGui::End();
	}

	void EditorUI::drawAirplanePhysicsComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		auto query = GCoreComponents::airplanePhysicsManager().getIndex(last_selection_eID);

		if (query.first)
		{
			if (ImGui::CollapsingHeader("Airplane Physics Component"))
			{
				uint transform_idx = GCoreComponents::transformManager().getIndex(last_selection_eID);
				Quat orientation = GCoreComponents::transformManager().getOrientation(transform_idx);

				Quat qFront = glm::cross(glm::cross(orientation, glm::quat(0.0, 0.0, 0.0, 1.0)), glm::conjugate(orientation));
				Quat qUp = glm::cross(glm::cross(orientation, glm::quat(0.0, 0.0, 1.0, 0.0)), glm::conjugate(orientation));
				Quat qRighthand = glm::cross(glm::cross(orientation, glm::quat(0.0, -1.0, 0.0, 0.0)), glm::conjugate(orientation));

				Vec3 vFront(qFront.x, qFront.y, qFront.z);
				Vec3 vUp(qUp.x, qUp.y, qUp.z);
				Vec3 vRighthand(qRighthand.x, qRighthand.y, qRighthand.z);

				Vec3 velocity = GCoreComponents::airplanePhysicsManager().getVelocity(query.second);
				float aoa = GCoreComponents::airplanePhysicsManager().getAngleOfAttack(query.second);
				float aos = GCoreComponents::airplanePhysicsManager().getAngleOfSideslip(query.second);
				float lc = GCoreComponents::airplanePhysicsManager().getLiftCoefficient(query.second);
				float dc = GCoreComponents::airplanePhysicsManager().getDragCoefficient(query.second);
				float et = GCoreComponents::airplanePhysicsManager().getEngineThrust(query.second);
				float al = GCoreComponents::airplanePhysicsManager().getAerodynamicLift(query.second);
				float aa = GCoreComponents::airplanePhysicsManager().getAileronAngle(query.second);
				float ra = GCoreComponents::airplanePhysicsManager().getRudderAngle(query.second);
				float ea = GCoreComponents::airplanePhysicsManager().getElevatorAngle(query.second);
				float ws = GCoreComponents::airplanePhysicsManager().getWingSurface(query.second);

				ImGui::Text("Up: %.3f , %.3f , %.3f", vUp.x, vUp.y, vUp.z);
				ImGui::Text("Front: %.3f , %.3f , %.3f", vFront.x, vFront.y, vFront.z);
				ImGui::Text("Righthand: %.3f , %.3f , %.3f", vRighthand.x, vRighthand.y, vRighthand.z);
				ImGui::Text("Velocity: %.3f , %.3f , %.3f", velocity.x, velocity.y, velocity.z);
				ImGui::Text("Airspeed.: %.3f", glm::length(velocity) * 3.6f);
				ImGui::Text("Angle of attack: %.3f", aoa);
				ImGui::Text("Angle of sideslip: %.3f", aos);
				ImGui::Text("Lift coefficient: %.3f", lc);
				ImGui::Text("Drag coefficient: %.3f", dc);
				ImGui::Text("Engine thrust: %.3f", et);
				ImGui::Text("Lift force: %.3f", al);

				ImGui::Text("Aileron angle: %.3f", aa);
				ImGui::Text("Rudder angle: %.3f", ra);
				ImGui::Text("Elevator angle: %.3f", ea);

				ImGui::Text("Wing surface %.3f", ws);
			}
		}

	}

	void EditorUI::drawCameraComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		if (ImGui::CollapsingHeader("Camera Component"))
		{
			uint idx = GCoreComponents::cameraManager().getIndex(last_selection_eID);

			float fovy = GCoreComponents::cameraManager().getFovy(idx);
			float exposure = GCoreComponents::cameraManager().getExposure(idx);

			ImGui::SliderFloat("Fovy", &fovy, 0.0f, 1.552f);
			ImGui::SliderFloat("Exposure", &exposure, 0.000018f, 0.0018f, "%.6f");

			GCoreComponents::cameraManager().setFovy(idx,fovy);
			GCoreComponents::cameraManager().setExposure(idx,exposure);
			GCoreComponents::cameraManager().updateProjectionMatrix(idx);
		}
	}

	void EditorUI::drawLandscapeComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		auto query = GLandscapeComponents::landscapeManager().getIndex(last_selection_eID);

		if (query.first)
		{
			if (ImGui::CollapsingHeader("Landscape Component"))
			{
				//TODO brick selection
				int brick_cnt = 0;
				std::vector<const char*> brick_char_list;

				auto bricks = GLandscapeComponents::landscapeManager().getBrickList(query.second);
				brick_cnt = bricks.size();

				std::vector<std::string> brick_list(brick_cnt);
				for (uint i = 0; i < brick_cnt; i++)
				{
					brick_list[i] = "Brick_" + std::to_string(i);
					brick_char_list.push_back(brick_list[i].c_str());
				}
				
				static int brick_item = 0;
				ImGui::Combo("Brick", &brick_item, brick_char_list.data(), brick_cnt);

				ImGui::Separator();

				
				//TODO show information
				const char* debug_field_items[] = { "none", "normal", "gradient", "noise", "surface", "surface_boundary" };
				static int debug_field_item = 0;
				ImGui::Combo("Debug field data", &debug_field_item, debug_field_items, 6);

				Entity brick_entity = bricks[brick_item];
				uint brick_idx = GLandscapeComponents::brickManager().getIndex(brick_entity);

				
				switch (debug_field_item)
				{
				case 0:
					GRenderingComponents::volumeManager().setVisibility(brick_entity, false);
					break;
				case 1:
					GRenderingComponents::volumeManager().setVisibility(brick_entity, true);
					GLandscapeComponents::brickManager().setDebugField(brick_idx, Landscape::LandscapeBrickComponentManager::Datafield::NORMAL);
					break;
				case 2:
					GRenderingComponents::volumeManager().setVisibility(brick_entity, true);
					GLandscapeComponents::brickManager().setDebugField(brick_idx, Landscape::LandscapeBrickComponentManager::Datafield::GRADIENT);
					break;
				case 3:
					GRenderingComponents::volumeManager().setVisibility(brick_entity, true);
					GLandscapeComponents::brickManager().setDebugField(brick_idx, Landscape::LandscapeBrickComponentManager::Datafield::NOISE);
					break;
				case 4:
					GRenderingComponents::volumeManager().setVisibility(brick_entity, true);
					GLandscapeComponents::brickManager().setDebugField(brick_idx, Landscape::LandscapeBrickComponentManager::Datafield::SURFACE);
					break;
				case 5:
					GRenderingComponents::volumeManager().setVisibility(brick_entity, true);
					GLandscapeComponents::brickManager().setDebugField(brick_idx, Landscape::LandscapeBrickComponentManager::Datafield::SURFACE_BOUNDARY);
					break;
				default:
					break;
				}


				/*
				//select surface material

				const char* material_items[] = { "coastline", "desert", "snow" };
				static int material_item = 0;
				bool update = ImGui::Combo("Surface Material", &material_item, material_items, 3);

				//TODO improve getting material from resource manager

				MaterialInfo surface_material_info;
				Material* mat;

				if (update)
				{
					switch (material_item)
					{
					case 0:
						surface_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_coastline.slmtl");
						mat = GEngineCore::resourceManager().createMaterial("../resources/materials/landscape_coastline.slmtl", surface_material_info.shader_filepaths, surface_material_info.texture_filepaths).second;
						GLandscapeComponents::brickManager().setSurfaceMaterial(brick_item, mat);
						break;
					case 1:
						surface_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_rocky_desert.slmtl");
						mat = GEngineCore::resourceManager().createMaterial("../resources/materials/landscape_rocky_desert.slmtl", surface_material_info.shader_filepaths, surface_material_info.texture_filepaths).second;
						GLandscapeComponents::brickManager().setSurfaceMaterial(brick_item, mat);
						break;
					case 2:
						surface_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_snow_mountain.slmtl");
						mat = GEngineCore::resourceManager().createMaterial("../resources/materials/landscape_snow_mountain.slmtl", surface_material_info.shader_filepaths, surface_material_info.texture_filepaths).second;
						GLandscapeComponents::brickManager().setSurfaceMaterial(brick_item, mat);
						break;
					default:
						break;
					}
				}
				*/
			}
		}
	}

	void EditorUI::drawTransformComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		if (ImGui::CollapsingHeader("Transform Component"))
		{
			uint idx = GCoreComponents::transformManager().getIndex(last_selection_eID);
			Vec3 world_pos = GCoreComponents::transformManager().getWorldPosition(idx);
			Quat orientation = GCoreComponents::transformManager().getOrientation(idx);

			// Query current transform state

			ImGui::Text("Position");
			ImGui::Text("X"); ImGui::SameLine(); ImGui::InputFloat("##x", &world_pos.x, 0.1f);
			ImGui::Text("Y"); ImGui::SameLine(); ImGui::InputFloat("##y", &world_pos.y, 0.1f);
			ImGui::Text("Z"); ImGui::SameLine(); ImGui::InputFloat("##z", &world_pos.z, 0.1f);

			ImGui::Separator();

			ImGui::Text("Scale");
			static float sx, sy, sz = 0.0f;
			ImGui::Text("X"); ImGui::SameLine(); ImGui::InputFloat("##sx", &sx, 0.1f);
			ImGui::Text("Y"); ImGui::SameLine(); ImGui::InputFloat("##sy", &sy, 0.1f);
			ImGui::Text("Z"); ImGui::SameLine(); ImGui::InputFloat("##sz", &sz, 0.1f);

			ImGui::Separator();

			ImGui::Text("Orientation");
			ImGui::Text("X"); ImGui::SameLine(); ImGui::InputFloat("##ox", &orientation.x, 0.1f);
			ImGui::Text("Y"); ImGui::SameLine(); ImGui::InputFloat("##oy", &orientation.y, 0.1f);
			ImGui::Text("Z"); ImGui::SameLine(); ImGui::InputFloat("##oz", &orientation.z, 0.1f);
			ImGui::Text("Z"); ImGui::SameLine(); ImGui::InputFloat("##ow", &orientation.w, 0.1f);


			//GCoreComponents::transformManager().setPosition(idx, world_pos);
			//TODO scale
			//GCoreComponents::transformManager().setOrientation(idx, orientation);
		}
	}

	void EditorUI::drawOceanComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		auto query = GRenderingComponents::oceanManager().getIndex(last_selection_eID);

		if (query.first)
		{
			if (ImGui::CollapsingHeader("Ocean Component"))
			{
				/*
				const char* items[] = { "gaussian noise", "h0_k", "h0_-k", "h_k", "displacement", "twiddle" };
				static int item2 = 1;
				ImGui::Combo("", &item2, items, 6);

				switch (item2)
				{
				case 0:
					if (ocean_data.gaussian_noise != nullptr) ImGui::Image(reinterpret_cast<ImTextureID>(ocean_data.gaussian_noise->getHandle()), ImVec2(static_cast<float>(ocean_data.grid_size), static_cast<float>(ocean_data.grid_size)));
					break;
				case 1:
					if (ocean_data.tilde_h0_of_k != nullptr) ImGui::Image(reinterpret_cast<ImTextureID>(ocean_data.tilde_h0_of_k->getHandle()), ImVec2(static_cast<float>(ocean_data.grid_size), static_cast<float>(ocean_data.grid_size)));
					break;
				case 2:
					if (ocean_data.tilde_h0_of_minus_k != nullptr) ImGui::Image(reinterpret_cast<ImTextureID>(ocean_data.tilde_h0_of_minus_k->getHandle()), ImVec2(static_cast<float>(ocean_data.grid_size), static_cast<float>(ocean_data.grid_size)));
					break;
				case 3:
					if (ocean_data.spectrum_y_displacement != nullptr) ImGui::Image(reinterpret_cast<ImTextureID>(ocean_data.spectrum_y_displacement->getHandle()), ImVec2(static_cast<float>(ocean_data.grid_size), static_cast<float>(ocean_data.grid_size)));
					break;
				case 4:
					if (ocean_data.displacement != nullptr) ImGui::Image(ImTextureID(ocean_data.displacement->getHandle()), ImVec2(ocean_data.grid_size, ocean_data.grid_size));
					break;
				case 5:
					if (ocean_data.twiddle != nullptr) ImGui::Image(ImTextureID(ocean_data.twiddle->getHandle()), ImVec2(std::log2(ocean_data.grid_size), static_cast<float>(ocean_data.grid_size)));
					break;
				default:
					break;
				}
				*/
			}
		}
	}

	void EditorUI::drawPointlightComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		auto query = GCoreComponents::pointlightManager().getIndex(last_selection_eID);

		if (query.first)
		{
			if (ImGui::CollapsingHeader("Pointlight Component"))
			{
				Vec3 colour = GCoreComponents::pointlightManager().getColour(query.second);
				float lumen = GCoreComponents::pointlightManager().getLumen(query.second);
				float radius = GCoreComponents::pointlightManager().getRadius(query.second);

				ImVec4 im_color(colour.x, colour.y, colour.z, 1.0);

				ImGui::Text("Colour"); ImGui::SameLine(); ImGui::ColorButton(im_color);
				ImGui::Text("Lumen"); ImGui::SameLine(); ImGui::InputFloat("##lumen", &lumen, 0.1f);
				ImGui::Text("Radius"); ImGui::SameLine(); ImGui::InputFloat("##radius", &radius, 0.1f);

				GCoreComponents::pointlightManager().setColour(query.second, colour);
				GCoreComponents::pointlightManager().setLumen(query.second, lumen);
				GCoreComponents::pointlightManager().setRadius(query.second, radius);
			}
		}
	}

	void EditorUI::drawSunlightComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		auto query = GCoreComponents::sunlightManager().getIndex(last_selection_eID);

		if (query.first)
		{
			if (ImGui::CollapsingHeader("Sunlight Component"))
			{
				Vec3 colour = GCoreComponents::sunlightManager().getColour(query.second);
				float lumen = GCoreComponents::sunlightManager().getLumen(query.second);
				float radius = GCoreComponents::sunlightManager().getStarRadius(query.second);

				ImVec4 im_color(colour.x, colour.y, colour.z, 1.0);

				ImGui::ColorEdit4("Colour", (float*)&im_color);
				//ImGui::Text("Colour"); ImGui::SameLine(); ImGui::ColorButton(im_color);
				ImGui::Text("Lumen"); ImGui::SameLine(); ImGui::InputFloat("##star_lumen", &lumen, 0.1f);
				ImGui::Text("(Star)Radius"); ImGui::SameLine(); ImGui::InputFloat("##star_radius", &radius, 0.1f);

				GCoreComponents::sunlightManager().setColour(query.second, Vec3(im_color.x, im_color.y, im_color.z));
				GCoreComponents::sunlightManager().setLumen(query.second, lumen);
				GCoreComponents::sunlightManager().setStarRadius(query.second, radius);

				ImGui::Separator();

				static float alpha;
				static float beta;

				ImGui::SliderFloat("Alpha", &alpha, -1.552f, 1.552f);
				ImGui::SliderFloat("Beta", &beta, 0.0f, 6.282f);

				float lat_sin = sin(alpha);
				float lon_sin = sin(beta);

				float lat_cos = cos(alpha);
				float lon_cos = cos(beta);

				float d = 149600000000.0f;

				Vec3 sun_position;
				sun_position.x = (lon_sin * lat_cos * d);
				sun_position.y = (lat_sin * d);
				sun_position.z = (lat_cos * lon_cos * d);

				GCoreComponents::transformManager().setPosition(GCoreComponents::sunlightManager().getEntity(query.second), sun_position);
			}
		}
	}

	void EditorUI::drawDecalComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		auto query = GRenderingComponents::decalManager().getIndex(last_selection_eID);

		if (query.first)
		{
			if (ImGui::CollapsingHeader("Decal Component"))
			{
				std::string material_path = GRenderingComponents::decalManager().getMaterialName(last_selection_eID);

				char current_material_path[200];
				
#pragma warning(disable : 4996)
				std::strcpy(current_material_path, material_path.c_str());

				if (ImGui::InputText("##current_material_path", current_material_path, IM_ARRAYSIZE(current_material_path), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					GRenderingComponents::decalManager().updateComponent(last_selection_eID, current_material_path);
				}

				if (ImGui::Button("Update", ImVec2(120, 0)))
				{
					GRenderingComponents::decalManager().updateComponent(last_selection_eID, current_material_path);
				}

				std::pair<float, float> decal_size = GRenderingComponents::decalManager().getDecalSize(last_selection_eID);

				ImGui::Text("Width"); ImGui::SameLine();
				if (ImGui::InputFloat("##decal_width", &decal_size.first, 0.1f))
					GRenderingComponents::decalManager().updateComponent(last_selection_eID, decal_size.first, decal_size.second);


				ImGui::Text("Height"); ImGui::SameLine();
				if(ImGui::InputFloat("##decal_height", &decal_size.second, 0.1f))
					GRenderingComponents::decalManager().updateComponent(last_selection_eID, decal_size.first, decal_size.second);
			}
		}
	}

	void EditorUI::drawRearSteerBicycleComponentProperties()
	{
		long last_selection_eID = GTools::selectManager().getLatestSelectionId();

		auto query = GCoreComponents::rearSteerBicycleManager().getIndex(last_selection_eID);

		if (query.first)
		{
			if (ImGui::CollapsingHeader("RearSteerBicycle"))
			{
				SimulationState state;

				state = GCoreComponents::rearSteerBicycleManager().computeCurrentState(query.second,0.0f);

				ImGui::Text("x_e"); ImGui::SameLine(); ImGui::InputFloat("##x_e", &state.x_e, 0.1f);
				ImGui::Text("y_e"); ImGui::SameLine(); ImGui::InputFloat("##y_e", &state.y_e, 0.1f);
				ImGui::Text("phi"); ImGui::SameLine(); ImGui::InputFloat("##phi", &state.phi, 0.1f);
				ImGui::Text("phi_dt"); ImGui::SameLine(); ImGui::InputFloat("##phi_dt", &state.phi_dt, 0.1f);
				ImGui::Text("delta"); ImGui::SameLine(); ImGui::InputFloat("##delta", &state.delta, 0.1f);
				ImGui::Text("delta_dt"); ImGui::SameLine(); ImGui::InputFloat("##delta_dt", &state.delta_dt, 0.1f);
				ImGui::Text("psi"); ImGui::SameLine(); ImGui::InputFloat("##psi", &state.psi, 0.1f);
				ImGui::Text("omega"); ImGui::SameLine(); ImGui::InputFloat("##omega", &state.omega, 0.1f);

				//GCoreComponents::rearSteerBicycleManager().pushSimulationState(query.second, state);

				if (ImGui::Button("Attach Camera"))
				{
					Entity bike = GCoreComponents::rearSteerBicycleManager().getEntity(query.second);
					Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
					uint idx = GCoreComponents::transformManager().getIndex(active_camera);
					auto cam_pos = GCoreComponents::transformManager().getPosition(idx);
					auto bike_pos = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(bike));
					auto d = cam_pos - bike_pos;
					GCoreComponents::transformManager().setPosition(idx, d);
					GCoreComponents::transformManager().setParent(idx, bike);

				}

				if (ImGui::Button("Detach Camera"))
				{
					Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
					uint idx = GCoreComponents::transformManager().getIndex(active_camera);
					GCoreComponents::transformManager().setParent(idx, active_camera);
				}
			}
		}
	}
}