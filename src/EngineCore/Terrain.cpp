#include "Terrain.hpp"

#include <array>
#include <iterator>
#include <list>
#include <numeric>

#include "Frame.hpp"
#include "TaskSchedueler.hpp"

#include "GlobalRenderingComponents.hpp"
#include "StaticMeshComponent.hpp"
#include "VolumeComponent.hpp"
#include "PtexMeshComponent.hpp"
#include "DecalComponent.hpp"

#include "EditorUI.hpp"
#include "EditorSelectComponent.hpp"

#include "MarchingCubesTriangleTable.hpp"
#include "ResourceLoading.hpp"

namespace Landscape
{
	uint FeatureMeshComponentManager::getIndex(Entity entity) const
	{
		auto search = m_index_map.find(entity.id());

		assert((search != m_index_map.end()));

		return search->second;
	}

	void FeatureMeshComponentManager::addComponent(Entity entity, std::string mesh_path)
	{
		uint idx = static_cast<uint>(m_data.size());

		m_index_map.insert(std::pair<uint, uint>(entity.id(), idx));

		m_data.push_back(Data(entity, mesh_path));

		std::vector<uint8_t> temp_vertex_databuffer;

		ResourceLoading::loadFbxGeometry(mesh_path, temp_vertex_databuffer, m_data.back().m_index_data, m_data.back().m_vertex_layout);

		size_t src_vertex_byteSize = static_cast<size_t>(m_data.back().m_vertex_layout.byte_size);
		m_data.back().m_vertex_data.resize((temp_vertex_databuffer.size() / src_vertex_byteSize) * 12); //resize to #vertices * 3floats(12byte)
		m_data.back().m_vertex_layout = VertexLayout(12, {VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0)}); //set actual vertex layout
		m_data.back().m_lower_corner = Vec3(std::numeric_limits<float>::max());
		m_data.back().m_upper_corner = Vec3(std::numeric_limits<float>::min());

		// Extract position information only
		size_t tgt_byte_counter = 0;
		for (size_t src_byte_counter = 0; src_byte_counter < temp_vertex_databuffer.size(); src_byte_counter += src_vertex_byteSize)
		{
			// Assumes the first three floats are x,y,z position

			// Update extends
			float x = *((float*)(temp_vertex_databuffer.data() + src_byte_counter));
			float y = *((float*)(temp_vertex_databuffer.data() + src_byte_counter + 4));
			float z = *((float*)(temp_vertex_databuffer.data() + src_byte_counter + 8));

			// Update extends
			m_data.back().m_lower_corner.x = std::min(m_data.back().m_lower_corner.x, x);
			m_data.back().m_lower_corner.y = std::min(m_data.back().m_lower_corner.y, y);
			m_data.back().m_lower_corner.z = std::min(m_data.back().m_lower_corner.z, z);

			m_data.back().m_upper_corner.x = std::max(m_data.back().m_upper_corner.x, x);
			m_data.back().m_upper_corner.y = std::max(m_data.back().m_upper_corner.y, y);
			m_data.back().m_upper_corner.z = std::max(m_data.back().m_upper_corner.z, z);

			// Copy data
			std::copy(temp_vertex_databuffer.data() + src_byte_counter, temp_vertex_databuffer.data() + src_byte_counter + 12, m_data.back().m_vertex_data.data() + tgt_byte_counter);
			tgt_byte_counter += 12;
		}

		m_data.back().m_mesh = GEngineCore::resourceManager().createMesh(m_data.back().m_mesh_path,
																			m_data.back().m_vertex_data,
																			m_data.back().m_index_data,
																			m_data.back().m_vertex_layout,
																			GL_TRIANGLES).resource;

		// add interface mesh....TODO maybe use a different material
		GRenderingComponents::interfaceMeshManager().addComponent(entity, "../resources/materials/editor/interface_curve.slmtl", m_data.back().m_mesh);

		// add select component
		GRenderingComponents::pickingManager().addComponent<std::vector<uint8_t>, std::vector<uint32_t>>(entity,
			"feature_mesh_" + std::to_string(idx) + "_selectProxy",
			"../resources/materials/editor/picking.slmtl",
			m_data.back().m_vertex_data,
			m_data.back().m_index_data,
			m_data.back().m_vertex_layout,
			GL_TRIANGLES);
		GTools::selectManager().addComponent(entity,
			std::bind(&Editor::LandscapeTools::activateFeatureMeshTool, &GTools::landscapeTool(), entity),
			std::bind(&Editor::LandscapeTools::deactivateFeatureMeshTool, &GTools::landscapeTool()));
	}

	Vec3 FeatureMeshComponentManager::getLowerCorner(uint index) const
	{
		assert(m_data.size() > index);

		return m_data[index].m_lower_corner;
	}

	Vec3 FeatureMeshComponentManager::getUpperCorner(uint index) const
	{
		assert(m_data.size() > index);

		return m_data[index].m_upper_corner;
	}

	Mesh const * const FeatureMeshComponentManager::getMesh(uint index) const
	{
		assert(m_data.size() > index);

		return m_data[index].m_mesh;
	}


	void FeatureCurveComponentManager::addComponent(Entity entity, Vec3 position, Quat orientation, bool is_surface_seed)
	{
		std::unique_lock<std::mutex>(m_dataAccess_mutex);

		m_featureCurves.push_back(FeatureCurveComponent(entity));
		GCoreComponents::transformManager().addComponent(entity,position,orientation);
		uint idx = static_cast<uint>(m_featureCurves.size() -1);
		m_curve_index_map.insert(std::pair<uint,uint>(entity.id(),idx));

		m_featureCurves[idx].m_is_surface_seed = is_surface_seed;

		// Build intial knot vector
		m_featureCurves.back().m_knots.push_back(0.0f);
		m_featureCurves.back().m_knots.push_back(0.0f);
		
		m_featureCurves.back().m_knots.push_back(1.0f);
		m_featureCurves.back().m_knots.push_back(1.0f);

		// Add intial control vertices
		addControlVertex(idx, Vec3(-1.0,0.0,0.0));
		addControlVertex(idx, Vec3(1.0,0.0,0.0));

		// Set AABB corners
		m_featureCurves.back().m_lower_corner = Vec3(-1.0,0.0,0.0);
		m_featureCurves.back().m_upper_corner = Vec3(1.0,0.0,0.0);

		m_featureCurves.back().m_ribbon_width = 1.0f;

		// Add inital constraint points at the beginning and end of the curve
		addConstraintPoint(idx, 0.0f, Vec3(0.0, 0.0, -1.0), Vec3(0.0, -1.0, 0.0));
		addConstraintPoint(idx, 1.0f, Vec3(0.0, 0.0, -1.0), Vec3(0.0, -1.0, 0.0));

		// Build intial mesh data
		recomputeCurveMesh(idx);

		// Create texture array for storing feature curve texture decals //TODO replaced by decal component
		TextureLayout layout;
		layout.internal_format = GL_RGBA8;
		layout.format = GL_RGBA;
		layout.type = GL_UNSIGNED_BYTE;
		layout.width = 256;
		layout.height = 256;
		layout.depth = 100.0;
		layout.levels = 1;

		layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
		layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
		layout.int_parameters.push_back({ GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER });
		layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
		layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
		m_featureCurves.back().m_segement_textures = GEngineCore::resourceManager().createTexture2DArrayAsync("feature_curve_eID_" + std::to_string(entity.id()) + "_segment_textures", layout, nullptr);


#if EDITOR_MODE // preprocessor definition

		VertexLayout vertex_description(60, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
													VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 3),
													VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 6),
													VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 9),
													VertexLayout::Attribute(GL_FLOAT,2,GL_FALSE,sizeof(GLfloat) * 12),
													VertexLayout::Attribute(GL_FLOAT,1,GL_FALSE,sizeof(GLfloat) * 14) });

		GRenderingComponents::interfaceMeshManager().addComponent<std::vector<float>,std::vector<uint>>(entity,
											"feature_curve"+std::to_string(idx),
											"../resources/materials/editor/interface_curve.slmtl",
											m_featureCurves.back().m_mesh_vertices,
											m_featureCurves.back().m_mesh_indices,
											vertex_description,
											GL_TRIANGLES);

		// add selectable component
		GRenderingComponents::pickingManager().addComponent<std::vector<float>, std::vector<uint>>(entity,
			"feature_curve_" + std::to_string(idx) + "_selectProxy",
			"../resources/materials/editor/picking.slmtl",
			m_featureCurves.back().m_mesh_vertices,
			m_featureCurves.back().m_mesh_indices,
			vertex_description,
			GL_TRIANGLES);
		GTools::selectManager().addComponent(entity, std::bind(&Editor::LandscapeTools::activateFeatureCurveTools, &GTools::landscapeTool(), entity),
			std::bind(&Editor::LandscapeTools::deactivateFeatureCurveTools, &GTools::landscapeTool()));
#endif
	}

	void FeatureCurveComponentManager::deleteComponent(uint index)
	{
	}
	
	uint FeatureCurveComponentManager::getIndex(Entity entity) const
	{
		auto search = m_curve_index_map.find(entity.id());

		assert( (search != m_curve_index_map.end()) );

		return search->second;
	}

	FeatureCurveComponentManager::FeatureCurveComponent& FeatureCurveComponentManager::getCurve(Entity entity)
	{
		auto search = m_curve_index_map.find(entity.id());

		assert(search != m_curve_index_map.end());

		return m_featureCurves[search->second];
	}

	Entity FeatureCurveComponentManager::getCurveFromControlVertex(Entity cv_entity)
	{
		auto search = m_cv_index_map.find(cv_entity.id());

		assert(search != m_cv_index_map.end());

		return m_featureCurves[search->second].m_entity;
	}

	Entity FeatureCurveComponentManager::getCurveFromConstraintPoint(Entity cp_entity)
	{
		auto search = m_cp_index_map.find(cp_entity.id());

		assert(search != m_cp_index_map.end());

		return m_featureCurves[search->second].m_entity;
	}

	Entity FeatureCurveComponentManager::getConstraintPointFromGradient(Entity gradient)
	{
		auto search = m_gradient_cp_entity_map.find(gradient.id());

		assert(search != m_gradient_cp_entity_map.end());

		Entity rtn = search->second;

		return rtn;
	}

	void FeatureCurveComponentManager::addControlVertex(uint index, Vec3 position)
	{
		Entity cv_entity = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(cv_entity, position, Quat(), Vec3(1.0));
		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(cv_entity), m_featureCurves[index].m_entity);

		m_featureCurves[index].m_controlVertices.push_back(FeatureCurveComponent::ControlVertex(cv_entity));

		// Compute world space position of new point
		Vec3 ws_position = GCoreComponents::transformManager().getWorldPosition(cv_entity);

		// Update extends
		m_featureCurves[index].m_lower_corner.x = std::min(m_featureCurves[index].m_lower_corner.x, ws_position.x);
		m_featureCurves[index].m_lower_corner.y = std::min(m_featureCurves[index].m_lower_corner.y, ws_position.y);
		m_featureCurves[index].m_lower_corner.z = std::min(m_featureCurves[index].m_lower_corner.z, ws_position.z);

		m_featureCurves[index].m_upper_corner.x = std::max(m_featureCurves[index].m_upper_corner.x, ws_position.x);
		m_featureCurves[index].m_upper_corner.y = std::max(m_featureCurves[index].m_upper_corner.y, ws_position.y);
		m_featureCurves[index].m_upper_corner.z = std::max(m_featureCurves[index].m_upper_corner.z, ws_position.z);


		// Adjust knot vector and degree
		if (m_featureCurves[index].m_controlVertices.size() > 2 && m_featureCurves[index].m_controlVertices.size() < 5)
		{
			size_t idx = m_featureCurves[index].m_controlVertices.size() - 1;
			m_featureCurves[index].m_knots[idx]--;

			float end_value = m_featureCurves[index].m_knots.back();
			m_featureCurves[index].m_knots.push_back(end_value);
			m_featureCurves[index].m_knots.push_back(end_value);

			m_featureCurves[index].degree++;

		}
		else if (m_featureCurves[index].m_controlVertices.size() > 4)
		{
			size_t start_index = m_featureCurves[index].m_controlVertices.size();
			size_t end_index = m_featureCurves[index].m_knots.size();

			for (size_t i = start_index; i < end_index; i++)
			{
				m_featureCurves[index].m_knots[i]++;
			}

			float end_value = m_featureCurves[index].m_knots.back();
			m_featureCurves[index].m_knots.push_back(end_value);
		}

		m_cv_index_map.insert({ cv_entity.id(),index });

		if (m_featureCurves[index].m_controlVertices.size() > 2)
			updateCurve(index);

#if EDITOR_MODE // preprocessor definition

		std::vector<float> cv_interface_vertices({ -0.1f,-0.1f,0.1f,0.0f,1.0f,0.0f,1.0f,
													0.1f,-0.1f,0.1f,0.0f,1.0f,0.0f,1.0f,
													0.1f,-0.1f,-0.1f,0.0f,1.0f,0.0f,1.0f,
													-0.1f,-0.1f,-0.1f,0.0f,1.0f,0.0f,1.0f,
													-0.1f,0.1f,0.1f,0.0f,1.0f,0.0f,1.0f,
													0.1f,0.1f,0.1f,0.0f,1.0f,0.0f,1.0f,
													0.1f,0.1f,-0.1f,0.0f,1.0f,0.0f,1.0f,
													-0.1f,0.1f,-0.1f,0.0f,1.0f,0.0f,1.0 });
		std::vector<uint32_t> cv_interface_indices({ 0,1, 0,3, 0,4, 1,2, 1,5, 2,3, 2,6, 3,7, 4,5, 4,7, 5,6, 6,7 });

		VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
											VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });

		GRenderingComponents::interfaceMeshManager().addComponent(cv_entity, "cv", "../resources/materials/editor/interface_cv.slmtl", cv_interface_vertices, cv_interface_indices, vertex_description, GL_LINES);

		std::vector<float> cv_selectProxy_vertices({ -0.1f,-0.1f,0.1f,
													0.1f,-0.1f,0.1f,
													0.1f,-0.1f,-0.1f,
													-0.1f,-0.1f,-0.1f,
													-0.1f,0.1f,0.1f,
													0.1f,0.1f,0.1f,
													0.1f,0.1f,-0.1f,
													-0.1f,0.1f,-0.1f });
		std::vector<uint32_t> cv_selectProxy_indices({ 0,2,1, 0,3,2, 4,5,6, 4,6,7, 0,1,5, 0,5,4, 3,6,2, 3,7,6, 3,0,4, 3,4,7, 1,2,6, 1,6,5 });
		VertexLayout selectProxy_vd(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });
		GRenderingComponents::pickingManager().addComponent(cv_entity, "cv_selectProxy", "../resources/materials/editor/picking.slmtl", cv_selectProxy_vertices, cv_selectProxy_indices, selectProxy_vd, GL_TRIANGLES);
		//TODO add editor SelectComponent
		GTools::selectManager().addComponent(cv_entity,
			std::bind(&Editor::LandscapeTools::activateControlVertexTools, &GTools::landscapeTool(), cv_entity),
			std::bind(&Editor::LandscapeTools::deactivateControlVertexTools, &GTools::landscapeTool()),
			std::bind(&Editor::ControlVertexTools::update, &GTools::controlVertexTool()));
		//GRenderingComponents::interfaceMeshManager().addComponent(cv_entity, "cv", "../resources/materials/editor/interface_cv.slmtl", cv_selectProxy_vertices, cv_selectProxy_indices, selectProxy_vd, GL_TRIANGLES);
#endif
	}
	
	void FeatureCurveComponentManager::addControlVertex(Entity curve, Vec3 position)
	{
		auto search = m_curve_index_map.find(curve.id());

		if (search == m_curve_index_map.end())
			return;

		addControlVertex(search->second, position);
	}

	Entity FeatureCurveComponentManager::insertControlVertex(Entity curve, Entity insert_point_cv, Vec3 position, bool insert_behind)
	{
		auto search = m_curve_index_map.find(curve.id());

		assert(search != m_curve_index_map.end());

		uint index = search->second;

		// go with linear search for now ... not many cvs per curve anyway
		uint counter = 0;
		for (auto cv : m_featureCurves[index].m_controlVertices)
		{
			if (cv.m_entity == insert_point_cv)
			{
				Entity cv_entity = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(cv_entity, position, Quat(), Vec3(1.0));
				GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(cv_entity), m_featureCurves[index].m_entity);

				if(insert_behind)
					m_featureCurves[search->second].m_controlVertices.insert(m_featureCurves[search->second].m_controlVertices.begin() + counter + 1, cv_entity);
				else
					m_featureCurves[search->second].m_controlVertices.insert(m_featureCurves[search->second].m_controlVertices.begin() + counter, cv_entity);

				// Compute world space position of new point
				Vec3 ws_position = GCoreComponents::transformManager().getWorldPosition(cv_entity);

				// Update extends
				m_featureCurves[index].m_lower_corner.x = std::min(m_featureCurves[index].m_lower_corner.x, ws_position.x);
				m_featureCurves[index].m_lower_corner.y = std::min(m_featureCurves[index].m_lower_corner.y, ws_position.y);
				m_featureCurves[index].m_lower_corner.z = std::min(m_featureCurves[index].m_lower_corner.z, ws_position.z);

				m_featureCurves[index].m_upper_corner.x = std::max(m_featureCurves[index].m_upper_corner.x, ws_position.x);
				m_featureCurves[index].m_upper_corner.y = std::max(m_featureCurves[index].m_upper_corner.y, ws_position.y);
				m_featureCurves[index].m_upper_corner.z = std::max(m_featureCurves[index].m_upper_corner.z, ws_position.z);


				// Adjust knot vector and degree
				if (m_featureCurves[index].m_controlVertices.size() > 2 && m_featureCurves[index].m_controlVertices.size() < 5)
				{
					size_t idx = m_featureCurves[index].m_controlVertices.size() - 1;
					m_featureCurves[index].m_knots[idx]--;

					float end_value = m_featureCurves[index].m_knots.back();
					m_featureCurves[index].m_knots.push_back(end_value);
					m_featureCurves[index].m_knots.push_back(end_value);

					m_featureCurves[index].degree++;

				}
				else if (m_featureCurves[index].m_controlVertices.size() > 4)
				{
					size_t start_index = m_featureCurves[index].m_controlVertices.size();
					size_t end_index = m_featureCurves[index].m_knots.size();

					for (size_t i = start_index; i<end_index; i++)
					{
						m_featureCurves[index].m_knots[i]++;
					}

					float end_value = m_featureCurves[index].m_knots.back();
					m_featureCurves[index].m_knots.push_back(end_value);
				}

				m_cv_index_map.insert({ cv_entity.id(),index });

				if (m_featureCurves[index].m_controlVertices.size() > 2)
					updateCurve(index);

#if EDITOR_MODE // preprocessor definition

				std::vector<float> cv_interface_vertices({ -0.1f,-0.1f,0.1f,0.0f,1.0f,0.0f,1.0f,
					0.1f,-0.1f,0.1f,0.0f,1.0f,0.0f,1.0f,
					0.1f,-0.1f,-0.1f,0.0f,1.0f,0.0f,1.0f,
					-0.1f,-0.1f,-0.1f,0.0f,1.0f,0.0f,1.0f,
					-0.1f,0.1f,0.1f,0.0f,1.0f,0.0f,1.0f,
					0.1f,0.1f,0.1f,0.0f,1.0f,0.0f,1.0f,
					0.1f,0.1f,-0.1f,0.0f,1.0f,0.0f,1.0f,
					-0.1f,0.1f,-0.1f,0.0f,1.0f,0.0f,1.0 });
				std::vector<uint32_t> cv_interface_indices({ 0,1, 0,3, 0,4, 1,2, 1,5, 2,3, 2,6, 3,7, 4,5, 4,7, 5,6, 6,7 });

				VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
					VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });

				GRenderingComponents::interfaceMeshManager().addComponent(cv_entity, "cv", "../resources/materials/editor/interface_cv.slmtl", cv_interface_vertices, cv_interface_indices, vertex_description, GL_LINES);

				std::vector<float> cv_selectProxy_vertices({ -0.1f,-0.1f,0.1f,
					0.1f,-0.1f,0.1f,
					0.1f,-0.1f,-0.1f,
					-0.1f,-0.1f,-0.1f,
					-0.1f,0.1f,0.1f,
					0.1f,0.1f,0.1f,
					0.1f,0.1f,-0.1f,
					-0.1f,0.1f,-0.1f });
				std::vector<uint32_t> cv_selectProxy_indices({ 0,2,1, 0,3,2, 4,5,6, 4,6,7, 0,1,5, 0,5,4, 3,6,2, 3,7,6, 3,0,4, 3,4,7, 1,2,6, 1,6,5 });
				VertexLayout selectProxy_vd(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });
				GRenderingComponents::pickingManager().addComponent(cv_entity, "cv_selectProxy", "../resources/materials/editor/picking.slmtl", cv_selectProxy_vertices, cv_selectProxy_indices, selectProxy_vd, GL_TRIANGLES);
				GTools::selectManager().addComponent(cv_entity,
					std::bind(&Editor::LandscapeTools::activateControlVertexTools, &GTools::landscapeTool(), cv_entity),
					std::bind(&Editor::LandscapeTools::deactivateControlVertexTools, &GTools::landscapeTool()),
					std::bind(&Editor::ControlVertexTools::update, &GTools::controlVertexTool()));
				//GRenderingComponents::interfaceMeshManager().addComponent(cv_entity, "cv", "../resources/materials/editor/interface_cv.slmtl", cv_selectProxy_vertices, cv_selectProxy_indices, selectProxy_vd, GL_TRIANGLES);
#endif
				return cv_entity;
			}

			counter++;
		}

		return insert_point_cv;
	}

	Entity FeatureCurveComponentManager::addConstraintPoint(uint index, float curve_position)
	{
		Entity cp_entity = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(cp_entity,calculateCurvePoint(index,curve_position),Quat(),Vec3(1.0));
		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(cp_entity),m_featureCurves[index].m_entity);

		Entity lh_gradient_entity = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(lh_gradient_entity, calculateCurvePoint(index, curve_position), Quat(), Vec3(1.0));
		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(lh_gradient_entity), m_featureCurves[index].m_entity);

		Entity rh_gradient_entity = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(rh_gradient_entity, calculateCurvePoint(index, curve_position), Quat(), Vec3(1.0));
		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(rh_gradient_entity), m_featureCurves[index].m_entity);

		m_featureCurves[index].m_contraintPoints.push_back(FeatureCurveComponent::ConstraintPoint(cp_entity, lh_gradient_entity, rh_gradient_entity, curve_position));

		m_cp_index_map.insert(std::pair<uint, uint>(cp_entity.id(), index));
		m_gradient_cp_entity_map.insert(std::pair<uint, Entity>(lh_gradient_entity.id(), cp_entity));
		m_gradient_cp_entity_map.insert(std::pair<uint, Entity>(rh_gradient_entity.id(), cp_entity));

		// sorting the constraint points by curve_position simplfies computations below
		// but this might have to be improved later on
		std::sort(m_featureCurves[index].m_contraintPoints.begin(),m_featureCurves[index].m_contraintPoints.end(),
			[](const FeatureCurveComponent::ConstraintPoint& a, const FeatureCurveComponent::ConstraintPoint& b){ return a.m_curve_position < b.m_curve_position; } );

		// find inserted constraint point in sorted list
		uint cp_idx = 0;
		for(auto& cp : m_featureCurves[index].m_contraintPoints)
		{
			if(cp.m_entity == cp_entity)
			{
				break;
			}

			cp_idx++;
		}

		FeatureCurveComponent::ConstraintPoint& lower_cp =  m_featureCurves[index].m_contraintPoints[cp_idx -1];
		FeatureCurveComponent::ConstraintPoint& cp =  m_featureCurves[index].m_contraintPoints[cp_idx];
		FeatureCurveComponent::ConstraintPoint& upper_cp =  m_featureCurves[index].m_contraintPoints[cp_idx +1];

		// interpolate constraint point properties
		float alpha = (curve_position - lower_cp.m_curve_position) / (upper_cp.m_curve_position - lower_cp.m_curve_position);

		cp.m_tangent = computeCurveTangent(index, cp.m_curve_position);
		cp.m_gradient_0 = (1.0f-alpha) * lower_cp.m_gradient_0 + alpha * upper_cp.m_gradient_0;
		cp.m_gradient_1 = (1.0f-alpha) * lower_cp.m_gradient_1 + alpha * upper_cp.m_gradient_1;

		// compute curve tangent around cp
		Vec3 previous_tangent = lower_cp.m_tangent;
		Vec3 tangent = glm::normalize(calculateCurvePoint(index,curve_position+0.01f) - calculateCurvePoint(index,curve_position-0.01f));
		float angle = dot(previous_tangent, tangent);
		Vec3 axis = glm::cross(previous_tangent, tangent);

		if (angle < 0.99)
		{
			Quat rotation = glm::angleAxis(acos(angle), glm::normalize(axis));

			cp.m_gradient_0 = glm::normalize(Vec3(glm::toMat4(rotation) * Vec4(cp.m_gradient_0, 1.0)));
			cp.m_gradient_1 = glm::normalize(Vec3(glm::toMat4(rotation) * Vec4(cp.m_gradient_1, 1.0)));
		}
		// better safe than sorry, so project gradient into plane given by tangent
		cp.m_gradient_0 = (cp.m_gradient_0 - (glm::dot(tangent, cp.m_gradient_0) * tangent));
		cp.m_gradient_1 = (cp.m_gradient_1 - (glm::dot(tangent, cp.m_gradient_1) * tangent));


		cp.m_noise_amplitude = (1.0f-alpha) * lower_cp.m_noise_amplitude + alpha * upper_cp.m_noise_amplitude;
		cp.m_noise_roughness = (1.0f-alpha) * lower_cp.m_noise_roughness + alpha * upper_cp.m_noise_roughness;

		recomputeCurveMesh(index);

#if EDITOR_MODE // preprocessor definition

		Vec3 v0 = Vec3(0.0);
		Vec3 v1 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * cp.m_gradient_0;
		Vec3 v2 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * cp.m_gradient_1;
		std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
			v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0,
			v2.x,v2.y,v2.z,1.0,1.0,0.0,1.0 });

		//std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,0.47f,0.125f,0.125f,1.0f,
		//	v1.x,v1.y,v1.z,0.47f,0.125f,0.125f,1.0f,
		//	v2.x,v2.y,v2.z,0.47f,0.125f,0.125f,1.0f });

		std::vector<float> gradient_select_vertices({ v0.x,v0.y,v0.z, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z });

		//TODO insert selection mesh
		std::vector<uint> lh_gradient_indices({ 0,1 });

		VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
			VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });

		GRenderingComponents::interfaceMeshManager().addComponent(lh_gradient_entity,
			"cp_" + std::to_string(cp.m_entity.id()) + "_lh_gradients",
			"../resources/materials/editor/interface_cv.slmtl",
			gradient_interface_vertices,
			lh_gradient_indices,
			vertex_description,
			GL_LINES);

		VertexLayout picking_vertex_description(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });

		GRenderingComponents::pickingManager().addComponent(lh_gradient_entity,
			"cp_" + std::to_string(cp.m_entity.id()) + "_lh_gradient",
			"../resources/materials/editor/picking_cp.slmtl",
			gradient_select_vertices,
			lh_gradient_indices,
			picking_vertex_description,
			GL_LINES);
			//[cp]() { GTools::featureCurveGradientManipulator().activate(cp.m_lefthand_gradient, []() {}); },
			//[cp]() { GTools::featureCurveGradientManipulator().deactivate(); });
		GTools::selectManager().addComponent(lh_gradient_entity,
			[lh_gradient_entity]() { GTools::landscapeTool().activateConstraintPointTools(lh_gradient_entity); },
			[lh_gradient_entity]() { GTools::landscapeTool().deactivateConstraintPointTools(); });

		std::vector<uint> rh_gradient_indices({ 0,2 });

		GRenderingComponents::interfaceMeshManager().addComponent(rh_gradient_entity,
			"cp_" + std::to_string(cp.m_entity.id()) + "_rh_gradients",
			"../resources/materials/editor/interface_cv.slmtl",
			gradient_interface_vertices,
			rh_gradient_indices,
			vertex_description,
			GL_LINES);

		GRenderingComponents::pickingManager().addComponent(rh_gradient_entity,
			"cp_" + std::to_string(cp.m_entity.id()) + "_rh_gradient",
			"../resources/materials/editor/picking_cp.slmtl",
			gradient_select_vertices,
			rh_gradient_indices,
			picking_vertex_description,
			GL_LINES);
			//[cp]() { GTools::featureCurveGradientManipulator().activate(cp.m_righthand_gradient, []() {}); },
			//[cp]() { GTools::featureCurveGradientManipulator().deactivate(); });
		GTools::selectManager().addComponent(rh_gradient_entity,
			[rh_gradient_entity]() { GTools::landscapeTool().activateConstraintPointTools(rh_gradient_entity); },
			[rh_gradient_entity]() { GTools::landscapeTool().deactivateConstraintPointTools(); });
#endif

		return cp_entity;
	}

	Entity FeatureCurveComponentManager::addConstraintPoint(Entity curve_entity, float curve_position)
	{
		auto search = m_curve_index_map.find(curve_entity.id());

		assert(search != m_curve_index_map.end());

		return addConstraintPoint(search->second, curve_position);
	}

	Entity FeatureCurveComponentManager::addConstraintPoint(uint index, float curve_position, Vec3 gradient0, Vec3 gradient1)
	{
		Entity cp_entity = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(cp_entity, calculateCurvePoint(index, curve_position), Quat(), Vec3(1.0));
		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(cp_entity), m_featureCurves[index].m_entity);

		Entity lh_gradient_entity = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(lh_gradient_entity, calculateCurvePoint(index, curve_position), Quat(), Vec3(1.0));
		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(lh_gradient_entity), m_featureCurves[index].m_entity);

		Entity rh_gradient_entity = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(rh_gradient_entity, calculateCurvePoint(index, curve_position), Quat(), Vec3(1.0));
		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(rh_gradient_entity), m_featureCurves[index].m_entity);

		m_featureCurves[index].m_contraintPoints.push_back(FeatureCurveComponent::ConstraintPoint(cp_entity, lh_gradient_entity, rh_gradient_entity, curve_position));

		m_cp_index_map.insert(std::pair<uint, uint>(cp_entity.id(), index));
		m_gradient_cp_entity_map.insert(std::pair<uint, Entity>(lh_gradient_entity.id(), cp_entity));
		m_gradient_cp_entity_map.insert(std::pair<uint, Entity>(rh_gradient_entity.id(), cp_entity));

		// sorting the constraint points by curve_position simplfies computations below
		// but this might have to be improved later on
		std::sort(m_featureCurves[index].m_contraintPoints.begin(), m_featureCurves[index].m_contraintPoints.end(),
			[](const FeatureCurveComponent::ConstraintPoint& a, const FeatureCurveComponent::ConstraintPoint& b) { return a.m_curve_position < b.m_curve_position; });


		// find inserted constraint point in sorted list
		uint cp_idx = 0;
		for (auto& cp : m_featureCurves[index].m_contraintPoints)
		{
			if (cp.m_entity == cp_entity)
			{
				break;
			}

			cp_idx++;
		}

		FeatureCurveComponent::ConstraintPoint& cp = m_featureCurves[index].m_contraintPoints[cp_idx];

		cp.m_tangent = computeCurveTangent(index, cp.m_curve_position);

		if (m_featureCurves[index].m_contraintPoints.size() <= 2) //check for <= since the current constraint point has already been added
		{
			cp.m_gradient_0 = gradient0;
			cp.m_gradient_1 = gradient1;
		}
		else
		{
			FeatureCurveComponent::ConstraintPoint& lower_cp = m_featureCurves[index].m_contraintPoints[cp_idx - 1];
			FeatureCurveComponent::ConstraintPoint& upper_cp = m_featureCurves[index].m_contraintPoints[cp_idx + 1];

			cp.m_gradient_0 = gradient0;
			cp.m_gradient_1 = gradient1;

			// compute curve tangent around cp
			Vec3 tangent = glm::normalize(calculateCurvePoint(index, std::min(1.0f, curve_position + 0.01f)) - calculateCurvePoint(index, std::max(0.0f, curve_position - 0.01f)));
			// project gradient vectors onto plance defined by the tangent
			cp.m_gradient_0 = cp.m_gradient_0 - ((glm::dot(cp.m_gradient_0, tangent) / cp.m_gradient_0.length()) * tangent);
			cp.m_gradient_1 = cp.m_gradient_1 - ((glm::dot(cp.m_gradient_1, tangent) / cp.m_gradient_1.length()) * tangent);


			// interpolate constraint point properties
			float alpha = (curve_position - lower_cp.m_curve_position) / (upper_cp.m_curve_position - lower_cp.m_curve_position);

			cp.m_noise_amplitude = (1.0f - alpha) * lower_cp.m_noise_amplitude + alpha * upper_cp.m_noise_amplitude;
			cp.m_noise_roughness = (1.0f - alpha) * lower_cp.m_noise_roughness + alpha * upper_cp.m_noise_roughness;

			recomputeCurveMesh(index);
		}

#if EDITOR_MODE // preprocessor definition

		Vec3 v0 = Vec3(0.0);
		Vec3 v1 = v0 + 1.5f * cp.m_gradient_0;
		Vec3 v2 = v0 + 1.5f * cp.m_gradient_1;
		std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
			v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0,
			v2.x,v2.y,v2.z,1.0,1.0,0.0,1.0 });

		//std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,0.47f,0.125f,0.125f,1.0f,
		//	v1.x,v1.y,v1.z,0.47f,0.125f,0.125f,1.0f,
		//	v2.x,v2.y,v2.z,0.47f,0.125f,0.125f,1.0f });
		//
		std::vector<float> gradient_select_vertices({ v0.x,v0.y,v0.z, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z });

		//TODO insert selection mesh
		std::vector<uint> lh_gradient_indices({ 0,1 });

		VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
			VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });

		GRenderingComponents::interfaceMeshManager().addComponent(lh_gradient_entity,
			"cp_" + std::to_string(cp.m_entity.id()) + "_lh_gradients",
			"../resources/materials/editor/interface_cv.slmtl",
			gradient_interface_vertices,
			lh_gradient_indices,
			vertex_description,
			GL_LINES);

		VertexLayout picking_vertex_description(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0)});

		GRenderingComponents::pickingManager().addComponent(lh_gradient_entity,
			"cp_" + std::to_string(cp.m_entity.id()) + "_lh_gradient",
			"../resources/materials/editor/picking_cp.slmtl",
			gradient_select_vertices,
			lh_gradient_indices,
			picking_vertex_description,
			GL_LINES);
		//[cp]() { GTools::featureCurveGradientManipulator().activate(cp.m_lefthand_gradient, []() {}); },
		//[cp]() { GTools::featureCurveGradientManipulator().deactivate(); });
		GTools::selectManager().addComponent(lh_gradient_entity,
			[lh_gradient_entity]() { GTools::landscapeTool().activateConstraintPointTools(lh_gradient_entity); },
			[lh_gradient_entity]() { GTools::landscapeTool().deactivateConstraintPointTools(); });

		std::vector<uint> rh_gradient_indices({ 0,2 });

		GRenderingComponents::interfaceMeshManager().addComponent(rh_gradient_entity,
			"cp_" + std::to_string(cp.m_entity.id()) + "_rh_gradients",
			"../resources/materials/editor/interface_cv.slmtl",
			gradient_interface_vertices,
			rh_gradient_indices,
			vertex_description,
			GL_LINES);

		GRenderingComponents::pickingManager().addComponent(rh_gradient_entity,
			"cp_" + std::to_string(cp.m_entity.id()) + "_rh_gradient",
			"../resources/materials/editor/picking_cp.slmtl",
			gradient_select_vertices,
			rh_gradient_indices,
			picking_vertex_description,
			GL_LINES);
		//[cp]() { GTools::featureCurveGradientManipulator().activate(cp.m_righthand_gradient, []() {}); },
		//[cp]() { GTools::featureCurveGradientManipulator().deactivate(); });
		GTools::selectManager().addComponent(rh_gradient_entity,
			[rh_gradient_entity]() { GTools::landscapeTool().activateConstraintPointTools(rh_gradient_entity); },
			[rh_gradient_entity]() { GTools::landscapeTool().deactivateConstraintPointTools(); });
#endif

		return cp_entity;
	}

	void FeatureCurveComponentManager::setConstraintPointCurvePosition(Entity cp_entity, float curve_position)
	{
		auto search = m_cp_index_map.find(cp_entity.id());

		if (search == m_cp_index_map.end())
			return;

		uint curve_idx = search->second;

		for (auto& cp : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (cp.m_entity == cp_entity)
			{
				cp.m_curve_position = curve_position;

				std::sort(m_featureCurves[curve_idx].m_contraintPoints.begin(), m_featureCurves[curve_idx].m_contraintPoints.end(),
					[](const FeatureCurveComponent::ConstraintPoint& a, const FeatureCurveComponent::ConstraintPoint& b) { return a.m_curve_position < b.m_curve_position; });

				updateCurve(curve_idx);

				return;
			}
		}
	}

	float FeatureCurveComponentManager::getConstraintPointCurvePosition(Entity cp_entity)
	{
		auto search = m_cp_index_map.find(cp_entity.id());

		if (search == m_cp_index_map.end())
			return 0.0f;

		uint curve_idx = search->second;

		for (auto& cp : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (cp.m_entity == cp_entity)
			{
				return cp.m_curve_position;
			}
		}

		return 0.0f;
	}

	void FeatureCurveComponentManager::setConstraintPointGradient(Entity gradient_entity, Vec3 new_gradient)
	{
		auto search = m_gradient_cp_entity_map.find(gradient_entity.id());

		if (search == m_gradient_cp_entity_map.end())
			return;

		Entity cp_entiy = search->second;

		auto search_2 = m_cp_index_map.find(cp_entiy.id());

		if (search_2 == m_cp_index_map.end())
			return;

		uint curve_idx = search_2->second;

		for (auto& cp : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (cp.m_entity.id() == cp_entiy.id())
			{
				if (gradient_entity == cp.m_lefthand_gradient)
					cp.m_gradient_0 = new_gradient;
				else if (gradient_entity == cp.m_righthand_gradient)
					cp.m_gradient_1 = new_gradient;

				updateCurve(curve_idx);

				return;
			}
		}
	}

	void FeatureCurveComponentManager::setConstraintPointGradient(Entity cp_entity, Vec3 gradient, int gradient_idx)
	{
		auto search = m_cp_index_map.find(cp_entity.id());
		
		if( search == m_cp_index_map.end() )
			return;
		
		uint curve_idx = search->second;
		
		for(auto& cp : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if(cp.m_entity == cp_entity)
			{
				if(gradient_idx == 0)
					cp.m_gradient_0 = gradient;
				else if(gradient_idx == 1)
					cp.m_gradient_1 = gradient;
		
				updateCurve(curve_idx);

				return;
			}
		}
	}

	void FeatureCurveComponentManager::setConstraintPointGradient(uint curve_idx, uint cp_idx, Vec3 gradient, int gradient_idx)
	{
		if (m_featureCurves[curve_idx].m_contraintPoints.size() <= cp_idx)
			return;

		if (gradient_idx == 0)
			m_featureCurves[curve_idx].m_contraintPoints[cp_idx].m_gradient_0 = gradient;
		else if (gradient_idx == 1)
			m_featureCurves[curve_idx].m_contraintPoints[cp_idx].m_gradient_1 = gradient;

		updateCurve(curve_idx);
	}

	void FeatureCurveComponentManager::setConstraintPointGradient(Entity curve, uint cp_idx, Vec3 gradient, int gradient_idx)
	{
		auto search = m_curve_index_map.find(curve.id());

		if (search == m_curve_index_map.end())
			return;

		setConstraintPointGradient(search->second, cp_idx, gradient, gradient_idx);
	}

	void FeatureCurveComponentManager::setConstraintPointNoise(uint curve_idx, uint cp_idx, float amplitude, float roughness)
	{
		if (m_featureCurves[curve_idx].m_contraintPoints.size() <= cp_idx)
			return;

		m_featureCurves[curve_idx].m_contraintPoints[cp_idx].m_noise_amplitude = amplitude;
		m_featureCurves[curve_idx].m_contraintPoints[cp_idx].m_noise_roughness = roughness;

		updateCurve(curve_idx);
	}

	void FeatureCurveComponentManager::setConstraintPointNoise(Entity curve, uint cp_idx, float amplitude, float roughness)
	{
		auto search = m_curve_index_map.find(curve.id());

		if (search == m_curve_index_map.end())
			return;

		setConstraintPointNoise(search->second, cp_idx, amplitude, roughness);
	}

	void FeatureCurveComponentManager::setConstraintPointNoiseAmplitude(Entity cp, float amplitude)
	{
		auto search = m_cp_index_map.find(cp.id());

		if (search == m_cp_index_map.end())
			return;

		uint curve_idx = search->second;

		for (auto& constraint_point : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (constraint_point.m_entity == cp)
			{
				constraint_point.m_noise_amplitude = amplitude;

				updateCurve(curve_idx);

				return;
			}
		}
	}

	float FeatureCurveComponentManager::getConstraintPointNoiseAmplitude(Entity cp)
	{
		auto search = m_cp_index_map.find(cp.id());

		if (search == m_cp_index_map.end())
			return 0.0f;

		uint curve_idx = search->second;

		for (auto& constraint_point : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (constraint_point.m_entity == cp)
			{
				return constraint_point.m_noise_amplitude;
			}
		}

		return 0.0f;
	}

	void FeatureCurveComponentManager::setConstraintPointNoiseRoughness(Entity cp, float roughness)
	{
		auto search = m_cp_index_map.find(cp.id());

		if (search == m_cp_index_map.end())
			return;

		uint curve_idx = search->second;

		for (auto& constraint_point : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (constraint_point.m_entity == cp)
			{
				constraint_point.m_noise_roughness = roughness;

				updateCurve(curve_idx);

				return;
			}
		}
	}

	float FeatureCurveComponentManager::getConstraintPointNoiseRoughness(Entity cp)
	{
		auto search = m_cp_index_map.find(cp.id());

		if (search == m_cp_index_map.end())
			return 0.0f;

		uint curve_idx = search->second;

		for (auto& constraint_point : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (constraint_point.m_entity == cp)
			{
				return constraint_point.m_noise_roughness;
			}
		}

		return 0.0f;
	}

	void FeatureCurveComponentManager::setConstrainPointMaterialIDs(Entity cp, int matID_0, int matID_1)
	{
		auto search = m_cp_index_map.find(cp.id());

		if (search == m_cp_index_map.end())
			return;

		uint curve_idx = search->second;

		for (auto& constraint_point : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (constraint_point.m_entity == cp)
			{
				constraint_point.m_material_0 = matID_0;
				constraint_point.m_material_1 = matID_1;

				updateCurve(curve_idx);

				return;
			}
		}
	}

	std::pair<int, int> FeatureCurveComponentManager::getConstraintPointMaterialIDs(Entity cp)
	{
		auto search = m_cp_index_map.find(cp.id());

		if (search == m_cp_index_map.end())
			return std::pair<int,int>(0,0);

		uint curve_idx = search->second;

		for (auto& constraint_point : m_featureCurves[curve_idx].m_contraintPoints)
		{
			if (constraint_point.m_entity == cp)
			{
				return std::pair<int, int>(constraint_point.m_material_0, constraint_point.m_material_1);
			}
		}

		return std::pair<int, int>(0, 0);
	}

	Vec3 FeatureCurveComponentManager::getConstraintPointTangent(Entity cp_entity)
	{
		auto search = m_cp_index_map.find(cp_entity.id());

		assert(search != m_cp_index_map.end());

		uint idx = search->second;
		float u = 0.0f;

		for (auto& cp : m_featureCurves[idx].m_contraintPoints)
		{
			if (cp.m_entity == cp_entity)
			{
				u = cp.m_curve_position;
				break;
			}
		}

		return computeCurveTangent(idx, u);
	}

	Entity FeatureCurveComponentManager::getPreviousCV(Entity cv)
	{
		auto search = m_cv_index_map.find(cv.id());

		assert(search != m_cv_index_map.end());

		uint curve_idx = search->second;

		for (auto itr = m_featureCurves[curve_idx].m_controlVertices.begin(); itr != m_featureCurves[curve_idx].m_controlVertices.end(); ++itr)
		{
			if ((*itr).m_entity == cv)
			{
				if (itr != m_featureCurves[curve_idx].m_controlVertices.begin())
					return (--itr)->m_entity;
				else
					return itr->m_entity;
			}
		}
	}

	Entity FeatureCurveComponentManager::getNextCV(Entity cv)
	{
		auto search = m_cv_index_map.find(cv.id());

		assert(search != m_cv_index_map.end());

		uint curve_idx = search->second;

		for (auto itr = m_featureCurves[curve_idx].m_controlVertices.begin(); itr != m_featureCurves[curve_idx].m_controlVertices.end(); ++itr)
		{
			if ((*itr).m_entity == cv)
			{
				if ( (itr+1) != m_featureCurves[curve_idx].m_controlVertices.end())
					return (++itr)->m_entity;
				else
					return itr->m_entity;
			}
		}
	}

	void FeatureCurveComponentManager::setRibbonWidth(float ribbon_width)
	{
		for (int i = 0; i < m_featureCurves.size(); ++i)
		{
			m_featureCurves[i].m_ribbon_width = ribbon_width;
			updateCurve(i);
		}
	}

	void FeatureCurveComponentManager::recomputeCurveMesh(uint index)
	{
		// recompute mesh curve points

		m_featureCurves[index].m_mesh_curvePoints.clear();
		
		//	float cv_spacing = 1.0f / (float)(m_featureCurves[index].m_controlVertices.size()-1);
		//	
		//	for(size_t i=1; i< m_featureCurves[index].m_controlVertices.size(); i++)
		//	{
		//		Vec3 p1 = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(m_featureCurves[index].m_controlVertices[i].m_entity));
		//		Vec3 p2 = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(m_featureCurves[index].m_controlVertices[i-1].m_entity));
		//	
		//		float distance = glm::length(p1-p2);
		//	
		//		uint segment_subdivs = std::max((uint)1,(uint)std::floor(distance/m_featureCurves[index].m_mesh_pointDistance));
		//	
		//		for(uint j=0; j<=segment_subdivs; j++)
		//		{
		//			m_featureCurves.back().m_mesh_curvePoints.push_back( (float)(i-1) * cv_spacing + ((float)j * (cv_spacing/(float)segment_subdivs)) );
		//		}
		//	}

		for(uint i=0; i<=100; i++)
			m_featureCurves[index].m_mesh_curvePoints.push_back((float)i/100.0f); // ?


		size_t mesh_subdivs = m_featureCurves[index].m_mesh_curvePoints.size();

		assert(mesh_subdivs > 0);

		m_featureCurves.back().m_mesh_vertices.resize( (mesh_subdivs) * 15 * 4); // 15 floats per vertex, 4 vertices per curve point
		m_featureCurves.back().m_mesh_indices.resize( (mesh_subdivs-1) * 4 * 3); // 4 tris per subdiv, 3 vertes per tri

		// find start cp
		uint current_cp = 0;
		for(auto& cp : m_featureCurves[index].m_contraintPoints)
		{
			if( cp.m_curve_position > m_featureCurves[index].m_mesh_curvePoints[0] ) // there should always be a first mesh curve point
				break;

			current_cp++;
		}

		float curve_section_length = m_featureCurves[index].m_contraintPoints[current_cp].m_curve_position
										- m_featureCurves[index].m_contraintPoints[current_cp-1].m_curve_position;

		uint counter = 0;

		for (auto curve_point : m_featureCurves[index].m_mesh_curvePoints)
		{
			if (curve_point > m_featureCurves[index].m_contraintPoints[current_cp].m_curve_position)
			{
				current_cp++;
				curve_section_length = m_featureCurves[index].m_contraintPoints[current_cp].m_curve_position
					- m_featureCurves[index].m_contraintPoints[current_cp - 1].m_curve_position;
			}

			float cp_b_curvePos = m_featureCurves[index].m_contraintPoints[current_cp - 1].m_curve_position; // backward cp's position in curve parameter space
			float cp_f_curvePos = m_featureCurves[index].m_contraintPoints[current_cp].m_curve_position; // forward cp's position in curve parameter space

			float alpha = (curve_point - cp_b_curvePos) / curve_section_length;

			// Transform gradients from surrounding cps to curve position, than interpolate
			Vec3 tangent_b = computeCurveTangent(index, cp_b_curvePos);
			Vec3 tangent_f = computeCurveTangent(index, cp_f_curvePos);
			Vec3 tangent_c = computeCurveTangent(index, curve_point);

			float b_rotation_angle = acos(dot(tangent_c, tangent_b));
			Vec3 b_rotation_axis = glm::normalize((glm::cross(tangent_b, tangent_c)));

			float f_rotation_angle = acos(dot(tangent_c, tangent_f));
			Vec3 f_rotation_axis = glm::normalize(glm::cross(tangent_f, tangent_c));

			Vec3 gradient_b0 = m_featureCurves[index].m_contraintPoints[current_cp - 1].m_gradient_0;
			Vec3 gradient_b1 = m_featureCurves[index].m_contraintPoints[current_cp - 1].m_gradient_1;
			Vec3 gradient_f0 = m_featureCurves[index].m_contraintPoints[current_cp].m_gradient_0;
			Vec3 gradient_f1 = m_featureCurves[index].m_contraintPoints[current_cp].m_gradient_1;

			if (b_rotation_angle > 0.01)
			{
				Quat rotation_b = glm::angleAxis(b_rotation_angle, b_rotation_axis);

				gradient_b0 = glm::normalize(glm::transpose(glm::inverse(glm::mat3(glm::toMat4(rotation_b)))) * gradient_b0);
				gradient_b1 = glm::normalize(glm::transpose(glm::inverse(glm::mat3(glm::toMat4(rotation_b)))) * gradient_b1);
			}

			if (f_rotation_angle > 0.01)
			{
				Quat rotation_f = glm::angleAxis(f_rotation_angle, f_rotation_axis);

				gradient_f1 = glm::normalize(glm::transpose(glm::inverse(glm::mat3(glm::toMat4(rotation_f)))) * gradient_f1);
				gradient_f0 = glm::normalize(glm::transpose(glm::inverse(glm::mat3(glm::toMat4(rotation_f)))) * gradient_f0);
			}

			Vec3 gradient_0 = alpha * gradient_f0 + (1.0f - alpha) * gradient_b0;
			Vec3 gradient_1 = alpha * gradient_f1 + (1.0f - alpha) * gradient_b1;

			// calculate position and tangent
			Vec3 position = calculateCurvePoint(index,curve_point);
			Vec3 tangent = glm::normalize( calculateCurvePoint(index,std::min(curve_point+0.02f,1.0f)) - calculateCurvePoint(index,std::max(curve_point-0.02f,0.0f)) );

			// project gradient into plane given by tangent
			gradient_0 = (gradient_0 - ( glm::dot(tangent,gradient_0) * tangent) );
			gradient_1 = (gradient_1 - ( glm::dot(tangent,gradient_1) * tangent) );

			// calculate normal from curve tangent and gradient vectors
			gradient_0 = 1.0f * glm::normalize(gradient_0);
			gradient_1 = 1.0f * glm::normalize(gradient_1);

			Vec3 normal = glm::normalize(glm::cross(tangent,gradient_0));

			// interpolate noise parameters
			float noise_amp_b = m_featureCurves[index].m_contraintPoints[current_cp - 1].m_noise_amplitude;
			float noise_amp_f = m_featureCurves[index].m_contraintPoints[current_cp].m_noise_amplitude;
			float noise_rough_b = m_featureCurves[index].m_contraintPoints[current_cp - 1].m_noise_roughness;
			float noise_rough_f = m_featureCurves[index].m_contraintPoints[current_cp].m_noise_roughness;			
			float noise_amplitude = (1.0f - alpha) * noise_amp_b + alpha * noise_amp_f;
			float noise_roughness = (1.0f - alpha) * noise_rough_b + alpha * noise_rough_f;

			// select material ID from closest contraint point
			int matID_0 = (curve_point - cp_b_curvePos) < (cp_f_curvePos - curve_point) ? m_featureCurves[index].m_contraintPoints[current_cp - 1].m_material_0 : m_featureCurves[index].m_contraintPoints[current_cp].m_material_0;
			int matID_1 = (curve_point - cp_b_curvePos) < (cp_f_curvePos - curve_point) ? m_featureCurves[index].m_contraintPoints[current_cp - 1].m_material_1 : m_featureCurves[index].m_contraintPoints[current_cp].m_material_1;

			float ribbon_width = m_featureCurves[index].m_ribbon_width;

			m_featureCurves[index].m_mesh_vertices[counter] = position.x;
			m_featureCurves[index].m_mesh_vertices[counter+1] = position.y;
			m_featureCurves[index].m_mesh_vertices[counter+2] = position.z;
			m_featureCurves[index].m_mesh_vertices[counter+3] = normal.x;
			m_featureCurves[index].m_mesh_vertices[counter+4] = normal.y;
			m_featureCurves[index].m_mesh_vertices[counter+5] = normal.z;
			m_featureCurves[index].m_mesh_vertices[counter + 6] = gradient_0.x;
			m_featureCurves[index].m_mesh_vertices[counter + 7] = gradient_0.y;
			m_featureCurves[index].m_mesh_vertices[counter + 8] = gradient_0.z;
			m_featureCurves[index].m_mesh_vertices[counter + 9] = 0.0;// gradient_1.x;
			m_featureCurves[index].m_mesh_vertices[counter + 10] = 0.0;//gradient_1.y;
			m_featureCurves[index].m_mesh_vertices[counter + 11] = 0.0;//gradient_1.z;
			m_featureCurves[index].m_mesh_vertices[counter + 12] = noise_amplitude; //noise amplitude
			m_featureCurves[index].m_mesh_vertices[counter + 13] = noise_roughness; //noise roughness
			m_featureCurves[index].m_mesh_vertices[counter + 14] = static_cast<float>(matID_0);

			m_featureCurves[index].m_mesh_vertices[counter+15] = (position + gradient_0*ribbon_width).x;
			m_featureCurves[index].m_mesh_vertices[counter+16] = (position + gradient_0*ribbon_width).y;
			m_featureCurves[index].m_mesh_vertices[counter+17] = (position + gradient_0*ribbon_width).z;
			m_featureCurves[index].m_mesh_vertices[counter+18] = normal.x;
			m_featureCurves[index].m_mesh_vertices[counter+19] = normal.y;
			m_featureCurves[index].m_mesh_vertices[counter+20] = normal.z;
			m_featureCurves[index].m_mesh_vertices[counter + 21] = gradient_0.x;
			m_featureCurves[index].m_mesh_vertices[counter + 22] = gradient_0.y;
			m_featureCurves[index].m_mesh_vertices[counter + 23] = gradient_0.z;
			m_featureCurves[index].m_mesh_vertices[counter + 24] = 0.0;//gradient_1.x;
			m_featureCurves[index].m_mesh_vertices[counter + 25] = 0.0;//gradient_1.y;
			m_featureCurves[index].m_mesh_vertices[counter + 26] = 0.0;//gradient_1.z;
			m_featureCurves[index].m_mesh_vertices[counter + 27] = noise_amplitude;
			m_featureCurves[index].m_mesh_vertices[counter + 28] = noise_roughness;
			m_featureCurves[index].m_mesh_vertices[counter + 29] = static_cast<float>(matID_0);
			
			size_t offset = m_featureCurves.back().m_mesh_vertices.size()/2;

			normal = glm::normalize(glm::cross(tangent,-gradient_1));
			
			m_featureCurves[index].m_mesh_vertices[counter + offset] = position.x;
			m_featureCurves[index].m_mesh_vertices[counter + offset +1] = position.y;
			m_featureCurves[index].m_mesh_vertices[counter + offset +2] = position.z;
			m_featureCurves[index].m_mesh_vertices[counter + offset +3] = normal.x;
			m_featureCurves[index].m_mesh_vertices[counter + offset +4] = normal.y;
			m_featureCurves[index].m_mesh_vertices[counter + offset +5] = normal.z;
			m_featureCurves[index].m_mesh_vertices[counter + offset +6] = 0.0;//gradient_0.x;
			m_featureCurves[index].m_mesh_vertices[counter + offset +7] = 0.0;//gradient_0.y;
			m_featureCurves[index].m_mesh_vertices[counter + offset +8] = 0.0;//gradient_0.z;
			m_featureCurves[index].m_mesh_vertices[counter + offset +9] = gradient_1.x;
			m_featureCurves[index].m_mesh_vertices[counter + offset +10] = gradient_1.y;
			m_featureCurves[index].m_mesh_vertices[counter + offset +11] = gradient_1.z;
			m_featureCurves[index].m_mesh_vertices[counter + offset +12] = noise_amplitude;
			m_featureCurves[index].m_mesh_vertices[counter + offset +13] = noise_roughness;
			m_featureCurves[index].m_mesh_vertices[counter + offset +14] = static_cast<float>(matID_1);
			
			m_featureCurves[index].m_mesh_vertices[counter + offset +15] = (position + gradient_1*ribbon_width).x;
			m_featureCurves[index].m_mesh_vertices[counter + offset +16] = (position + gradient_1*ribbon_width).y;
			m_featureCurves[index].m_mesh_vertices[counter + offset +17] = (position + gradient_1*ribbon_width).z;
			m_featureCurves[index].m_mesh_vertices[counter + offset +18] = normal.x;
			m_featureCurves[index].m_mesh_vertices[counter + offset +19] = normal.y;
			m_featureCurves[index].m_mesh_vertices[counter + offset +20] = normal.z;
			m_featureCurves[index].m_mesh_vertices[counter + offset +21] = 0.0;//gradient_0.x;
			m_featureCurves[index].m_mesh_vertices[counter + offset +22] = 0.0;//gradient_0.y;
			m_featureCurves[index].m_mesh_vertices[counter + offset +23] = 0.0;//gradient_0.z;
			m_featureCurves[index].m_mesh_vertices[counter + offset + 24] = gradient_1.x;
			m_featureCurves[index].m_mesh_vertices[counter + offset + 25] = gradient_1.y;
			m_featureCurves[index].m_mesh_vertices[counter + offset + 26] = gradient_1.z;
			m_featureCurves[index].m_mesh_vertices[counter + offset + 27] = noise_amplitude;
			m_featureCurves[index].m_mesh_vertices[counter + offset + 28] = noise_roughness;
			m_featureCurves[index].m_mesh_vertices[counter + offset + 29] = static_cast<float>(matID_1);
			
			counter += 30;
		}

		// set indices
		uint vertex_index = 2;

		for(uint i=0; i < m_featureCurves.back().m_mesh_indices.size(); i = i+6)
		{
			if(i == (m_featureCurves.back().m_mesh_indices.size())/2)
				vertex_index += 2;

			//	m_featureCurves[index].m_mesh_indices[i] = vertex_index;
			//	m_featureCurves[index].m_mesh_indices[i+1] = vertex_index-1;
			//	m_featureCurves[index].m_mesh_indices[i+2] = vertex_index-2;
			//	
			//	m_featureCurves[index].m_mesh_indices[i+3] = vertex_index;
			//	m_featureCurves[index].m_mesh_indices[i+4] = vertex_index-1;
			//	m_featureCurves[index].m_mesh_indices[i+5] = vertex_index+1;

			m_featureCurves[index].m_mesh_indices[i] = vertex_index - 2;
			m_featureCurves[index].m_mesh_indices[i + 1] = vertex_index - 1;
			m_featureCurves[index].m_mesh_indices[i + 2] = vertex_index;

			m_featureCurves[index].m_mesh_indices[i + 3] = vertex_index;
			m_featureCurves[index].m_mesh_indices[i + 4] = vertex_index + 1;
			m_featureCurves[index].m_mesh_indices[i + 5] = vertex_index - 1;

			vertex_index += 2;
		}

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask( [this,index] { this->bufferMeshData(index); } );
	}

	void FeatureCurveComponentManager::updateCurve(uint index)
	{
		// Update extends
		for (auto& cv : m_featureCurves[index].m_controlVertices)
		{
			Vec3 cv_position = GCoreComponents::transformManager().getWorldPosition(cv.m_entity);
			m_featureCurves[index].m_lower_corner.x = std::min(m_featureCurves[index].m_lower_corner.x, cv_position.x);
			m_featureCurves[index].m_lower_corner.y = std::min(m_featureCurves[index].m_lower_corner.y, cv_position.y);
			m_featureCurves[index].m_lower_corner.z = std::min(m_featureCurves[index].m_lower_corner.z, cv_position.z);

			m_featureCurves[index].m_upper_corner.x = std::max(m_featureCurves[index].m_upper_corner.x, cv_position.x);
			m_featureCurves[index].m_upper_corner.y = std::max(m_featureCurves[index].m_upper_corner.y, cv_position.y);
			m_featureCurves[index].m_upper_corner.z = std::max(m_featureCurves[index].m_upper_corner.z, cv_position.z);
		}

		// recompute all constraint point positions
		for(auto& constraintPoint : m_featureCurves[index].m_contraintPoints)
		{
			// Compute old tangent vector
			//TODO this doesn't work obviously...cvs have already changed...idea: store tangent with constraint point as reference
			Vec3 old_tangent = constraintPoint.m_tangent;

			Vec3 new_position = calculateCurvePoint(index,constraintPoint.m_curve_position);
			GCoreComponents::transformManager().setPosition(GCoreComponents::transformManager().getIndex(constraintPoint.m_entity), new_position);

			GCoreComponents::transformManager().setPosition(GCoreComponents::transformManager().getIndex(constraintPoint.m_lefthand_gradient), new_position);
			GCoreComponents::transformManager().setPosition(GCoreComponents::transformManager().getIndex(constraintPoint.m_righthand_gradient), new_position);

			// compute curve tangent around new cp position
			Vec3 tangent = computeCurveTangent(index, constraintPoint.m_curve_position);
			constraintPoint.m_tangent = tangent;

			float angle = dot(old_tangent, tangent);
			Vec3 axis = glm::cross(old_tangent, tangent);

			if ( angle < 0.99)
			{
				Quat rotation = glm::angleAxis(acos(angle), glm::normalize(axis));

				constraintPoint.m_gradient_0 = glm::normalize(Vec3(glm::toMat4(rotation) * Vec4(constraintPoint.m_gradient_0, 1.0)));
				constraintPoint.m_gradient_1 = glm::normalize(Vec3(glm::toMat4(rotation) * Vec4(constraintPoint.m_gradient_1, 1.0)));
			}

			// better safe than sorry, so project gradient into plane given by tangent
			constraintPoint.m_gradient_0 = (constraintPoint.m_gradient_0 - (glm::dot(tangent, constraintPoint.m_gradient_0) * tangent));
			constraintPoint.m_gradient_1 = (constraintPoint.m_gradient_1 - (glm::dot(tangent, constraintPoint.m_gradient_1) * tangent));

			/*
			//TODO fix gradient project or -more likely- switch to a different approach

			Vec3 ref = Vec3(0.0, 1.0, 0.0);
			if (glm::dot(tangent, ref) > 0.7)
				ref = Vec3(0.0, 0.0, 1.0);

			ref = glm::cross(tangent, ref);
			ref = glm::cross(tangent, ref);

			// project gradient vectors onto plane defined by the tangent (screws up because gradient can switch sides if curve curves too much)
			constraintPoint.m_gradient_0 = constraintPoint.m_gradient_0 - ( glm::dot(tangent,constraintPoint.m_gradient_0) * tangent);
			constraintPoint.m_gradient_1 = constraintPoint.m_gradient_1 - ( glm::dot(tangent,constraintPoint.m_gradient_1) * tangent);

			constraintPoint.m_gradient_0 = glm::normalize(constraintPoint.m_gradient_0);
			constraintPoint.m_gradient_1 = glm::normalize(constraintPoint.m_gradient_1);
			*/


#if EDITOR_MODE // preprocessor definition

			Vec3 v0 = Vec3(0.0);
			Vec3 v1 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * constraintPoint.m_gradient_0;
			Vec3 v2 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * constraintPoint.m_gradient_1;
			std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
				v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0,
				v2.x,v2.y,v2.z,1.0,1.0,0.0,1.0 });

			//std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,0.47f,0.125f,0.125f,1.0f,
			//	v1.x,v1.y,v1.z,0.47f,0.125f,0.125f,1.0f,
			//	v2.x,v2.y,v2.z,0.47f,0.125f,0.125f,1.0f });
			
			std::vector<float> gradient_select_vertices({ v0.x,v0.y,v0.z, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z });

			//TODO insert selection mesh
			std::vector<uint> lh_gradient_indices({ 0,1 });

			VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
				VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });

			GRenderingComponents::interfaceMeshManager().updateComponent(constraintPoint.m_lefthand_gradient,
				gradient_interface_vertices,
				lh_gradient_indices,
				vertex_description,
				GL_LINES);

			VertexLayout picking_vertex_description(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });

			GRenderingComponents::pickingManager().updateComponentProxyGeometry(constraintPoint.m_lefthand_gradient,
				gradient_select_vertices,
				lh_gradient_indices,
				picking_vertex_description,
				GL_LINES);


			std::vector<uint> rh_gradient_indices({ 0,2 });

			GRenderingComponents::interfaceMeshManager().updateComponent(constraintPoint.m_righthand_gradient,
				gradient_interface_vertices,
				rh_gradient_indices,
				vertex_description,
				GL_LINES);

			GRenderingComponents::pickingManager().updateComponentProxyGeometry(constraintPoint.m_righthand_gradient,
				gradient_select_vertices,
				rh_gradient_indices,
				picking_vertex_description,
				GL_LINES);
#endif
		}

		// recompute mesh data
		recomputeCurveMesh(index);

#if EDITOR_MODE // preprocessor definition

		VertexLayout vertex_description(60,{VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
											VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 3),
											VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 6),
											VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 9),
											VertexLayout::Attribute(GL_FLOAT,2,GL_FALSE,sizeof(GLfloat) * 12),
											VertexLayout::Attribute(GL_FLOAT,1,GL_FALSE,sizeof(GLfloat) * 14) });

		GRenderingComponents::interfaceMeshManager().updateComponent(m_featureCurves[index].m_entity,
												m_featureCurves[index].m_mesh_vertices,
												m_featureCurves[index].m_mesh_indices,
												vertex_description,
												GL_TRIANGLES);

		GRenderingComponents::pickingManager().updateComponentProxyGeometry(m_featureCurves[index].m_entity,
			m_featureCurves[index].m_mesh_vertices,
			m_featureCurves[index].m_mesh_indices,
			vertex_description,
			GL_TRIANGLES);
#endif
	}

	void FeatureCurveComponentManager::updateCorrespondingCurve(uint cv_entity_id)
	{
		auto search = m_cv_index_map.find(cv_entity_id);

		if (search == m_cv_index_map.end())
			return;

		updateCurve(search->second);
	}

	Vec3 FeatureCurveComponentManager::calculateCurvePoint(uint index, float curve_position)
	{
		// check if enough control points to build a spline
		assert( m_featureCurves[index].m_controlVertices.size() > 1 );
		assert( curve_position <= 1.0f );
		assert( curve_position >= 0.0f );

		//TODO think of proper solution! for now just clamp;

		curve_position = std::min(1.0f, std::max(0.0f, curve_position));

		// calculate i
		size_t num_CVs = m_featureCurves[index].m_controlVertices.size();
		size_t num_knots = m_featureCurves[index].m_knots.size();

		float knot_vector_value = curve_position * (num_CVs - m_featureCurves[index].degree); //num_CVs - degree

		//uint i = 0;
		//for(auto& knot : m_featureCurves[index].m_knots)
		//{
		//	if(knot >= knot_vector_value )
		//		break;
		//
		//	i++;
		//}
		//i--;

		uint i = m_featureCurves[index].degree;
		for(; i<m_featureCurves[index].m_knots.size(); i++)
		{
			if(m_featureCurves[index].m_knots[i+1] >= knot_vector_value )
				break;
		}


		//std::cout<<"i: "<<i<<std::endl;
		//std::cout<<"Degree: "<<m_featureCurves[index].degree<<std::endl;
		//
		//std::cout<<"Knot vector: ";
		//for(auto& knot : m_featureCurves[index].m_knots)
		//{
		//	std::cout<<knot<<" ";
		//
		//}
		//std::cout<<std::endl;

		return recursiveDeBoor(index,m_featureCurves[index].degree,i,knot_vector_value);
	}

	Vec3 FeatureCurveComponentManager::recursiveDeBoor(uint index, uint k, uint i, float x)
	{
		if(k == 0)
		{
			uint transform_idx = GCoreComponents::transformManager().getIndex(m_featureCurves[index].m_controlVertices[i].m_entity);

			return GCoreComponents::transformManager().getPosition(transform_idx);
		}
		else
		{
			float u_i = m_featureCurves[index].m_knots[i];
			float u_in1k = m_featureCurves[index].m_knots[i + m_featureCurves[index].degree + 1 - k];

			float alpha_k_i = ( x - u_i ) / ( u_in1k - u_i);

			return ( (1.0f-alpha_k_i)*recursiveDeBoor(index,k-1,i-1,x) + alpha_k_i*recursiveDeBoor(index,k-1,i,x) );
		}
	}

	Vec3 FeatureCurveComponentManager::computeCurveTangent(uint index, float u)
	{
		Vec3 p1 = calculateCurvePoint(index, std::min(1.0f, u + 0.02f));
		Vec3 p2 = calculateCurvePoint(index, std::max(0.0f, u - 0.02f));
		
		return glm::normalize(p1 - p2);
	}

	std::tuple<Vec3, Vec3> FeatureCurveComponentManager::computeCurveGradients(uint index, float u)
	{
		// find lower cp
		uint current_cp = 0;
		for (auto& cp : m_featureCurves[index].m_contraintPoints)
		{
			if (cp.m_curve_position > u) // there should always be a first mesh curve point
				break;

			current_cp++;
		}

		uint counter = 0;

		Vec3 b_rotation_axis = Vec3(1.0f);
		float b_rotation_angle = 1.0f;
		Vec3 f_rotation_axis = Vec3(1.0f);
		float f_rotation_angle = 1.0f;


		float curve_section_length = m_featureCurves[index].m_contraintPoints[current_cp].m_curve_position
										- m_featureCurves[index].m_contraintPoints[current_cp - 1].m_curve_position;

		float alpha = (u - m_featureCurves[index].m_contraintPoints[current_cp - 1].m_curve_position) / curve_section_length;

		// Transform gradients from surrounding cps to curve position, than interpolate
		Vec3 tangent_b = computeCurveTangent(index, m_featureCurves[index].m_contraintPoints[current_cp - 1].m_curve_position);
		Vec3 tangent_f = computeCurveTangent(index, m_featureCurves[index].m_contraintPoints[current_cp].m_curve_position);
		Vec3 tangent_c = computeCurveTangent(index, u);

		float angle_b = dot(tangent_b, tangent_c);
		Vec3 axis_b = glm::cross(tangent_b, tangent_c);

		float angle_f = dot(tangent_f, tangent_c);
		Vec3 axis_f = glm::cross(tangent_f, tangent_c);

		Vec3 gradient_b0 = m_featureCurves[index].m_contraintPoints[current_cp - 1].m_gradient_0;
		Vec3 gradient_b1 = m_featureCurves[index].m_contraintPoints[current_cp - 1].m_gradient_1;
		Vec3 gradient_f0 = m_featureCurves[index].m_contraintPoints[current_cp].m_gradient_0;
		Vec3 gradient_f1 = m_featureCurves[index].m_contraintPoints[current_cp].m_gradient_1;

		if (abs(angle_b) < 0.99)
		{
			b_rotation_axis = axis_b;
			b_rotation_angle = angle_b;
		}

		Quat rotation_b = glm::angleAxis(acos(b_rotation_angle), glm::normalize(b_rotation_axis));

		gradient_b0 = glm::normalize(glm::mat3(glm::toMat4(rotation_b)) * gradient_b0);
		gradient_b1 = glm::normalize(glm::mat3(glm::toMat4(rotation_b)) * gradient_b1);

		if (abs(angle_f) < 0.99)
		{
			f_rotation_axis = axis_f;
			f_rotation_angle = angle_f;
		}

		Quat rotation_f = glm::angleAxis(acos(f_rotation_angle), glm::normalize(f_rotation_axis));

		gradient_f1 = glm::normalize(glm::mat3(glm::toMat4(rotation_f)) * gradient_f1);
		gradient_f0 = glm::normalize(glm::mat3(glm::toMat4(rotation_f)) * gradient_f0);

		Vec3 gradient_0 = alpha * gradient_f0 + (1.0f - alpha) * gradient_b0;
		Vec3 gradient_1 = alpha * gradient_f1 + (1.0f - alpha) * gradient_b1;

		// project gradient into plane given by tangent
		gradient_0 = (gradient_0 - (glm::dot(tangent_c, gradient_0) * tangent_c));
		gradient_1 = (gradient_1 - (glm::dot(tangent_c, gradient_1) * tangent_c));

		return std::tuple<Vec3, Vec3>(gradient_0, gradient_1);
	}

	void FeatureCurveComponentManager::bufferMeshData(uint index)
	{
		//TODO use ResourceManager to manage SSBOs
		std::unique_lock<std::mutex>(m_dataAccess_mutex);

		if( m_featureCurves[index].m_mesh_vertices_ssbo == nullptr)
		{
			m_featureCurves[index].m_mesh_vertices_ssbo = new ShaderStorageBufferObject(m_featureCurves[index].m_mesh_vertices);
		}
		else
		{
			m_featureCurves[index].m_mesh_vertices_ssbo->reload(m_featureCurves[index].m_mesh_vertices);
			//delete m_featureCurves[index].m_mesh_vertices_ssbo;
			//m_featureCurves[index].m_mesh_vertices_ssbo = new ShaderStorageBufferObject(m_featureCurves[index].m_mesh_vertices);
		}

		if( m_featureCurves[index].m_mesh_indices_ssbo == nullptr)
		{
			m_featureCurves[index].m_mesh_indices_ssbo = new ShaderStorageBufferObject(m_featureCurves[index].m_mesh_indices);
		}
		else
		{
			m_featureCurves[index].m_mesh_indices_ssbo->reload(m_featureCurves[index].m_mesh_indices);
			//delete m_featureCurves[index].m_mesh_indices_ssbo;
			//m_featureCurves[index].m_mesh_indices_ssbo = new ShaderStorageBufferObject(m_featureCurves[index].m_mesh_indices);
		}
	}

	void FeatureCurveComponentManager::bakeSegementTextures(uint index)
	{

	}

	Vec3 FeatureCurveComponentManager::getLowerCorner(uint index)
	{
		return m_featureCurves[index].m_lower_corner;
	}

	Vec3 FeatureCurveComponentManager::getUpperCorner(uint index)
	{
		return m_featureCurves[index].m_upper_corner;
	}

	bool FeatureCurveComponentManager::isSurfaceSeed(uint index)
	{
		return m_featureCurves[index].m_is_surface_seed;
	}

	ShaderStorageBufferObject* FeatureCurveComponentManager::getVertexBuffer(uint index)
	{
		return m_featureCurves[index].m_mesh_vertices_ssbo;
	}

	ShaderStorageBufferObject* FeatureCurveComponentManager::getIndexBuffer(uint index)
	{
		return m_featureCurves[index].m_mesh_indices_ssbo;
	}

	std::vector<FeatureCurveComponentManager::FeatureCurveComponent::ControlVertex> FeatureCurveComponentManager::getControlVertices(Entity e) const
	{
		uint idx = getIndex(e);

		std::vector<FeatureCurveComponentManager::FeatureCurveComponent::ControlVertex> rtn = m_featureCurves[idx].m_controlVertices;

		return rtn;
	}

	std::vector<FeatureCurveComponentManager::FeatureCurveComponent::ConstraintPoint> FeatureCurveComponentManager::getConstraintPoints(Entity e) const
	{
		uint idx = getIndex(e);

		std::vector<FeatureCurveComponentManager::FeatureCurveComponent::ConstraintPoint> rtn = m_featureCurves[idx].m_contraintPoints;
		return rtn;
	}
	
	FeatureCurveComponentManager::FeatureCurveComponent::FeatureCurveComponent(Entity entity)
		: m_entity(entity), degree(1), m_mesh_vertices_ssbo(nullptr), m_mesh_indices_ssbo(nullptr), m_segement_textures(GEngineCore::resourceManager().getInvalidResourceID())
	{
	}

	FeatureCurveComponentManager::FeatureCurveComponent::~FeatureCurveComponent()
	{
	}


	////////////////////////////////////////
	// LandscapeBrickComponentManager
	////////////////////////////////////////

	LandscapeBrickComponentManager::LandscapeBrickComponentManager()
		: m_voxelize_featureCurves(true), m_voxelize_featureMeshes(true), m_voxelize_heightmaps(true)
	{
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this]
		{
			this->voxelize_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_gather_c.glsl"},"lcsp_voxelize").resource;
			this->voxelize_mesh_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelize_mesh_gather_c.glsl" }, "lcsp_voxelize_mesh").resource;
			this->voxelize_heightmapMesh_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelize_heightmapMesh_gather_c.glsl" }, "lcsp_voxelize_heightmap").resource;
			this->average_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_average_c.glsl" }, "lcsp_voxelize_average").resource;
			this->reset_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_reset_c.glsl" }, "lcsp_reset").resource;
			this->buildGuidance_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/buildGuidanceField_c.glsl" }, "lcsp_buildGuidance").resource;

			this->buildNoiseField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/buildNoiseField_c.glsl" }, "lcsp_buildNoiseField").resource;
			this->copyNoiseField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/copyTexture3D_RGBA16_c.glsl" }, "lcsp_copyNoiseField").resource;

			//this->surfacePropagation_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_pull_c.glsl" }).get();
			this->surfacePropagationInit_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_initBoundaryRegion_c.glsl" }, "lcsp_surfacePropagationInit").resource;
			this->surfacePropagation_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_distApprox_c.glsl" }, "lcsp_surfacePropagation").resource;
			this->copySurfaceField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/copyTexture3D_R16_c.glsl" }, "lcsp_copySurfaceField").resource;
			this->smooth_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/seperatedGaussian3d_c.glsl" }, "lcsp_smooth").resource;

			this->classifyVoxels_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_classify_c.glsl" }, "lcsp_classifyVoxels").resource;

			// Generate prgms for different datatypes on different levels by insertig define files
			this->buildHpLvl_R8_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, ("#define SRC_R8UI\n#define TGT_R8UI\n"), "lcsp_buildHpLvl_R8").resource;
			this->buildHpLvl_R8toR16_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R8UI\n#define TGT_R16UI\n", "lcsp_buildHpLvl_R8toR16").resource;
			this->buildHpLvl_R16_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R16UI\n#define TGT_R16UI\n", "lcsp_buildHpLvl_R16").resource;
			this->buildHpLvl_R16toR32_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R16UI\n#define TGT_R32UI\n", "lcsp_buildHpLvl_R16to32").resource;
			this->buildHpLvl_R32_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R32UI\n#define TGT_R32UI\n", "lcsp_buildHpLvl_R32").resource;

			this->generateTriangles_prgms = new GLSLProgram*[5];
			this->generateTriangles_prgms[0] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 6\n", "lcsp_generateTriangles_L6").resource;
			this->generateTriangles_prgms[1] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 7\n", "lcsp_generateTriangles_L7").resource;
			this->generateTriangles_prgms[2] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 8\n", "lcsp_generateTriangles_L8").resource;
			this->generateTriangles_prgms[3] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 9\n", "lcsp_generateTriangles_L9").resource;
			this->generateTriangles_prgms[4] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 10\n", "lcsp_generateTriangles_L10").resource;

			// Create surface nets programs

			this->updatePtexTilesDisplacement_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesDisplacement_c.glsl" }, "lscp_updatePtexTilesDisplacement").resource;
			this->updatePtexTilesTextures_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesTextures_c.glsl" }, "lscp_updatePtexTilesTextures").resource;

			this->surfaceNets_classify_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/surfaceNets_classify_c.glsl" },"surfaceNets_classify").resource;
			this->surfaceNets_generateQuads_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/surfaceNets_generateQuads_c.glsl" }, "surfaceNets_generateQuads").resource;
			this->computePtexNeighbours_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/computePtexNeighbours_c.glsl" }, "landscape_computePtexNeighbours").resource;
			this->computePatchDistances_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/computePatchDistances_c.glsl" }, "landscape_computePatchDistances").resource;
			this->updateTextureTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updateTextureTiles_c.glsl" }, "landscape_updateTextureTiles").resource;
			this->textureBaking_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/textureBaking_c.glsl" }, "landscape_textureBaking").resource;
			this->setInitialLODTextureTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/set_initial_LOD_textureTiles_c.glsl" }, "landscape_initialLODTextureTiles").resource;
			this->setPtexVistaTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/setPtexVistaTiles_c.glsl" }, "landscape_setPtexVistaTiles").resource;
			this->updatePtexTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTiles_c.glsl" }, "landscape_updatePtexTiles").resource;
			this->updatePtexTilesMipmaps_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesMipmaps_c.glsl" }, "landscape_updatePtexTilesMipmaps").resource;

			// create transform feedback program manually
			transformFeedback_terrainOutput_prgm = new GLSLProgram();
			transformFeedback_terrainOutput_prgm->init();

			std::string vertex_src = ResourceLoading::readShaderFile("../resources/shaders/dfr_landscapeSurface_v.glsl");
			std::string tessellationControl_src = ResourceLoading::readShaderFile("../resources/shaders/dfr_landscapeSurface_tc.glsl");
			std::string tessellationEvaluation_src = ResourceLoading::readShaderFile("../resources/shaders/dfr_landscapeSurface_te.glsl");
		
			transformFeedback_terrainOutput_prgm->bindAttribLocation(0, "vPosition");
			transformFeedback_terrainOutput_prgm->bindAttribLocation(1, "vNormal");

			if (!vertex_src.empty())
				if (!transformFeedback_terrainOutput_prgm->compileShaderFromString(&vertex_src, GL_VERTEX_SHADER)) { std::cout << transformFeedback_terrainOutput_prgm->getLog();}
			if (!tessellationControl_src.empty())
				if (!transformFeedback_terrainOutput_prgm->compileShaderFromString(&tessellationControl_src, GL_TESS_CONTROL_SHADER)) { std::cout << transformFeedback_terrainOutput_prgm->getLog();}
			if (!tessellationEvaluation_src.empty())
				if (!transformFeedback_terrainOutput_prgm->compileShaderFromString(&tessellationEvaluation_src, GL_TESS_EVALUATION_SHADER)) { std::cout << transformFeedback_terrainOutput_prgm->getLog();}
			
			// transform feedback varyings
			const char* varyings[2] = { "tf_position", "tf_normal" };
			glTransformFeedbackVaryings(transformFeedback_terrainOutput_prgm->getHandle(), 2, varyings, GL_INTERLEAVED_ATTRIBS);
			
			if (!transformFeedback_terrainOutput_prgm->link()) { std::cout << transformFeedback_terrainOutput_prgm->getLog();}

			// Generate buffer
			glGenBuffers(1, &transformFeedback_terrainBuffer);
		});
	}

	LandscapeBrickComponentManager::~LandscapeBrickComponentManager()
	{
	}

	void LandscapeBrickComponentManager::addComponent(Entity entity, Vec3 position, Vec3 dimension, uint res_x, uint res_y, uint res_z)
	{
		//m_bricks.push_back(LandscapeBrickComponent(entity,dimension,res_x,res_y,res_z));
		m_bricks.emplace_back(LandscapeBrickComponent(entity, dimension, res_x, res_y, res_z));
		GCoreComponents::transformManager().addComponent(entity, position);

		uint idx = static_cast<uint>(m_bricks.size()-1);

		m_index_map.insert(std::pair<uint,uint>(entity.id(),idx));

		// TODO Think about thread safety. Do I have to lock down the whole LandscapeBrickManager when calling methods from the render thread?
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask( [this,idx] { this->createGpuResources(idx); } );

		GEngineCore::renderingPipeline().addPerFrameComputeGpuTask([this, idx]()
		{
			//std::unique_lock<std::mutex> ptex_lock(m_ptex_update);

			std::unique_lock<std::mutex> ptex_lock(m_ptex_update, std::try_to_lock);
			if (ptex_lock.owns_lock())
				updateTextureTiles(idx);
		});
		
#if EDITOR_MODE // preprocessor definition

		std::vector<float> cv_interface_vertices({-dimension.x/2.0f,-dimension.y/2.0f,dimension.z/2.0f,0.0f,1.0f,0.0f,1.0f,
													dimension.x/2.0f,-dimension.y/2.0f,dimension.z/2.0f,0.0f,1.0f,0.0f,1.0f,
													dimension.x/2.0f,-dimension.y/2.0f,-dimension.z/2.0f,0.0f,1.0f,0.0f,1.0f,
													-dimension.x/2.0f,-dimension.y/2.0f,-dimension.z/2.0f,0.0f,1.0f,0.0f,1.0f,
													-dimension.x/2.0f,dimension.y/2.0f,dimension.z/2.0f,0.0f,1.0f,0.0f,1.0f,
													dimension.x/2.0f,dimension.y/2.0f,dimension.z/2.0f,0.0f,1.0f,0.0f,1.0f,
													dimension.x/2.0f,dimension.y/2.0f,-dimension.z/2.0f,0.0f,1.0f,0.0f,1.0f,
													-dimension.x/2.0f,dimension.y/2.0f,-dimension.z/2.0f,0.0f,1.0f,0.0f,1.0});
		std::vector<uint32_t> cv_interface_indices({0,1, 0,3, 0,4, 1,2, 1,5, 2,3, 2,6, 3,7, 4,5, 4,7, 5,6, 6,7});
		
		VertexLayout vertex_description(28,{VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
											VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat)*3)});

		GRenderingComponents::interfaceMeshManager().addComponent(entity,"brick_"+std::to_string(entity.id()),"../resources/materials/editor/interface_cv.slmtl",cv_interface_vertices,cv_interface_indices,vertex_description,GL_LINES);

		// TODO add selectable component
#endif

	}

	void LandscapeBrickComponentManager::deleteComponent(uint index)
	{

	}

	uint LandscapeBrickComponentManager::getIndex(Entity entity)
	{
		auto search = m_index_map.find(entity.id());

		assert( (search != m_index_map.end()) );

		return search->second;
	}

	void LandscapeBrickComponentManager::addFeatureCurve(uint brick_index, Entity feature_curve_entity)
	{
		m_bricks[brick_index].m_featureCurves.push_back(feature_curve_entity);
	}

	void LandscapeBrickComponentManager::addFeatureMesh(uint brick_index, Entity feature_mesh_entity)
	{
		m_bricks[brick_index].m_featureMeshes.push_back(feature_mesh_entity);
	}

	void LandscapeBrickComponentManager::addHeightmapMesh(uint brick_index, Entity heightmap_entity)
	{
		m_bricks[brick_index].m_heightmaps.push_back(heightmap_entity);
	}

	void LandscapeBrickComponentManager::clearFeatureCurves(uint brick_index)
	{
		m_bricks[brick_index].m_featureCurves.clear();
	}

	bool LandscapeBrickComponentManager::isEmpty(uint brick_index)
	{
		return (m_bricks[brick_index].m_featureCurves.size() == 0) ? true : false;
	}

	void LandscapeBrickComponentManager::setNeighbour(uint index, NeighbourDirection direction, Entity neighbour)
	{
		switch (direction)
		{
		case Landscape::LandscapeBrickComponentManager::EAST:
			m_bricks[index].m_eastern_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
			m_bricks[index].m_eastern_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
			m_bricks[index].m_eastern_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
			m_bricks[index].m_eastern_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
			m_bricks[index].m_east_neighbour = neighbour;
			break;
		case Landscape::LandscapeBrickComponentManager::WEST:
			m_bricks[index].m_western_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
			m_bricks[index].m_western_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
			m_bricks[index].m_western_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
			m_bricks[index].m_western_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
			m_bricks[index].m_west_neighbour = neighbour;
			break;
		case Landscape::LandscapeBrickComponentManager::DOWN:
			m_bricks[index].m_lower_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
			m_bricks[index].m_lower_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
			m_bricks[index].m_lower_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
			m_bricks[index].m_lower_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
			m_bricks[index].m_lower_neighbour = neighbour;
			break;
		case Landscape::LandscapeBrickComponentManager::UP:
			m_bricks[index].m_upper_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
			m_bricks[index].m_upper_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
			m_bricks[index].m_upper_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
			m_bricks[index].m_upper_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
			m_bricks[index].m_upper_neighbour = neighbour;
			break;
		case Landscape::LandscapeBrickComponentManager::SOUTH:
			m_bricks[index].m_southern_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
			m_bricks[index].m_southern_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
			m_bricks[index].m_southern_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
			m_bricks[index].m_southern_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
			m_bricks[index].m_south_neighbour = neighbour;
			break;
		case Landscape::LandscapeBrickComponentManager::NORTH:
			m_bricks[index].m_northern_boundary[0] = m_bricks[getIndex(neighbour)].m_normals;
			m_bricks[index].m_northern_boundary[1] = m_bricks[getIndex(neighbour)].m_gradients;
			m_bricks[index].m_northern_boundary[2] = m_bricks[getIndex(neighbour)].m_noise_params;
			m_bricks[index].m_northern_boundary[3] = m_bricks[getIndex(neighbour)].m_surface;
			m_bricks[index].m_north_neighbour = neighbour;
			break;
		default:
			break;
		}
	}

	Vec3 LandscapeBrickComponentManager::getDimension(uint index)
	{
		return m_bricks[index].m_dimensions;
	}

	Vec3 LandscapeBrickComponentManager::getResolution(uint index)
	{
		return Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z);
	}

	void LandscapeBrickComponentManager::setResolution(uint index, uint resX, uint resY, uint resZ)
	{
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index, resX, resY, resZ] {	m_bricks[index].m_res_x = resX;
																											m_bricks[index].m_res_y = resY;
																											m_bricks[index].m_res_z = resZ; 
																											this->updateGpuResources(index); });
	}
	
	void LandscapeBrickComponentManager::updateBrick(uint index)
	{
		//while (!m_bricks[index].m_ptex_ready);
		//m_bricks[index].m_ptex_ready = false;
		//m_bricks[index].m_cancel_ptex_update = true;

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->resetFields(index); });
		if(m_voxelize_featureCurves)
			GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeFeatureCurves(index); });
		if(m_voxelize_featureMeshes)
			GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeFeatureMeshes(index); });
		if(m_voxelize_heightmaps)
			GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeHeightmapMeshes(index); });
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->averageVoxelization(index); });

		
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeGuidanceField(index); });
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeNoiseField(index); });
		
		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->expandFeatureCurves(index); });
		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeFeatureCurves(index); });
		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeGuidanceField(index); });

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeSurfacePropagation(index); });
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->smoothSurfaceField(index); });
		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeSurfaceMesh(index); } );
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeNaiveSurfaceNetsMesh(index); });
		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->bakeSurfaceTextures(index); });

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->updateDebugVolume(index); });

		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask( [this,index] { 
		//	m_bricks[index].m_ptex_ready = true;
		//	m_bricks[index].m_cancel_ptex_update = false;
		//} );
	}

	void LandscapeBrickComponentManager::updateNoiseAndMaterial(uint index)
	{
		//TODO
	}

	void LandscapeBrickComponentManager::updateCorrespondingBricks(Entity modified_feature_curve)
	{
		//TODO write a more elegant version (currently brute force)

		uint brick_idx = 0;
		for (auto& brick : m_bricks)
		{
			for (auto& feature_curve : brick.m_featureCurves)
			{
				if (feature_curve == modified_feature_curve)
				{
					updateBrick(brick_idx);
				}
			}
			
			++brick_idx;
		}
	}

	void LandscapeBrickComponentManager::createGpuResources(uint index)
	{
		//std::cout<<"Creating brick gpu resources."<<std::endl;

		if (m_bricks[index].m_head == nullptr)
		{
			m_bricks[index].m_head = GEngineCore::resourceManager().createSSBO("brick_"+std::to_string(index) +"head",(m_bricks[index].m_res_x * m_bricks[index].m_res_y * m_bricks[index].m_res_z) * 4 * 2, nullptr).resource;
		}
		if (m_bricks[index].m_guidancefield_data == nullptr)
		{
			m_bricks[index].m_guidancefield_data = GEngineCore::resourceManager().createSSBO("brick_" + std::to_string(index) + "guidancefield_data", (m_bricks[index].m_res_x * m_bricks[index].m_res_y * m_bricks[index].m_res_z) * 4 * 20 * 6, nullptr).resource;
		}

		
		TextureLayout rgba32f_layout(GL_RGBA32F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_FLOAT, 1,
			{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
				std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

		// Create normal field resources
		if( m_bricks[index].m_normals == nullptr)
		{
			m_bricks[index].m_normals = GEngineCore::resourceManager().createTexture3D("brick_"+std::to_string(index)+"_normals",rgba32f_layout,nullptr).resource;
		}

		if( m_bricks[index].m_normals_backbuffer == nullptr)
		{
			m_bricks[index].m_normals_backbuffer = GEngineCore::resourceManager().createTexture3D("brick_"+std::to_string(index)+"_normals_backbuffer",rgba32f_layout,nullptr).resource;
		}

		// Create gradient field resources
		if (m_bricks[index].m_gradients == nullptr)
		{
			m_bricks[index].m_gradients = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_gradients",rgba32f_layout, nullptr).resource;
		}

		if (m_bricks[index].m_gradients_backbuffer == nullptr)
		{
			m_bricks[index].m_gradients_backbuffer = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_gradients_backbuffer",rgba32f_layout, nullptr).resource;
		}

		// Create atomic counter
		glGenBuffers(1, &(m_bricks[index].m_counter_buffer) );
		GLuint zero = 0;
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, m_bricks[index].m_counter_buffer);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, 0);


		TextureLayout r16f_layout(GL_R16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RED, GL_HALF_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

		// Create surface field
		if( m_bricks[index].m_surface == nullptr)
		{
			m_bricks[index].m_surface = GEngineCore::resourceManager().createTexture3D("brick_"+std::to_string(index)+"_surface", r16f_layout,nullptr).resource;
		}

		if( m_bricks[index].m_surface_backbuffer == nullptr)
		{
			m_bricks[index].m_surface_backbuffer = GEngineCore::resourceManager().createTexture3D("brick_"+std::to_string(index)+"_surface_backbuffer", r16f_layout,nullptr).resource;
		}


		TextureLayout r8ui_layout(GL_R8UI, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

		if (m_bricks[index].m_surface_boundaryRegion == nullptr)
		{
			m_bricks[index].m_surface_boundaryRegion = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_surface_boundaryRegion", r8ui_layout, nullptr).resource;
		}



		TextureLayout rgba16f_layout(GL_RGBA16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_HALF_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

		// Create noise parameters field
		if(m_bricks[index].m_noise_params == nullptr)
		{
			m_bricks[index].m_noise_params = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_noise_params", rgba16f_layout, nullptr).resource;
		}


		// Dummy surface mesh
		static float vertices[27] = {-10.0,0.0,-10.0, 0.0, 0.0,1.0,0.0, 0.0,0.0,
										10.0,0.0,-10.0, 0.0, 0.0,1.0,0.0, 0.0,0.0,
										0.0,0.0,10.0, 0.0, 0.0,1.0,0.0, 0.0,0.0};
		std::vector<uint8_t> surface_vertices( reinterpret_cast<uint8_t*>(vertices), reinterpret_cast<uint8_t*>(vertices) + (24*4) );
		std::vector<uint32_t> surface_indices( {0,1,2} );
		//VertexLayout vertex_desc(8 * 4, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 3),VertexLayout::Attribute(GL_FLOAT,2,GL_FALSE,sizeof(GLfloat) * 6) });
		VertexLayout vertex_desc(9 * 4, { VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 4), VertexLayout::Attribute(GL_FLOAT, 1, GL_FALSE, sizeof(GLfloat) * 8) });
		m_bricks[index].m_surface_mesh = GEngineCore::resourceManager().createMesh("brick"+std::to_string(index)+"_surface_mesh",surface_vertices,surface_indices,vertex_desc,GL_PATCHES).resource;

		// Create landscape surface material
		//MaterialInfo surface_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_rocky_desert.slmtl");
		//m_bricks[index].m_surface_material = GEngineCore::resourceManager().createMaterial("../resources/materials/landscape_rocky_desert.slmtl",surface_material_info.shader_filepaths,surface_material_info.texture_filepaths).get();
		MaterialInfo surface_material_info = ResourceLoading::parseMaterial("../resources/materials/surfaceNets_landscape_coastline.slmtl");
		m_bricks[index].m_surface_material = GEngineCore::resourceManager().createMaterial("../resources/materials/surfaceNets_landscape_coastline.slmtl",surface_material_info.shader_filepaths,surface_material_info.texture_filepaths).resource;

		//GRenderingComponents::staticMeshManager().addComponent(m_bricks[index].m_entity, m_bricks[index].m_surface_mesh, m_bricks[index].m_surface_material);

		// Create shadow caster for landscape
		MaterialInfo shadowCast_material_info;
		shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/dfr_landscapeSurface_v.glsl");
		shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/dfr_landscapeSurface_tc.glsl");
		shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/landscape_shadowCaster_te.glsl");
		shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/pointlight_shadowMap_g.glsl");
		shadowCast_material_info.shader_filepaths.push_back("../resources/shaders/pointlight_shadowMap_f.glsl");
		m_bricks[index].m_shadowCaster_material = GEngineCore::resourceManager().createMaterial("landscape_shadowCaster", shadowCast_material_info.shader_filepaths, shadowCast_material_info.texture_filepaths).resource;
		//GRenderingComponents::shadowCastMeshManager().addComponent(m_bricks[index].m_entity, m_bricks[index].m_surface_mesh, m_bricks[index].m_shadowCaster_material);

		// Ptex testing
		VertexLayout ptex_vertex_layout(6 * 4, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 3) });
		m_bricks[index].m_ptex_mesh = GEngineCore::resourceManager().createMeshAsync("brick" + std::to_string(index) + "_ptex_mesh", {}, {}, ptex_vertex_layout, GL_PATCHES);
		MaterialInfo surface_ptex_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_ptex.slmtl");
		m_bricks[index].m_ptex_material = GEngineCore::resourceManager().createMaterialAsync("landscape_ptex_material", surface_ptex_material_info.shader_filepaths, surface_ptex_material_info.texture_filepaths );
		m_bricks[index].m_ptex_parameters = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_parameters",0,nullptr);
		m_bricks[index].m_ptex_parameters_backbuffer = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_parameters_backbuffer", 0, nullptr);
		m_bricks[index].m_ptex_bindless_texture_handles = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_bindless_texuture_handles", 0, nullptr);
		m_bricks[index].m_ptex_bindless_image_handles = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_bindless_image_handles", 0, nullptr);
		m_bricks[index].m_ptex_bindless_mipmap_image_handles = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_bindless_mipmap_image_handles", 0, nullptr);
		m_bricks[index].m_ptex_material_bth = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_material_bth", 0, nullptr);

		m_bricks[index].m_ptex_tiles_per_edge = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_tiles_per_edge", 0, nullptr);

		m_bricks[index].m_ptex_patch_distances_SSBO = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_patch_distances", 0, nullptr);
		m_bricks[index].m_ptex_availableTiles_SSBO = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_availableTiles", 0, nullptr);

		GRenderingComponents::ptexMeshManager().addComponent(m_bricks[index].m_entity, m_bricks[index].m_ptex_mesh, m_bricks[index].m_ptex_material, m_bricks[index].m_ptex_parameters, m_bricks[index].m_ptex_bindless_texture_handles,true);


		// create update/free SSBOs
		m_bricks[index].m_ptex_updatePatches_tgt_SSBO = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_updatePatches", 0, nullptr);
		m_bricks[index].m_ptex_updatePatches_src_SSBO = GEngineCore::resourceManager().createSSBOAsync("brick_" + std::to_string(index) + "_ptex_freeSlots", 0, nullptr);


		addDebugVolume(index);
	}

	void LandscapeBrickComponentManager::updateGpuResources(uint index)
	{
		m_bricks[index].m_head->reload((m_bricks[index].m_res_x * m_bricks[index].m_res_y * m_bricks[index].m_res_z) * 4 * 2, 0, 0);
		m_bricks[index].m_guidancefield_data->reload((m_bricks[index].m_res_x * m_bricks[index].m_res_y * m_bricks[index].m_res_z) * 4 * 8 * 5, 0, 0);
		
		TextureLayout rgba32f_layout(GL_RGBA32F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});
		
		m_bricks[index].m_normals->reload(rgba32f_layout,nullptr);
		m_bricks[index].m_normals_backbuffer->reload(rgba32f_layout,nullptr);

		m_bricks[index].m_gradients->reload(rgba32f_layout, nullptr);
		m_bricks[index].m_gradients_backbuffer->reload(rgba32f_layout, nullptr);
		
		TextureLayout r16f_layout(GL_R16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RED, GL_HALF_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

		m_bricks[index].m_surface->reload(r16f_layout,nullptr);
		m_bricks[index].m_surface_backbuffer->reload(r16f_layout,nullptr);
		
		TextureLayout rgba16f_layout(GL_RGBA16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_HALF_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

		m_bricks[index].m_noise_params->reload(rgba16f_layout,nullptr);
	}

	void LandscapeBrickComponentManager::resetFields(uint index)
	{
		// Reset head buffer and surface texture
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		reset_prgm->use();

		reset_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));
		reset_prgm->setUniform("cell_size", Vec3(m_bricks[index].m_dimensions.x / m_bricks[index].m_res_x,
			m_bricks[index].m_dimensions.y / m_bricks[index].m_res_y,
			m_bricks[index].m_dimensions.z / m_bricks[index].m_res_z));

		m_bricks[index].m_head->bind(0);

		m_bricks[index].m_surface->bindImage(0, GL_WRITE_ONLY);
		reset_prgm->setUniform("surface_tx3D", 0);
		m_bricks[index].m_surface_backbuffer->bindImage(1, GL_WRITE_ONLY);
		reset_prgm->setUniform("surface_backbuffer_tx3D", 1);
		m_bricks[index].m_noise_params->bindImage(2, GL_WRITE_ONLY);
		reset_prgm->setUniform("noise_tx3D",2);

		reset_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x/4)) +1,
										static_cast<uint>(std::floor(m_bricks[index].m_res_y/2)) +1,
										static_cast<uint>(std::floor(m_bricks[index].m_res_z/4)) +1);

		//sm_bricks[index].m_gradients->bindTexture();
		glClearTexImage(m_bricks[index].m_gradients->getName(), 0, GL_RGBA, GL_FLOAT, NULL);

		//glClearTexImage(m_bricks[index].m_noise_params->getName(), 0, GL_RGBA, GL_HALF_FLOAT, NULL);

		/*	Reset the atomic counter */
		GLuint zero = 0;
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 7, m_bricks[index].m_counter_buffer);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 7, 0);

		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Reset fields - " <<(t_1 - t_0) / 1000000.0 << "ms" << std::endl;
	}

	void LandscapeBrickComponentManager::expandFeatureCurves(uint index)
	{
		// For now, fetch texture data from GPU and expand feature curves on the CPU

		m_bricks[index].m_normals->bindTexture();
		std::vector<glm::vec4> normal_data(m_bricks[index].m_normals->getWidth() * m_bricks[index].m_normals->getHeight() * m_bricks[index].m_normals->getDepth());
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, normal_data.data());

		m_bricks[index].m_gradients->bindTexture();
		std::vector<glm::vec4> gradient_data(m_bricks[index].m_gradients->getWidth() * m_bricks[index].m_gradients->getHeight() * m_bricks[index].m_gradients->getDepth());
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, gradient_data.data());

		m_bricks[index].m_surface->bindTexture();
		std::vector<GLubyte> surface_data(m_bricks[index].m_surface->getWidth() * m_bricks[index].m_surface->getHeight() * m_bricks[index].m_surface->getDepth());
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_UNSIGNED_BYTE, surface_data.data());

		for (auto& feature_curve : m_bricks[index].m_featureCurves)
		{
			//TODO scale relative to brick size

			uint curve_idx = GLandscapeComponents::featureCurveManager().getIndex(feature_curve);

			Vec3 end_point = GLandscapeComponents::featureCurveManager().calculateCurvePoint(curve_idx, 1.0);

			float distance = 0.0;

			for (int i = 0; i < 200; i++)
			{
				// get position in world space
				Vec3 point = Vec3(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(feature_curve)) * glm::vec4(end_point, 1.0));
				// get brick origin in world space
				Vec3 brick_origin = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity)) - m_bricks[index].m_dimensions / 2.0f;
				// compute position in brick volume space
				point = (point - brick_origin) / m_bricks[index].m_dimensions;

				if (!((point.x >= 0.0 && point.x <= 1.0) &&
					(point.y >= 0.0 && point.y <= 1.0) &&
					(point.z >= 0.0 && point.z <= 1.0)))
				{
					break;
				}

				// Check if near to another feature curve (naive)
				//	for (auto& fc : m_bricks[index].m_featureCurves)
				//	{
				//		uint fc_idx = GLandscapeComponents::featureCurveManager().getIndex(fc);
				//		GLandscapeComponents::featureCurveManager().
				//		for(auto& cv : fc->)
				//	}


				point *= Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z);

				// compute normal data access index
				uint normal_data_idx = (uint)point.x + ((uint)point.y * m_bricks[index].m_res_x) + ((uint)point.z * m_bricks[index].m_res_x * m_bricks[index].m_res_y);

				// compute direction based on vector field using trilinear interpolation
				
				uint x_offset = 1;
				uint y_offset = m_bricks[index].m_res_x;
				uint z_offset = (m_bricks[index].m_res_x * m_bricks[index].m_res_y);

				uint c000_idx = (uint)point.x + ((uint)point.y * m_bricks[index].m_res_x) + ((uint)point.z * m_bricks[index].m_res_x * m_bricks[index].m_res_y);
				uint c001_idx = c000_idx + z_offset;
				uint c010_idx = c000_idx + y_offset;
				uint c011_idx = c000_idx + y_offset + z_offset;
				uint c100_idx = c000_idx + x_offset;
				uint c101_idx = c000_idx + x_offset + z_offset;
				uint c110_idx = c000_idx + x_offset + y_offset;
				uint c111_idx = c000_idx + x_offset + y_offset + z_offset;

				Vec3 n00 = Vec3(normal_data[c000_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(normal_data[c100_idx]) * (point.x - int(point.x));
				Vec3 n01 = Vec3(normal_data[c001_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(normal_data[c101_idx]) * (point.x - int(point.x));
				Vec3 n10 = Vec3(normal_data[c010_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(normal_data[c110_idx]) * (point.x - int(point.x));
				Vec3 n11 = Vec3(normal_data[c011_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(normal_data[c111_idx]) * (point.x - int(point.x));
				Vec3 n0 = n00 * (1.0f - (point.y - int(point.y))) + n10 * (point.y - int(point.y));
				Vec3 n1 = n01 * (1.0f - (point.y - int(point.y))) + n11 * (point.y - int(point.y));
				Vec3 normal = glm::normalize(n0 * (1.0f - (point.z - int(point.z))) + n1 * (point.y - int(point.z)));

				Vec3 g00 = Vec3(gradient_data[c000_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(gradient_data[c100_idx]) * (point.x - int(point.x));
				Vec3 g01 = Vec3(gradient_data[c001_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(gradient_data[c101_idx]) * (point.x - int(point.x));
				Vec3 g10 = Vec3(gradient_data[c010_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(gradient_data[c110_idx]) * (point.x - int(point.x));
				Vec3 g11 = Vec3(gradient_data[c011_idx]) * (1.0f - (point.x - int(point.x))) + Vec3(gradient_data[c111_idx]) * (point.x - int(point.x));
				Vec3 g0 = g00 * (1.0f - (point.y - int(point.y))) + g10 * (point.y - int(point.y));
				Vec3 g1 = g01 * (1.0f - (point.y - int(point.y))) + g11 * (point.y - int(point.y));
				Vec3 gradient = glm::normalize( g0 * (1.0f - (point.z - int(point.z))) + g1 * (point.y - int(point.z)) );


				Vec3 tangent = 3.0f * glm::normalize(end_point - GLandscapeComponents::featureCurveManager().calculateCurvePoint(curve_idx, 0.95f));

				//Vec3 dir = glm::normalize(glm::cross(normal, tangent));
				//dir = glm::normalize(glm::cross(dir, normal)) * 0.1f;

				Vec3 dir = glm::normalize(glm::cross(normal, gradient)) * 0.1f;

				// make sure to follow the guidance field in the direction of the tangent
				float sign = (glm::dot(dir, tangent) < 0.0f) ? -1.0f : 1.0f;

				dir *= sign;

				distance += glm::length(dir);

				//TODO break condition
				if (distance > 5.0)
				{
					GLandscapeComponents::featureCurveManager().addControlVertex(curve_idx, end_point + dir);
					distance = 0.0;
				}

				end_point += dir;


#if EDITOR_MODE // preprocessor definition

				Entity normals = GEngineCore::entityManager().create();
				GCoreComponents::transformManager().addComponent(normals, Vec3(0.0));

				Vec3 v0 = Vec3(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(feature_curve)) * glm::vec4(end_point, 1.0));;
				Vec3 v1 = v0 + gradient;
				std::vector<float> cv_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
															v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0 });
				std::vector<uint> cv_interface_indices({0,1});

				VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
					VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });

				GRenderingComponents::interfaceMeshManager().addComponent(normals, "brick_" + std::to_string(normals.id()), "../resources/materials/editor/interface_cv.slmtl", cv_interface_vertices, cv_interface_indices, vertex_description, GL_LINES);

				// TODO add selectable component
#endif
			}
		}

		// continue pipeline
		//	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->voxelizeFeatureCurves(index); });
		//	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeGuidanceField(index); });
		
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask( [this,index] { this->computeSurfacePropagation(index); } );
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask( [this,index] { this->computeSurfaceMesh(index); } );

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask( [this,index] { this->addDebugVolume(index); } );
	}
	
	void LandscapeBrickComponentManager::voxelizeFeatureCurves(uint index)
	{
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);
		
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		
		// Voxelize geometry
		voxelize_prgm->use();
	
		m_bricks[index].m_head->bind(2);
		m_bricks[index].m_guidancefield_data->bind(3);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, m_bricks[index].m_counter_buffer);

		m_bricks[index].m_surface->bindImage(0, GL_WRITE_ONLY);
		voxelize_prgm->setUniform("surface_tx3D",0);
		m_bricks[index].m_surface_backbuffer->bindImage(1, GL_WRITE_ONLY);
		voxelize_prgm->setUniform("surface_backbuffer_tx3D",1);

		m_bricks[index].m_noise_params->bindImage(2, GL_WRITE_ONLY);
		voxelize_prgm->setUniform("noise_tx3D", 2);
	
		// Compute voxel transform matrix
		Vec3 brick_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity));
		Vec3 brick_dimension = m_bricks[index].m_dimensions;
		//	Mat4x4 voxel_matrix = glm::ortho(brick_position.x - brick_dimension.x/2.0f,
		//										brick_position.x + brick_dimension.x/2.0f,
		//										brick_position.y - brick_dimension.y/2.0f,
		//										brick_position.y + brick_dimension.y/2.0f,
		//										brick_position.z + brick_dimension.z/2.0f,
		//										brick_position.z - brick_dimension.z/2.0f);

		//voxel_matrix = glm::translate(Mat4x4(), -1.0f * Vec3( brick_position.x, brick_position.y, brick_position.z));
		//voxel_matrix = glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity)));
		Mat4x4 voxel_matrix = glm::scale(Mat4x4(), Vec3(1.0/brick_dimension.x, 1.0f/brick_dimension.y, 1.0f/brick_dimension.z));
		voxel_matrix = voxel_matrix * glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity)));
		voxelize_prgm->setUniform("voxel_matrix", voxel_matrix);

		voxelize_prgm->setUniform("grid_size",Vec3(m_bricks[index].m_res_x,m_bricks[index].m_res_y,m_bricks[index].m_res_z));

		voxelize_prgm->setUniform("cell_size", Vec3(m_bricks[index].m_dimensions.x / m_bricks[index].m_res_x,
			m_bricks[index].m_dimensions.y / m_bricks[index].m_res_y,
			m_bricks[index].m_dimensions.z / m_bricks[index].m_res_z));

		std::cout << "Feature Curves in Brick: " << m_bricks[index].m_featureCurves.size() << std::endl;
		
		for(auto& featureCurve : m_bricks[index].m_featureCurves)
		{
			uint featureCurve_idx = GLandscapeComponents::featureCurveManager().getIndex(featureCurve);
	
			// get feature curve model matrix
			auto model_matrix = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(featureCurve));
			voxelize_prgm->setUniform("model_matrix",model_matrix);

			auto normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
			voxelize_prgm->setUniform("normal_matrix",normal_matrix);
	
			auto vertex_ssbo = GLandscapeComponents::featureCurveManager().getVertexBuffer(featureCurve_idx);
			auto index_ssbo = GLandscapeComponents::featureCurveManager().getIndexBuffer(featureCurve_idx);

			if (vertex_ssbo == nullptr || index_ssbo == nullptr)
				continue;

			voxelize_prgm->setUniform("num_triangles", (index_ssbo->getSize() / 12) ); // ssbo size is in bytes, therefore /4 and /3 to get triangle count
			voxelize_prgm->setUniform("is_surface_seed", GLandscapeComponents::featureCurveManager().isSurfaceSeed(featureCurve_idx));
	
			vertex_ssbo->bind(0);
			index_ssbo->bind(1);

			voxelize_prgm->dispatchCompute(static_cast<uint>(std::ceil( static_cast<float>(index_ssbo->getSize()/ 12) / 32.0)) , 1, 1);

			std::cout << "Triangles: " << index_ssbo->getSize() / 12 << std::endl;
	
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		}


		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Voxelize Feature Curves - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

		/*
		Entity vox_vol = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(vox_vol, Vec3(-128.0,0.0,0.0), Quat(), Vec3(64, 32, 64));
		std::shared_ptr<Mesh> bb = GEngineCore::resourceManager().createBox();

		std::shared_ptr<Texture3D> out_vol = GEngineCore::resourceManager().createTexture3D("brick_" + std::to_string(index) + "_normals",
																						GL_RGBA32F,
																						m_bricks[index].m_res_x,
																						m_bricks[index].m_res_y,
																						m_bricks[index].m_res_z,
																						GL_RGBA, GL_FLOAT, nullptr);
		
		GRenderingComponents::volumeManager().addComponent(vox_vol, out_vol, bb, Vec3(), Vec3(64.0,32.0,64.0));
		*/
	}

	void LandscapeBrickComponentManager::voxelizeFeatureMeshes(uint index)
	{
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);


		// Voxelize geometry
		voxelize_mesh_prgm->use();

		m_bricks[index].m_head->bind(2);
		m_bricks[index].m_guidancefield_data->bind(3);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, m_bricks[index].m_counter_buffer);

		m_bricks[index].m_surface->bindImage(0, GL_WRITE_ONLY);
		voxelize_mesh_prgm->setUniform("surface_tx3D", 0);
		m_bricks[index].m_surface_backbuffer->bindImage(1, GL_WRITE_ONLY);
		voxelize_mesh_prgm->setUniform("surface_backbuffer_tx3D", 1);

		m_bricks[index].m_noise_params->bindImage(2, GL_READ_WRITE);
		voxelize_mesh_prgm->setUniform("noise_tx3D", 2);

		// Compute voxel transform matrix
		Vec3 brick_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity));
		Vec3 brick_dimension = m_bricks[index].m_dimensions;

		Mat4x4 voxel_matrix = glm::scale(Mat4x4(), Vec3(1.0 / brick_dimension.x, 1.0f / brick_dimension.y, 1.0f / brick_dimension.z));
		voxel_matrix = voxel_matrix * glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity)));
		voxelize_mesh_prgm->setUniform("voxel_matrix", voxel_matrix);

		voxelize_mesh_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x,m_bricks[index].m_res_y,m_bricks[index].m_res_z));

		voxelize_mesh_prgm->setUniform("cell_size", Vec3(m_bricks[index].m_dimensions.x / m_bricks[index].m_res_x,
			m_bricks[index].m_dimensions.y / m_bricks[index].m_res_y,
			m_bricks[index].m_dimensions.z / m_bricks[index].m_res_z));

		std::cout << "Feature Meshes in Brick: " << m_bricks[index].m_featureMeshes.size() << std::endl;

		for (auto& featureMesh : m_bricks[index].m_featureMeshes)
		{
			uint featureMesh_idx = GLandscapeComponents::featureMeshManager().getIndex(featureMesh);

			// get feature curve model matrix
			auto model_matrix = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(featureMesh));
			voxelize_mesh_prgm->setUniform("model_matrix", model_matrix);

			auto normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
			voxelize_mesh_prgm->setUniform("normal_matrix", normal_matrix);

			auto mesh = GLandscapeComponents::featureMeshManager().getMesh(featureMesh_idx);

			if (mesh == nullptr)
				continue;

			voxelize_mesh_prgm->setUniform("num_triangles", (mesh->getIndicesCount() / 3));
			voxelize_mesh_prgm->setUniform("is_surface_seed", true);

			// Bind vertex and index buffer as storage buffer
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->getVboHandle());
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->getIboHandle());
			mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
			mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);

			voxelize_mesh_prgm->dispatchCompute(static_cast<uint>(std::ceil(static_cast<float>((mesh->getIndicesCount() / 3) / 32.0))), 1, 1);

			std::cout << "Triangles: " << (mesh->getIndicesCount() / 3) << std::endl;

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		}


		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Voxelize Feature Mesh - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
	}

	void LandscapeBrickComponentManager::voxelizeHeightmapMeshes(uint index)
	{
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);


		// Voxelize geometry
		voxelize_heightmapMesh_prgm->use();

		m_bricks[index].m_head->bind(2);
		m_bricks[index].m_guidancefield_data->bind(3);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, m_bricks[index].m_counter_buffer);

		m_bricks[index].m_surface->bindImage(0, GL_WRITE_ONLY);
		voxelize_heightmapMesh_prgm->setUniform("surface_tx3D", 0);
		m_bricks[index].m_surface_backbuffer->bindImage(1, GL_WRITE_ONLY);
		voxelize_heightmapMesh_prgm->setUniform("surface_backbuffer_tx3D", 1);

		m_bricks[index].m_noise_params->bindImage(2, GL_READ_WRITE);
		voxelize_heightmapMesh_prgm->setUniform("noise_tx3D", 2);

		// Compute voxel transform matrix
		Vec3 brick_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity));
		Vec3 brick_dimension = m_bricks[index].m_dimensions;

		Mat4x4 voxel_matrix = glm::scale(Mat4x4(), Vec3(1.0 / brick_dimension.x, 1.0f / brick_dimension.y, 1.0f / brick_dimension.z));
		voxel_matrix = voxel_matrix * glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity)));
		voxelize_heightmapMesh_prgm->setUniform("voxel_matrix", voxel_matrix);

		voxelize_heightmapMesh_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));

		voxelize_heightmapMesh_prgm->setUniform("cell_size", Vec3(m_bricks[index].m_dimensions.x / m_bricks[index].m_res_x,
			m_bricks[index].m_dimensions.y / m_bricks[index].m_res_y,
			m_bricks[index].m_dimensions.z / m_bricks[index].m_res_z));

		std::cout << "Feature Meshes in Brick: " << m_bricks[index].m_heightmaps.size() << std::endl;

		for (auto& heightmap : m_bricks[index].m_heightmaps)
		{
			uint featureMesh_idx = GLandscapeComponents::featureMeshManager().getIndex(heightmap);

			// get feature curve model matrix
			auto model_matrix = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(heightmap));
			voxelize_heightmapMesh_prgm->setUniform("model_matrix", model_matrix);

			auto normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
			voxelize_heightmapMesh_prgm->setUniform("normal_matrix", normal_matrix);

			auto mesh = GLandscapeComponents::featureMeshManager().getMesh(featureMesh_idx);

			if (mesh == nullptr)
				continue;

			voxelize_heightmapMesh_prgm->setUniform("num_triangles", (mesh->getIndicesCount() / 3));
			voxelize_heightmapMesh_prgm->setUniform("is_surface_seed", true);

			// Bind vertex and index buffer as storage buffer
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->getVboHandle());
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->getIboHandle());
			mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
			mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);

			voxelize_heightmapMesh_prgm->dispatchCompute(static_cast<uint>(std::ceil(static_cast<float>((mesh->getIndicesCount() / 3) / 32.0))), 1, 1);

			std::cout << "Triangles: " << (mesh->getIndicesCount() / 3) << std::endl;

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		}


		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Voxelize Heightmap Meshes - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
	}

	void LandscapeBrickComponentManager::averageVoxelization(uint index)
	{
		// Average the gathered vectors
		average_prgm->use();
		average_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));
		m_bricks[index].m_head->bind(0);
		m_bricks[index].m_guidancefield_data->bind(1);

		m_bricks[index].m_normals->bindImage(0, GL_WRITE_ONLY);
		average_prgm->setUniform("normals_tx3D", 0);
		m_bricks[index].m_normals_backbuffer->bindImage(1, GL_WRITE_ONLY);
		average_prgm->setUniform("normals_backbuffer_tx3D", 1);

		m_bricks[index].m_gradients->bindImage(2, GL_WRITE_ONLY);
		average_prgm->setUniform("gradient_tx3D", 2);
		m_bricks[index].m_gradients_backbuffer->bindImage(3, GL_WRITE_ONLY);
		average_prgm->setUniform("gradient_backbuffer_tx3D", 3);

		m_bricks[index].m_surface->bindImage(4, GL_WRITE_ONLY);
		average_prgm->setUniform("surface_tx3D", 4);
		m_bricks[index].m_surface_backbuffer->bindImage(5, GL_WRITE_ONLY);
		average_prgm->setUniform("surface_backbuffer_tx3D", 5);

		m_bricks[index].m_noise_params->bindImage(6, GL_READ_WRITE);
		average_prgm->setUniform("noise_tx3D", 6);

		average_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x / 4)) + 1,
										static_cast<uint>(std::floor(m_bricks[index].m_res_y / 2)) + 1,
										static_cast<uint>(std::floor(m_bricks[index].m_res_z / 4)) + 1);
	}

	void LandscapeBrickComponentManager::computeGuidanceField(uint index, uint iterations)
	{
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		// TODO set textures of current brick to mirrored repeat

		buildGuidance_prgm->use();

		buildGuidance_prgm->setUniform("src_tx3D",0);
		buildGuidance_prgm->setUniform("tgt_tx3D",1);

		m_bricks[index].m_surface->bindImage(2, GL_READ_ONLY);
		buildGuidance_prgm->setUniform("bc_tx3D",2);

		buildGuidance_prgm->setUniform("grid_size",Vec3(m_bricks[index].m_res_x,m_bricks[index].m_res_y,m_bricks[index].m_res_z));

		// set boundary textures
		glActiveTexture(GL_TEXTURE0);
		m_bricks[index].m_eastern_boundary[0]->bindTexture();
		buildGuidance_prgm->setUniform("east_tx3D", 0);
		glActiveTexture(GL_TEXTURE1);
		m_bricks[index].m_western_boundary[0]->bindTexture();
		buildGuidance_prgm->setUniform("west_tx3D", 1);
		glActiveTexture(GL_TEXTURE2);
		m_bricks[index].m_upper_boundary[0]->bindTexture();
		buildGuidance_prgm->setUniform("up_tx3D", 2);
		glActiveTexture(GL_TEXTURE3);
		m_bricks[index].m_lower_boundary[0]->bindTexture();
		buildGuidance_prgm->setUniform("down_tx3D", 3);
		glActiveTexture(GL_TEXTURE4);
		m_bricks[index].m_northern_boundary[0]->bindTexture();
		buildGuidance_prgm->setUniform("north_tx3D", 4);
		glActiveTexture(GL_TEXTURE5);
		m_bricks[index].m_southern_boundary[0]->bindTexture();
		buildGuidance_prgm->setUniform("south_tx3D", 5);

		int src = 0;

		// adapt iterations to ~1.5 times grid size
		if(iterations == 0)
			iterations = static_cast<int>(1.5f * std::max(std::max(m_bricks[index].m_normals->getWidth(), m_bricks[index].m_normals->getHeight()), m_bricks[index].m_normals->getDepth()));
			//iterations = static_cast<int>(2.5f * std::max(std::max(m_bricks[index].m_normals->getWidth(), m_bricks[index].m_normals->getHeight()), m_bricks[index].m_normals->getDepth()));

		for(uint i=0; i<iterations; i++)
		{
			m_bricks[index].m_normals->bindImage(src, GL_READ_ONLY);
			m_bricks[index].m_normals_backbuffer->bindImage((src + 1) % 2, GL_WRITE_ONLY);

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			buildGuidance_prgm->dispatchCompute( static_cast<uint>(std::floor(m_bricks[index].m_normals->getWidth()/4)) +1,
												 static_cast<uint>(std::floor(m_bricks[index].m_normals->getHeight()/4)) +1,
												 static_cast<uint>(std::floor(m_bricks[index].m_normals->getDepth()/4)) +1 );


			src = (src==0) ? 1 : 0;
		}

		// TODO reset textures of current brick to repeat

		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1],GL_QUERY_RESULT_AVAILABLE,&stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Guidance field diffusion - " << iterations << " iterations - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

		/*
		auto buildGuidanceGradient_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/buildGuidanceField_gradients_c.glsl" });

		buildGuidanceGradient_prgm->use();

		buildGuidanceGradient_prgm->setUniform("src_tx3D", 0);
		buildGuidanceGradient_prgm->setUniform("tgt_tx3D", 1);

		m_bricks[index].m_surface->bindImage(2, GL_READ_ONLY);
		buildGuidanceGradient_prgm->setUniform("bc_tx3D", 2);

		m_bricks[index].m_normals->bindImage(3, GL_READ_ONLY);
		buildGuidanceGradient_prgm->setUniform("normals_tx3D", 3);

		buildGuidanceGradient_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));

		src = 0;

		for (int i = 1; i<128; i++)
		{
			glBindImageTexture(src, m_bricks[index].m_gradients->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
			glBindImageTexture((src + 1) % 2, m_bricks[index].m_gradients_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glTextureBarrierNV();


			buildGuidanceGradient_prgm->dispatchCompute(m_bricks[index].m_gradients->getWidth(), m_bricks[index].m_gradients->getHeight(), m_bricks[index].m_gradients->getDepth());


			src = (src == 0) ? 1 : 0;
		}
		*/
	}

	void LandscapeBrickComponentManager::computeNoiseField(uint index, uint iterations)
	{
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		TextureLayout rgba16f_layout(GL_RGBA16F, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RGBA, GL_HALF_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

		Texture3D backbuffer("noise_backbuffer", rgba16f_layout, 0);

		copyNoiseField_prgm->use();
		copyNoiseField_prgm->setUniform("src_tx3D", 0);
		copyNoiseField_prgm->setUniform("tgt_tx3D", 1);
		copyNoiseField_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));
		m_bricks[index].m_noise_params->bindImage(0, GL_READ_ONLY);
		backbuffer.bindImage(1, GL_WRITE_ONLY);
		copyNoiseField_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getWidth()/4)) +1,
												static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getHeight()/2)) +1,
												static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getDepth()/4)) +1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		buildNoiseField_prgm->use();

		buildNoiseField_prgm->setUniform("src_tx3D", 0);
		buildNoiseField_prgm->setUniform("tgt_tx3D", 1);
		buildNoiseField_prgm->setUniform("bc_tx3D", 2);
		m_bricks[index].m_surface->bindImage(2, GL_READ_ONLY);
		buildNoiseField_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));

		//buildNoiseField_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getWidth() / 4)) + 1,
		//										static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getHeight() / 2)) + 1,
		//										static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getDepth() / 4)) + 1);

		int src = 0;

		// adapt iterations to ~1.5 times grid size
		if (iterations == 0)
			iterations = static_cast<int>(1.5f * std::max(std::max(m_bricks[index].m_normals->getWidth(), m_bricks[index].m_normals->getHeight()), m_bricks[index].m_normals->getDepth()));

		for (uint i = 0; i < iterations; i++)
		{
			m_bricks[index].m_noise_params->bindImage(src, GL_READ_ONLY);
			backbuffer.bindImage((src + 1) % 2, GL_WRITE_ONLY);

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glTextureBarrierNV();

			buildNoiseField_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getWidth()/4)) +1,
													static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getHeight()/2)) +1,
													static_cast<uint>(std::floor(m_bricks[index].m_noise_params->getDepth()/4)) +1);

			src = (src == 0) ? 1 : 0;
		}

		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Guidance noise field diffusion - " << iterations << " iterations - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

	}

	void LandscapeBrickComponentManager::computeSurfacePropagation(uint index, uint iterations)
	{
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		// TODO set textures of current brick to mirrored repeat


		// initialize boundary region
		surfacePropagationInit_prgm->use();

		surfacePropagationInit_prgm->setUniform("surface_tx3D", 0);
		surfacePropagationInit_prgm->setUniform("boundaryRegion_tx3D", 1);

		// set boundary textures
		glActiveTexture(GL_TEXTURE3);
		m_bricks[index].m_eastern_boundary[3]->bindTexture();
		surfacePropagationInit_prgm->setUniform("east_tx3D", 3);
		glActiveTexture(GL_TEXTURE4);
		m_bricks[index].m_western_boundary[3]->bindTexture();
		surfacePropagationInit_prgm->setUniform("west_tx3D", 4);
		glActiveTexture(GL_TEXTURE5);
		m_bricks[index].m_upper_boundary[3]->bindTexture();
		surfacePropagationInit_prgm->setUniform("up_tx3D", 5);
		glActiveTexture(GL_TEXTURE6);
		m_bricks[index].m_lower_boundary[3]->bindTexture();
		surfacePropagationInit_prgm->setUniform("down_tx3D", 6);
		glActiveTexture(GL_TEXTURE7);
		m_bricks[index].m_northern_boundary[3]->bindTexture();
		surfacePropagationInit_prgm->setUniform("north_tx3D", 7);
		glActiveTexture(GL_TEXTURE8);
		m_bricks[index].m_southern_boundary[3]->bindTexture();
		surfacePropagationInit_prgm->setUniform("south_tx3D", 8);

		surfacePropagationInit_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));
		surfacePropagationInit_prgm->setUniform("cell_size", Vec3(m_bricks[index].m_dimensions.x / m_bricks[index].m_res_x,
			m_bricks[index].m_dimensions.y / m_bricks[index].m_res_y,
			m_bricks[index].m_dimensions.z / m_bricks[index].m_res_z));

		m_bricks[index].m_surface->bindImage(0, GL_READ_ONLY);
		m_bricks[index].m_surface_boundaryRegion->bindImage(1, GL_WRITE_ONLY);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		surfacePropagationInit_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x / 4)) + 1,
			static_cast<uint>(std::floor(m_bricks[index].m_res_y / 2)) + 1,
			static_cast<uint>(std::floor(m_bricks[index].m_res_z / 4)) + 1);

		TextureLayout r8ui_layout(GL_R8UI, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});
		
		// regionGrowing
		Texture3D boundaryRegion_backbuffer("boundaryRegion_backbuffer", r8ui_layout, nullptr);

		surfacePropagation_prgm->use();

		glActiveTexture(GL_TEXTURE0);
		surfacePropagation_prgm->setUniform("normals_tx3D", 0);
		m_bricks[index].m_normals->bindTexture();

		surfacePropagation_prgm->setUniform("surface_tx3D", 0);
		m_bricks[index].m_surface->bindImage(0, GL_READ_WRITE);

		// set boundary textures
		glActiveTexture(GL_TEXTURE3);
		m_bricks[index].m_eastern_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("east_tx3D", 3);
		glActiveTexture(GL_TEXTURE4);
		m_bricks[index].m_western_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("west_tx3D", 4);
		glActiveTexture(GL_TEXTURE5);
		m_bricks[index].m_upper_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("up_tx3D", 5);
		glActiveTexture(GL_TEXTURE6);
		m_bricks[index].m_lower_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("down_tx3D", 6);
		glActiveTexture(GL_TEXTURE7);
		m_bricks[index].m_northern_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("north_tx3D", 7);
		glActiveTexture(GL_TEXTURE8);
		m_bricks[index].m_southern_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("south_tx3D", 8);

		glActiveTexture(GL_TEXTURE9);
		m_bricks[index].m_eastern_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_east_tx3D", 9);
		glActiveTexture(GL_TEXTURE10);
		m_bricks[index].m_western_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_west_tx3D", 10);
		glActiveTexture(GL_TEXTURE11);
		m_bricks[index].m_upper_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_up_tx3D", 11);
		glActiveTexture(GL_TEXTURE12);
		m_bricks[index].m_lower_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_down_tx3D", 12);
		glActiveTexture(GL_TEXTURE13);
		m_bricks[index].m_northern_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_north_tx3D", 13);
		glActiveTexture(GL_TEXTURE14);
		m_bricks[index].m_southern_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_south_tx3D", 14);

		surfacePropagation_prgm->setUniform("boundaryRegion_src_tx3D", 1);
		surfacePropagation_prgm->setUniform("boundaryRegion_tgt_tx3D", 2);

		surfacePropagation_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));
		surfacePropagation_prgm->setUniform("cell_size", Vec3(m_bricks[index].m_dimensions.x / m_bricks[index].m_res_x,
			m_bricks[index].m_dimensions.y / m_bricks[index].m_res_y,
			m_bricks[index].m_dimensions.z / m_bricks[index].m_res_z));

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// Adapt iterations to ~1.5 times grid size
		if (iterations == 0)
			iterations = static_cast<uint>(1.5f * std::max(std::max(m_bricks[index].m_surface->getWidth(), m_bricks[index].m_surface->getHeight()), m_bricks[index].m_surface->getDepth()));

		for (uint i = 0; i < iterations; i++)
		{
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			m_bricks[index].m_surface_boundaryRegion->bindImage(1, GL_READ_ONLY);
			boundaryRegion_backbuffer.bindImage(2, GL_WRITE_ONLY);

			surfacePropagation_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x / 4)) + 1,
				static_cast<uint>(std::floor(m_bricks[index].m_res_y / 2)) + 1,
				static_cast<uint>(std::floor(m_bricks[index].m_res_z / 4)) + 1);

			
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			m_bricks[index].m_surface_boundaryRegion->bindImage(2, GL_WRITE_ONLY);
			boundaryRegion_backbuffer.bindImage(1, GL_READ_ONLY);

			surfacePropagation_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x / 4)) + 1,
				static_cast<uint>(std::floor(m_bricks[index].m_res_y / 2)) + 1,
				static_cast<uint>(std::floor(m_bricks[index].m_res_z / 4)) + 1);
			
		}

		// TODO reset textures of current brick to repeat

		/*

		// set textures of current brick to mirrored repeat
		m_bricks[index].m_surface->texParameteri<std::vector<std::pair<GLenum, GLenum>>>({ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
																							std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
																							std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE) });

		m_bricks[index].m_normals->texParameteri<std::vector<std::pair<GLenum, GLenum>>>({ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
																							std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
																							std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE) });

		surfacePropagation_prgm->use();
		
		glActiveTexture(GL_TEXTURE0);
		surfacePropagation_prgm->setUniform("normals_tx3D",0);
		m_bricks[index].m_normals->bindTexture();

		glActiveTexture(GL_TEXTURE1);
		surfacePropagation_prgm->setUniform("gradients_tx3D", 1);
		m_bricks[index].m_gradients->bindTexture();


		surfacePropagation_prgm->setUniform("src_tx3D",0);
		surfacePropagation_prgm->setUniform("tgt_tx3D",1);

		surfacePropagation_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));

		surfacePropagation_prgm->setUniform("cell_size", Vec3(m_bricks[index].m_dimensions.x / m_bricks[index].m_res_x,
			m_bricks[index].m_dimensions.y / m_bricks[index].m_res_y,
			m_bricks[index].m_dimensions.z / m_bricks[index].m_res_z));


		// set boundary textures
		glActiveTexture(GL_TEXTURE3);
		m_bricks[index].m_eastern_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("east_tx3D", 3);
		glActiveTexture(GL_TEXTURE4);
		m_bricks[index].m_western_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("west_tx3D", 4);
		glActiveTexture(GL_TEXTURE5);
		m_bricks[index].m_upper_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("up_tx3D", 5);
		glActiveTexture(GL_TEXTURE6);
		m_bricks[index].m_lower_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("down_tx3D", 6);
		glActiveTexture(GL_TEXTURE7);
		m_bricks[index].m_northern_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("north_tx3D", 7);
		glActiveTexture(GL_TEXTURE8);
		m_bricks[index].m_southern_boundary[3]->bindTexture();
		surfacePropagation_prgm->setUniform("south_tx3D", 8);

		glActiveTexture(GL_TEXTURE9);
		m_bricks[index].m_eastern_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_east_tx3D", 9);
		glActiveTexture(GL_TEXTURE10);
		m_bricks[index].m_western_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_west_tx3D", 10);
		glActiveTexture(GL_TEXTURE11);
		m_bricks[index].m_upper_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_up_tx3D",11);
		glActiveTexture(GL_TEXTURE12);
		m_bricks[index].m_lower_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_down_tx3D", 12);
		glActiveTexture(GL_TEXTURE13);
		m_bricks[index].m_northern_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_north_tx3D", 13);
		glActiveTexture(GL_TEXTURE14);
		m_bricks[index].m_southern_boundary[0]->bindTexture();
		surfacePropagation_prgm->setUniform("normals_south_tx3D", 14);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// Adapt iterations to ~1.5 times grid size. Note: Each loops contains two iteration
		if(iterations == 0)
			iterations = static_cast<int>(1.5f * std::max(std::max(m_bricks[index].m_surface->getWidth(), m_bricks[index].m_surface->getHeight()), m_bricks[index].m_surface->getDepth()));

		for (uint i = 0; i < iterations; i++)
		{
			m_bricks[index].m_surface->bindImage(0, GL_READ_ONLY);
			//glBindImageTexture(3, m_bricks[index].m_surface_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
			m_bricks[index].m_surface_backbuffer->bindImage(1, GL_WRITE_ONLY);

			surfacePropagation_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x/4)) +1 ,
														static_cast<uint>(std::floor(m_bricks[index].m_res_y/2)) +1,
														static_cast<uint>(std::floor(m_bricks[index].m_res_z/4)) +1);

			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			m_bricks[index].m_surface_backbuffer->bindImage(0, GL_READ_ONLY);
			//glBindImageTexture(3, m_bricks[index].m_surface->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
			m_bricks[index].m_surface->bindImage(1, GL_WRITE_ONLY);

			surfacePropagation_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x / 4)) + 1,
														static_cast<uint>(std::floor(m_bricks[index].m_res_y / 2)) + 1,
														static_cast<uint>(std::floor(m_bricks[index].m_res_z / 4)) + 1);
		}


		m_bricks[index].m_surface->texParameteri<std::vector<std::pair<GLenum, GLenum>>>({ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_REPEAT),
																							std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_REPEAT),
																							std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_REPEAT) });

		m_bricks[index].m_normals->texParameteri<std::vector<std::pair<GLenum, GLenum>>>({ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_REPEAT),
																							std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_REPEAT),
																							std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_REPEAT) });

		*/

		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Surface Propagation - " << iterations*2 << " iterations - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;



		//TODO SPEED UP: Compute sparse volume with long lic travel distance, then fill gaps with very short lics

		/*
		for(int i=0; i<1; i++)
		{
			m_bricks[index].m_surface->bindTexture();

			glBindImageTexture(3,m_bricks[index].m_surface_backbuffer->getHandle(),0,GL_FALSE,0,GL_READ_WRITE,GL_R8);

			surfacePropagation_prgm->setUniform("flip", 1 );

			surfacePropagation_prgm->dispatchCompute(m_bricks[index].m_res_x,
														m_bricks[index].m_res_y,
														m_bricks[index].m_res_z);
			
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			m_bricks[index].m_surface_backbuffer->bindTexture();
			glBindImageTexture(3,m_bricks[index].m_surface->getHandle(),0,GL_FALSE,0,GL_READ_WRITE,GL_R8);
			
			surfacePropagation_prgm->setUniform("flip", 0 );
			
			surfacePropagation_prgm->dispatchCompute(m_bricks[index].m_res_x,
														m_bricks[index].m_res_y,
														m_bricks[index].m_res_z);
			
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}		
		*/
		
	}

	void LandscapeBrickComponentManager::smoothSurfaceField(uint index)
	{
		// Smooth resulting surface field
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		
		smooth_prgm->use();

		smooth_prgm->setUniform("stencilRadius", 2);

		smooth_prgm->setUniform("sigma", 0.8f);

		smooth_prgm->setUniform("src_tx3D", 0);
		//glBindImageTexture(0, m_bricks[index].m_surface->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
		m_bricks[index].m_surface->bindImage(0, GL_READ_WRITE);
		smooth_prgm->setUniform("tgt_tx3D", 1);
		//glBindImageTexture(1, m_bricks[index].m_surface_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
		m_bricks[index].m_surface_backbuffer->bindImage(1, GL_READ_WRITE);

		smooth_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));

		smooth_prgm->setUniform("offset", Vec3(1.0, 0.0, 0.0));

		smooth_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x / 4)) + 1,
			static_cast<uint>(std::floor(m_bricks[index].m_res_y / 2)) + 1,
			static_cast<uint>(std::floor(m_bricks[index].m_res_z / 4)) + 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


		//glBindImageTexture(1, m_bricks[index].m_surface->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
		//glBindImageTexture(0, m_bricks[index].m_surface_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
		m_bricks[index].m_surface->bindImage(1, GL_READ_WRITE);
		m_bricks[index].m_surface_backbuffer->bindImage(0, GL_READ_WRITE);

		smooth_prgm->setUniform("offset", Vec3(0.0, 1.0, 0.0));

		smooth_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x / 4)) + 1,
			static_cast<uint>(std::floor(m_bricks[index].m_res_y / 2)) + 1,
			static_cast<uint>(std::floor(m_bricks[index].m_res_z / 4)) + 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		//glBindImageTexture(0, m_bricks[index].m_surface->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
		//glBindImageTexture(1, m_bricks[index].m_surface_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
		m_bricks[index].m_surface->bindImage(0, GL_READ_WRITE);
		m_bricks[index].m_surface_backbuffer->bindImage(1, GL_READ_WRITE);

		smooth_prgm->setUniform("offset", Vec3(0.0, 0.0, 1.0));

		smooth_prgm->dispatchCompute(static_cast<uint>(std::floor(m_bricks[index].m_res_x  /4)) +1,
										static_cast<uint>(std::floor(m_bricks[index].m_res_y / 2)) +1,
										static_cast<uint>(std::floor(m_bricks[index].m_res_z / 4)) +1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// Copy result to surface field
		copySurfaceField_prgm->use();
		copySurfaceField_prgm->setUniform("src_tx3D", 0);
		copySurfaceField_prgm->setUniform("tgt_tx3D", 1);
		copySurfaceField_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z));
		m_bricks[index].m_surface_backbuffer->bindImage(0, GL_READ_ONLY);
		m_bricks[index].m_surface->bindImage(1, GL_WRITE_ONLY);
		copySurfaceField_prgm->dispatchCompute( static_cast<uint>(std::floor(m_bricks[index].m_surface->getWidth()/4)) +1,
												static_cast<uint>(std::floor(m_bricks[index].m_surface->getHeight()/2)) +1,
												static_cast<uint>(std::floor(m_bricks[index].m_surface->getDepth()/4)) +1);

		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Smooth fields - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
	}

	void LandscapeBrickComponentManager::computeSurfaceMesh(uint index)
	{
		std::cout << "Compute surface mesh" << std::endl;

		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		// See http://www.eriksmistad.no/marching-cubes-implementation-using-opencl-and-opengl/ for reference

		// Create texture for histogram pyramid (hp)
		int pyramid_lvls = static_cast<int>(std::log2( std::max(m_bricks[index].m_res_x,std::max(m_bricks[index].m_res_y,m_bricks[index].m_res_z)) ) + 1);
		Texture3D** histogram_pyramid = new Texture3D*[pyramid_lvls];

		//TODO this get's generated a lot...
		//auto generateTriangles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS "+std::to_string(pyramid_lvls)+"\n");

		GLSLProgram* generateTriangles_prgm;

		std::cout << "Pyramid levels: " << pyramid_lvls << std::endl;

		if (pyramid_lvls == 6)
			generateTriangles_prgm = generateTriangles_prgms[0];
		else if (pyramid_lvls == 7)
			generateTriangles_prgm = generateTriangles_prgms[1];
		else if (pyramid_lvls == 8)
			generateTriangles_prgm = generateTriangles_prgms[2];
		else if (pyramid_lvls == 9)
			generateTriangles_prgm = generateTriangles_prgms[3];
		else if (pyramid_lvls == 10)
			generateTriangles_prgm = generateTriangles_prgms[4];
		else
			return;


		// Create texture for storing each cubes index (for marching cube lut)
		TextureLayout r8ui_layout(GL_R8UI, m_bricks[index].m_res_x, m_bricks[index].m_res_y, m_bricks[index].m_res_z, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1);
		Texture3D cube_indices("cube_indices", r8ui_layout, nullptr);

		for(int i=0; i<pyramid_lvls; i++)
		{
			uint x = std::max( 1, (int)( (m_bricks[index].m_res_x-1) / pow(2,i)) );
			uint y = std::max( 1, (int)( (m_bricks[index].m_res_y-1) / pow(2,i)) );
			uint z = std::max( 1, (int)( (m_bricks[index].m_res_z-1) / pow(2,i)) );

			TextureLayout r8ui_layout(GL_R8UI, x, y, z, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1);
			TextureLayout r16ui_layout(GL_R16UI, x, y, z, GL_RED_INTEGER, GL_UNSIGNED_SHORT, 1);
			TextureLayout r32i_layout(GL_R32UI, x, y, z, GL_RED_INTEGER, GL_UNSIGNED_INT, 1);

			if( i <= 1 )
				histogram_pyramid[i] = new Texture3D("hp"+std::to_string(i), r8ui_layout, nullptr);
			else if( i > 1 && i < 5 )
				histogram_pyramid[i] = new Texture3D("hp"+std::to_string(i), r16ui_layout, nullptr);
			else
				histogram_pyramid[i] = new Texture3D("hp"+std::to_string(i), r32i_layout, nullptr);
		}

		// Iso value for surface extraction
		float iso_value = 0.0;

		// Classify voxels
		classifyVoxels_prgm->use();
		classifyVoxels_prgm->setUniform("iso_value",iso_value);
		classifyVoxels_prgm->setUniform("surface_tx3D",0);
		classifyVoxels_prgm->setUniform("histogram_pyramid_tx3D",1);
		classifyVoxels_prgm->setUniform("cube_indices_tx3D",2);
		m_bricks[index].m_surface->bindImage(0, GL_READ_ONLY);
		histogram_pyramid[0]->bindImage(1, GL_WRITE_ONLY);
		cube_indices.bindImage(2, GL_WRITE_ONLY);

		glActiveTexture(GL_TEXTURE0);
		m_bricks[index].m_eastern_boundary[3]->bindTexture();
		classifyVoxels_prgm->setUniform("east_surface_tx3D", 0);
		glActiveTexture(GL_TEXTURE1);
		m_bricks[index].m_upper_boundary[3]->bindTexture();
		classifyVoxels_prgm->setUniform("up_surface_tx3D", 1);
		glActiveTexture(GL_TEXTURE2);
		m_bricks[index].m_northern_boundary[3]->bindTexture();
		classifyVoxels_prgm->setUniform("north_surface_tx3D", 2);

		classifyVoxels_prgm->dispatchCompute(static_cast<uint>(std::floor(histogram_pyramid[0]->getWidth()/4)),
			static_cast<uint>(std::floor(histogram_pyramid[0]->getHeight()/2)),
			static_cast<uint>(std::floor(histogram_pyramid[0]->getDepth()/4)) );

		//std::cout<<"Pyramid levels: "<<pyramid_lvls<<std::endl;

		// Compute hp
		for(int i=0; i<pyramid_lvls-1; i++)
		{
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			if( i == 0)
			{
				buildHpLvl_R8_prgm->use();
				buildHpLvl_R8_prgm->setUniform("src_tx3D",0);
				buildHpLvl_R8_prgm->setUniform("tgt_tx3D",1);
				histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
				histogram_pyramid[i+1]->bindImage(1, GL_READ_WRITE);
				buildHpLvl_R8_prgm->dispatchCompute(histogram_pyramid[i+1]->getWidth(),histogram_pyramid[i+1]->getHeight(),histogram_pyramid[i+1]->getDepth());
			}
			else if ( i == 1)
			{
				buildHpLvl_R8toR16_prgm->use();
				buildHpLvl_R8toR16_prgm->setUniform("src_tx3D",0);
				buildHpLvl_R8toR16_prgm->setUniform("tgt_tx3D",1);
				histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
				histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
				buildHpLvl_R8toR16_prgm->dispatchCompute(histogram_pyramid[i+1]->getWidth(),histogram_pyramid[i+1]->getHeight(),histogram_pyramid[i+1]->getDepth());
			}
			else if( i == 2 || i == 3 )
			{
				buildHpLvl_R16_prgm->use();
				buildHpLvl_R16_prgm->setUniform("src_tx3D",0);
				buildHpLvl_R16_prgm->setUniform("tgt_tx3D",1);
				histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
				histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
				buildHpLvl_R16_prgm->dispatchCompute(histogram_pyramid[i+1]->getWidth(),histogram_pyramid[i+1]->getHeight(),histogram_pyramid[i+1]->getDepth());
			}
			else if ( i == 4)
			{
				buildHpLvl_R16toR32_prgm->use();
				buildHpLvl_R16toR32_prgm->setUniform("src_tx3D",0);
				buildHpLvl_R16toR32_prgm->setUniform("tgt_tx3D",1);
				histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
				histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
				buildHpLvl_R16toR32_prgm->dispatchCompute(histogram_pyramid[i+1]->getWidth(),histogram_pyramid[i+1]->getHeight(),histogram_pyramid[i+1]->getDepth());
			}
			else
			{
				buildHpLvl_R32_prgm->use();
				buildHpLvl_R32_prgm->setUniform("src_tx3D",0);
				buildHpLvl_R32_prgm->setUniform("tgt_tx3D",1);
				histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
				histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
				buildHpLvl_R32_prgm->dispatchCompute(histogram_pyramid[i+1]->getWidth(),histogram_pyramid[i+1]->getHeight(),histogram_pyramid[i+1]->getDepth());
			}
		}


		// Generate triangles (traverse hp)
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		glUseProgram(0);
		GLuint triangle_count[1];
		histogram_pyramid[pyramid_lvls-1]->bindTexture();
		glGetTexImage(GL_TEXTURE_3D,0,GL_RED_INTEGER,GL_UNSIGNED_INT,triangle_count);

		//std::cout<<"hp size: "<<histogram_pyramid[pyramid_lvls-1]->getWidth()*histogram_pyramid[pyramid_lvls-1]->getHeight()*histogram_pyramid[pyramid_lvls-1]->getDepth()<<std::endl;
		std::cout<<"Triangle Count: "<<triangle_count[0]<<std::endl;
		
		// Adjust mesh size
		GLsizei vertices_byte_size = triangle_count[0] * 3 * 9 * 4; // triangle count * vertices per triangle * elements per vertex * byte per element
		GLsizei indices_byte_size = triangle_count[0] * 3 * 4; // triangle count * indices per triangle * byte size of index
		//m_bricks[index].m_surface_mesh->rebuffer(nullptr, nullptr, vertices_byte_size, indices_byte_size, GL_PATCHES);
		m_bricks[index].m_surface_mesh->rebufferVertexData(nullptr, vertices_byte_size);
		m_bricks[index].m_surface_mesh->rebufferIndexData(nullptr, indices_byte_size);

		ShaderStorageBufferObject triangle_table_ssbo(triangle_table);

		generateTriangles_prgm->use();

		generateTriangles_prgm->setUniform("iso_value",iso_value);
		generateTriangles_prgm->setUniform("grid_size",Vec3(m_bricks[index].m_res_x,m_bricks[index].m_res_y,m_bricks[index].m_res_z));
		generateTriangles_prgm->setUniform("brick_size",m_bricks[index].m_dimensions);

		glActiveTexture(GL_TEXTURE0);
		generateTriangles_prgm->setUniform("surface_tx3D",0);
		m_bricks[index].m_surface->bindTexture();

		glActiveTexture(GL_TEXTURE1);
		generateTriangles_prgm->setUniform("noise_tx3D", 1);
		m_bricks[index].m_noise_params->bindTexture();

		glActiveTexture(GL_TEXTURE2);
		generateTriangles_prgm->setUniform("cube_indices_tx3D",2);
		cube_indices.bindTexture();



		for(int i=0; i<pyramid_lvls; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i + 6);
			std::string uniform_name("hp"+std::to_string(i)+"_tx3D");
			generateTriangles_prgm->setUniform(uniform_name.c_str(),i+6);
			histogram_pyramid[i]->bindTexture();
		}

		triangle_table_ssbo.bind(0);

		uint tri_cnt = triangle_count[0];
		generateTriangles_prgm->setUniform("triangle_cnt", tri_cnt);

		// Bind vertex and index buffer as storage buffer
		m_bricks[index].m_surface_mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
		m_bricks[index].m_surface_mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 2);

		generateTriangles_prgm->dispatchCompute(static_cast<uint>(std::floor(tri_cnt / 32)) +1 , 1, 1);
		//generateTriangles_prgm->dispatchCompute(tri_cnt, 1, 1);

		// Add surface mesh to static meshes
		//GRenderingComponents::staticMeshManager().addComponent(m_bricks[index].m_entity,m_bricks[index].m_surface_mesh,m_bricks[index].m_surface_material,false);

		generateTriangles_prgm->dispatchCompute(static_cast<uint>(std::floor(tri_cnt / 32)) + 1, 1, 1);


		// Clean up resources
		for(int i=0; i<pyramid_lvls; i++)
			delete histogram_pyramid[i];
		
		delete[] histogram_pyramid;


		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until th e results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Surface Reconstruction - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
	}

	void LandscapeBrickComponentManager::computeNaiveSurfaceNetsMesh(uint index)
	{
		std::unique_lock<std::mutex> ptex_lock(m_ptex_update);

		m_bricks[index].m_cancel_ptex_update = true;

		std::cout << "Compute surface mesh" << std::endl;

		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		TextureLayout r32ui_layout(GL_R32UI,
			m_bricks[index].m_res_x - 1,
			m_bricks[index].m_res_y - 1,
			m_bricks[index].m_res_z - 1,
			GL_RED_INTEGER,
			GL_UNSIGNED_INT,
			1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_REPEAT),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_REPEAT),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_REPEAT),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

		TextureLayout r16i_layout(GL_R16UI,
			m_bricks[index].m_res_x - 1,
			m_bricks[index].m_res_y - 1,
			m_bricks[index].m_res_z - 1,
			GL_RED_INTEGER,
			GL_UNSIGNED_SHORT,
			1,
			{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_REPEAT),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_REPEAT),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_REPEAT),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

		Texture3D vertex_indices("surface_nets_vertex_indices", r32ui_layout, nullptr);

		Texture3D edge_crossings("surface_nets_edge_crossings", r16i_layout, nullptr);

		ShaderStorageBufferObject active_cubes( (m_bricks[index].m_res_x - 1)*(m_bricks[index].m_res_y - 1)*(m_bricks[index].m_res_z - 1) *4 *4, nullptr);
		std::vector<GLuint> data({ 0 });
		ShaderStorageBufferObject quad_counter_buffer(data);

		// Create atomic counter
		GLuint counter_buffer;
		glGenBuffers(1, &(counter_buffer));
		GLuint zero = 0;
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, counter_buffer);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, 0);

		// Pass 1 - classify cubes

		// Iso value for surface extraction
		float iso_value = 0.0;

		// Classify voxels
		surfaceNets_classify_prgm->use();
		surfaceNets_classify_prgm->setUniform("iso_value", iso_value);
		surfaceNets_classify_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x-1, m_bricks[index].m_res_y-1, m_bricks[index].m_res_z-1));
		surfaceNets_classify_prgm->setUniform("brick_size", m_bricks[index].m_dimensions);
		surfaceNets_classify_prgm->setUniform("surface_tx3D", 0);
		surfaceNets_classify_prgm->setUniform("vertex_indices_tx3D", 1);
		surfaceNets_classify_prgm->setUniform("edge_crossings_tx3D", 2);
		m_bricks[index].m_surface->bindImage(0, GL_READ_ONLY);
		vertex_indices.bindImage(1, GL_WRITE_ONLY);
		edge_crossings.bindImage(2, GL_WRITE_ONLY);
		
		active_cubes.bind(0);
		quad_counter_buffer.bind(1);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, counter_buffer);

		surfaceNets_classify_prgm->dispatchCompute(vertex_indices.getWidth()+1,
														vertex_indices.getHeight()+1,
														vertex_indices.getDepth()+1);

		GLuint quad_cnt = 0;
		quad_counter_buffer.bind();
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(&quad_cnt, p, 4);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		std::cout << "Quad count: " << quad_cnt << std::endl;


		GLuint buffer_counter = 0;
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer);
		p = glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
		memcpy(&buffer_counter, p, 4);
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

		std::cout << "Active cubes: " << buffer_counter << std::endl;

		if (buffer_counter == 0)
			return;

		// Adjust mesh size
		GLsizei vertices_byte_size = buffer_counter * 9 * 4; // active cubes * elements per vertex * byte per element
		GLsizei indices_byte_size = quad_cnt * 4 * 4; // quad count * indices per quad * byte size of index
		//m_bricks[index].m_surface_mesh->rebuffer(nullptr, nullptr, vertices_byte_size, indices_byte_size, GL_PATCHES);
		m_bricks[index].m_surface_mesh->rebufferVertexData(nullptr, vertices_byte_size);
		m_bricks[index].m_surface_mesh->rebufferIndexData(nullptr, indices_byte_size);

		//////////////////////////
		// PTEX BUFFERS
		//////////////////////////
		auto ptex_mesh_resource = GEngineCore::resourceManager().getMesh(m_bricks[index].m_ptex_mesh);
		//ptex_mesh_resource.resource->rebuffer(nullptr, nullptr, buffer_counter * 6 * 4, quad_cnt * 4 * 4, GL_PATCHES);
		ptex_mesh_resource.resource->rebufferVertexData(nullptr, buffer_counter * 6 * 4);
		ptex_mesh_resource.resource->rebufferIndexData(nullptr, quad_cnt * 4 * 4);

		auto ptex_tiles_per_edge_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_tiles_per_edge);
		std::vector<int> tiles_per_edges_init_buffer(buffer_counter * 6, -1);
		ptex_tiles_per_edge_resource.resource->reload(tiles_per_edges_init_buffer); // active cubes * 3 edges * 2 quad entries per edge * 4 byte per element

		auto ptex_parameters_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters);
		ptex_parameters_resource.resource->reload(quad_cnt * ((4 * 7)), 0, nullptr);
		auto ptex_parameters_backbuffer_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters_backbuffer);
		ptex_parameters_backbuffer_resource.resource->reload(quad_cnt * ((4 * 7)), 0, nullptr);

		auto patch_distances_SSBO_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_patch_distances_SSBO);
		patch_distances_SSBO_resource.resource->reload(quad_cnt * 3 * 4, 0, nullptr);

		m_bricks[index].m_ptex_patch_distances.resize(quad_cnt);
		//m_bricks[index].m_ptex_patch_lod_classification.resize(quad_cnt);
		//////////////////////////
		// PTEX BUFFERS
		//////////////////////////

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// TODO Pass 2 - generate Quads
		surfaceNets_generateQuads_prgm->use();

		glActiveTexture(GL_TEXTURE0);
		surfaceNets_generateQuads_prgm->setUniform("surface_tx3D", 0);
		m_bricks[index].m_surface->bindTexture();
		glActiveTexture(GL_TEXTURE1);
		surfaceNets_generateQuads_prgm->setUniform("noise_tx3D", 1);
		m_bricks[index].m_noise_params->bindTexture();

		surfaceNets_generateQuads_prgm->setUniform("vertex_indices_tx3D", 1);
		surfaceNets_generateQuads_prgm->setUniform("edge_crossings_tx3D", 2);
		vertex_indices.bindImage(1, GL_READ_ONLY);
		edge_crossings.bindImage(2, GL_READ_ONLY);

		active_cubes.bind(0);
		// Bind vertex and index buffer as storage buffer
		m_bricks[index].m_surface_mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
		m_bricks[index].m_surface_mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 2);

		// Also fill ptex mesh and eventually ditch the old mesh
		ptex_mesh_resource.resource->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 3);
		ptex_mesh_resource.resource->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 4);

		ptex_parameters_resource.resource->bind(5);
		ptex_tiles_per_edge_resource.resource->bind(6);

		surfaceNets_generateQuads_prgm->setUniform("iso_value", iso_value);
		surfaceNets_generateQuads_prgm->setUniform("grid_size", Vec3(m_bricks[index].m_res_x - 1, m_bricks[index].m_res_y - 1, m_bricks[index].m_res_z - 1));
		surfaceNets_generateQuads_prgm->setUniform("brick_size", m_bricks[index].m_dimensions);
		surfaceNets_generateQuads_prgm->setUniform("active_cubes_cnt", buffer_counter);

		surfaceNets_generateQuads_prgm->dispatchCompute(static_cast<uint>(floor(buffer_counter/32)+1),1,1);
		
		//std::vector<GLfloat> debug_vertices = {-10.0,0.0,10.0, 0.0, 0.0,1.0,0.0, 0.0, 0.0,
		//	10.0,0.0,10.0, 0.0, 0.0,1.0,0.0, 0.0, 1.0, 
		//	10.0,0.0,-10.0, 0.0, 0.0,1.0,0.0, 0.0, 1.0, 
		//	-8.0,0.0,-8.0, 0.0, 0.0,1.0,0.0, 0.0, 2.0 };
		//std::vector<GLuint> debug_indices = { 0,1,2,3 };
		//m_bricks[index].m_surface_mesh->rebuffer(debug_vertices, debug_indices, GL_PATCHES);


		// Compute Ptex neighbourhood
		computePtexNeighbours_prgm->use();

		ptex_parameters_resource.resource->bind(0);

		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ptex_mesh_resource.resource->getIboHandle());
		ptex_mesh_resource.resource->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 4);

		ptex_tiles_per_edge_resource.resource->bind(6);

		computePtexNeighbours_prgm->setUniform("ptex_tile_cnt", quad_cnt);

		computePtexNeighbours_prgm->dispatchCompute(static_cast<uint>(std::floor(quad_cnt / 32) + 1), 1, 1);


		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		std::cout << "Surface Reconstruction - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

		auto gl_err = glGetError();
		if (gl_err != GL_NO_ERROR)
			std::cerr << "GL error in surface reconstruction: " << gl_err << std::endl;

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// Copy resulting quad surface mesh data to CPU memory
		size_t byte_size = ptex_mesh_resource.resource->getVbo().getByteSize();
		m_bricks[index].m_mesh_vertex_data.resize(byte_size / 4);
		size_t element_cnt = m_bricks[index].m_mesh_vertex_data.size();
		ptex_mesh_resource.resource->getVbo().bind();
		GLvoid* vertex_data = glMapBufferRange(GL_ARRAY_BUFFER, 0, byte_size, GL_MAP_READ_BIT);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			// "Do something cop!"
			std::cerr << "GL error vertex data mapping: " << err << std::endl;
		}

		std::copy(reinterpret_cast<float*>(vertex_data), reinterpret_cast<float*>(vertex_data) + element_cnt, m_bricks[index].m_mesh_vertex_data.data());
		glUnmapBuffer(GL_ARRAY_BUFFER);

		byte_size = ptex_mesh_resource.resource->getIbo().getByteSize();
		m_bricks[index].m_mesh_index_data.resize(byte_size / 4);
		ptex_mesh_resource.resource->getIbo().bind();
		GLvoid* index_data = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, byte_size, GL_MAP_READ_BIT);
		memcpy(m_bricks[index].m_mesh_index_data.data(), index_data, byte_size);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

		byte_size = ptex_parameters_resource.resource->getSize();
		m_bricks[index].m_mesh_ptex_params.resize(quad_cnt); // 1 set of ptex params per quad
		ptex_parameters_resource.resource->bind();
		GLvoid* ptex_params = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byte_size, GL_MAP_READ_BIT);
		memcpy(m_bricks[index].m_mesh_ptex_params.data(), ptex_params, byte_size);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


		// Bake intial surface textures
		bakeSurfaceTextures(index);
	}

	void LandscapeBrickComponentManager::bakeSurfaceTextures(uint index)
	{
		std::cout << "Texture Baking" << std::endl;

		uint primitive_cnt = m_bricks[index].m_surface_mesh->getIndicesCount() / 4; //Quad primitives -> 4 indices per primitive
		
		std::cout << "Primitive count (texture baking stage): " << primitive_cnt << std::endl;

		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		// rebuffer ptex mesh, textures and parameters
		WeakResource<Mesh>						ptex_mesh_resource = GEngineCore::resourceManager().getMesh(m_bricks[index].m_ptex_mesh);
		WeakResource<ShaderStorageBufferObject>	ptex_bindless_texture_handles_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_texture_handles);
		WeakResource<ShaderStorageBufferObject>	ptex_bindless_images_handles_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_image_handles);
		WeakResource<ShaderStorageBufferObject>	ptex_bindless_mipmap_image_handles_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_mipmap_image_handles);
		WeakResource<ShaderStorageBufferObject>	ptex_parameters_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters);

		int material_components = 4;
		GLint layers;
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &layers);
		layers = layers - (layers % material_components);
		
		// TODO query GPU vendor and subsequently the available video memory to make an assumption about how much memory I want to spend on Ptex textures
		size_t available_ptex_memory = 1500000000; // GPU memory available for ptex textures given in byte
		size_t used_ptex_memory = 0;
		size_t texture_array_cnt = 0;

		// create vista texture tiles (8x8) for all surface patches
		size_t vista_tiles_array_cnt = (1 + ((primitive_cnt * material_components + 1) / layers));
		size_t vista_tiles_memory = vista_tiles_array_cnt * layers * (8 * 8) * 4;
		used_ptex_memory += vista_tiles_memory;
		texture_array_cnt += vista_tiles_array_cnt;

		// if more than ~1GB of available memory left, use 256x256 detail tiles
		m_bricks[index].m_lod_lvls = ((available_ptex_memory - used_ptex_memory) > 1000000000) ? 6 : 5;
		//m_bricks[index].m_lod_lvls = ((available_ptex_memory - used_ptex_memory) > 1000000000) ? 7 : 6;
		m_bricks[index].m_ptex_lod_bin_sizes.resize(m_bricks[index].m_lod_lvls);

		m_bricks[index].m_ptex_active_patch_lod_classification.clear();
		m_bricks[index].m_ptex_active_patch_lod_classification.resize(primitive_cnt, m_bricks[index].m_lod_lvls - 1); // intialize with max lod level (i.e. vista layer)

		m_bricks[index].m_ptex_latest_patch_lod_classification.clear();
		m_bricks[index].m_ptex_latest_patch_lod_classification.resize(primitive_cnt, m_bricks[index].m_lod_lvls - 1); // intialize with max lod level (i.e. vista layer)

		// use one set of detail tiles
		size_t detail_tiles_memory = layers * static_cast<size_t>(std::pow(2, m_bricks[index].m_lod_lvls + 2)) * static_cast<size_t>(std::pow(2, m_bricks[index].m_lod_lvls + 2)) * 4;
		used_ptex_memory += detail_tiles_memory;
		texture_array_cnt += 1;
		m_bricks[index].m_ptex_lod_bin_sizes[0] = layers / material_components;

		assert(available_ptex_memory > used_ptex_memory);

		// split remaining memory evenly among remaining lod levels
		size_t memory_per_lvl = static_cast<size_t>(std::floor(((available_ptex_memory - used_ptex_memory) / (m_bricks[index].m_lod_lvls - 2))));

		for (int i = 1; i < m_bricks[index].m_lod_lvls -1; ++i) // skip detail and vista level
		{
			// compute number of texture arrays per level based on available memory per level
			size_t array_memory = layers * (std::pow(2, (m_bricks[index].m_lod_lvls + 2)-i) * std::pow(2, (m_bricks[index].m_lod_lvls + 2) - i)) * 4;
			size_t array_cnt = static_cast<size_t>(std::floor(memory_per_lvl / array_memory));
			m_bricks[index].m_ptex_lod_bin_sizes[i] = (array_cnt * layers) / material_components;
			used_ptex_memory += array_memory*array_cnt;
			texture_array_cnt += array_cnt;
		}

		m_bricks[index].m_ptex_lod_bin_sizes[m_bricks[index].m_lod_lvls -1] = (vista_tiles_array_cnt * layers) / material_components;

		// clear current update target tiles (basically cancel current update) as it will no longer match the surface mesh
		m_bricks[index].m_ptex_updatePatches_tgt.clear();

		// intialize available tiles (at the beginning, all tiles on all levels are available). exclude vista layer
		m_bricks[index].m_ptex_active_availableTiles.resize(m_bricks[index].m_lod_lvls - 1);
		m_bricks[index].m_ptex_latest_availableTiles.resize(m_bricks[index].m_lod_lvls - 1);
		uint32_t assigned_tiles_a = 0;
		uint32_t assigned_tiles_l = 0;
		for (int i = 0; i < m_bricks[index].m_lod_lvls - 1; ++i)
		{
			m_bricks[index].m_ptex_active_availableTiles[i].resize(m_bricks[index].m_ptex_lod_bin_sizes[i]);
			m_bricks[index].m_ptex_latest_availableTiles[i].resize(m_bricks[index].m_ptex_lod_bin_sizes[i]);
			//std::iota(m_bricks[index].m_ptex_availableTiles[i].begin(), m_bricks[index].m_ptex_availableTiles[i].end(), 0);
			for(auto& slot : m_bricks[index].m_ptex_active_availableTiles[i])
			{
				LandscapeBrickComponent::TextureSlot new_slot;
				new_slot.tex_index = (assigned_tiles_a / layers);
				new_slot.base_slice = assigned_tiles_a % 2048;
				slot = new_slot;

				assigned_tiles_a += material_components;
			}

			for (auto& slot : m_bricks[index].m_ptex_latest_availableTiles[i])
			{
				LandscapeBrickComponent::TextureSlot new_slot;
				new_slot.tex_index = (assigned_tiles_l / layers);
				new_slot.base_slice = assigned_tiles_l % 2048;
				slot = new_slot;

				assigned_tiles_l += material_components;
			}
		}

		//DEBUGGING
		//for (auto v : m_bricks[index].m_ptex_availableTiles[0])
		//{
		//	std::cout << "Available tile:" << v.tex_index << " " << v.base_slice << std::endl;
		//}

		std::cout << "Num texture arrays: " << texture_array_cnt << std::endl;
		std::cout << "Num layers per array: " << layers << std::endl;
		std::cout << "Num lod levels: " << m_bricks[index].m_lod_lvls << std::endl;

		std::vector<GLuint64> texture_handles;
		texture_handles.reserve(texture_array_cnt);
		std::vector<GLuint64> image_handles;
		std::vector<GLuint64> mipmap_image_handles;
		image_handles.reserve(texture_array_cnt);
		m_bricks[index].m_ptex_textures.clear();
		m_bricks[index].m_ptex_textures.reserve(texture_array_cnt);

		//std::vector<std::vector<uint8_t>> debug_image_data(lod_lvls);

		for (size_t i = 0; i < m_bricks[index].m_lod_lvls; ++i)
		{
			TextureLayout tile_layout;
			tile_layout.internal_format = GL_RGBA8;
			tile_layout.format = GL_RGBA;
			tile_layout.type = GL_UNSIGNED_BYTE;
			tile_layout.width = static_cast<int>(std::pow(2, (m_bricks[index].m_lod_lvls + 2) - i)); //start at 8
			tile_layout.height = static_cast<int>(std::pow(2, (m_bricks[index].m_lod_lvls + 2) - i)); // times 8
			tile_layout.depth = layers;
			tile_layout.levels = 2;

			tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
			tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
			tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER });
			//tile_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST });
			tile_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
			tile_layout.int_parameters.push_back({ GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f });
			tile_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER, GL_LINEAR });

			//debug_image_data[i] = std::vector<uint8_t>(tile_layout.width*tile_layout.height*tile_layout.depth*4,255);

			size_t array_cnt = (m_bricks[index].m_ptex_lod_bin_sizes[i]*material_components)/layers;
			for (size_t j=0; j<array_cnt; ++j)
			{
				auto tx_resource = GEngineCore::resourceManager().createTexture2DArray("brick_" + std::to_string(index) + "_ptex_lvl_"+ std::to_string(i) + "_array_ " + std::to_string(j), tile_layout, nullptr, false);
				//auto tx_resource = GEngineCore::resourceManager().createTexture2DArray("brick_" + std::to_string(index) + "_ptex_lvl_" + std::to_string(i) + "_array_ " + std::to_string(j), tile_layout, debug_image_data[i].data(), false);
				m_bricks[index].m_ptex_textures.push_back(tx_resource.id);

				GLenum err = glGetError();
				if (err != GL_NO_ERROR)
				{
					// "Do something cop!"
					//std::cerr << "GL error during texture baking creation after texture creation: " << err << std::endl;
				}

				texture_handles.push_back(tx_resource.resource->getTextureHandle());
				image_handles.push_back(tx_resource.resource->getImageHandle(0, GL_TRUE, 0));
				mipmap_image_handles.push_back(tx_resource.resource->getImageHandle(1, GL_TRUE, 0)); // we use two mipmap levels for ptex tiles so query both levels

				tx_resource.resource->makeResident();
				glMakeImageHandleResidentARB(image_handles.back(), GL_WRITE_ONLY);
				glMakeImageHandleResidentARB(mipmap_image_handles.back(), GL_WRITE_ONLY);
			}
		}

		/*
		std::vector<uint8_t> image_data;
		TextureLayout image_layout;
		//ResourceLoading::loadPpmImageRGBA("../resources/textures/debug_uv_tile_64x64.ppm", image_data, image_layout);

		image_layout.internal_format = GL_RGBA8;
		image_layout.format = GL_RGBA;
		image_layout.type = GL_UNSIGNED_BYTE;
		image_layout.width = 16;
		image_layout.height = 16;
		image_layout.depth = layers;

		//std::vector<uint8_t> array_dummy_data(image_data.size()*layers, 255);
		//for (int i = 0; i < layers; i++)
		//{
		//	std::copy(image_data.data(), image_data.data() + image_data.size(), array_dummy_data.data() + (i * image_data.size()));
		//}

		for (uint i = 0; i < num_tx_arrays; i++)
		{
			TextureLayout layout = image_layout;

			layout.depth = layers;
			layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
			layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
			layout.int_parameters.push_back({ GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER });
			layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
			layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
			//layout.int_parameters.push_back({ GL_TEXTURE_SPARSE_ARB,GL_TRUE });

			//auto tx_resource = GEngineCore::resourceManager().createTexture2DArray("brick_" + std::to_string(index) + "_ptex_texture_" + std::to_string(i), layout, array_dummy_data.data(), false);
			auto tx_resource = GEngineCore::resourceManager().createTexture2DArray("brick_" + std::to_string(index) + "_ptex_texture_" + std::to_string(i), layout, nullptr, false);
			m_bricks[index].m_ptex_textures.push_back(tx_resource.entity);

			GLenum err = glGetError();
			if (err != GL_NO_ERROR)
			{
				// "Do something cop!"
				std::cerr << "GL error during texture baking creation after texture creation: " << err << std::endl;
			}
			
			texture_handles[i] = tx_resource.resource->getTextureHandle();
			image_handles[i] = tx_resource.resource->getImageHandle(0, GL_TRUE, 0);

			tx_resource.resource->makeResident();
			glMakeImageHandleResidentARB(image_handles[i], GL_WRITE_ONLY);
		}

		*/

		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		ptex_bindless_texture_handles_resource.resource->reload(texture_handles);
		ptex_bindless_images_handles_resource.resource->reload(image_handles);
		ptex_bindless_mipmap_image_handles_resource.resource->reload(mipmap_image_handles);

		// load bindless texture handles for all texture given by ptex material to make them available during texture baking
		std::vector<GLuint64> surface_texture_handles;
		WeakResource<Material> ptex_material_resource = GEngineCore::resourceManager().getMaterial(m_bricks[index].m_ptex_material);
		for (auto texture : ptex_material_resource.resource->getTextures())
		{
			surface_texture_handles.push_back(texture->getTextureHandle());
			texture->makeResident();
		}
		WeakResource<ShaderStorageBufferObject> ptex_material_bth_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_material_bth);
		ptex_material_bth_resource.resource->reload(surface_texture_handles);

		{
			auto err = glGetError();
			if (err != GL_NO_ERROR) {
				std::cerr << "Error - bakeSurfaceTexture - 3953: " << err << std::endl;
			}
		}
		
		// Bake surface textures
		textureBaking_prgm->use();

		// Bind vertex and index buffer as storage buffer
		m_bricks[index].m_surface_mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
		m_bricks[index].m_surface_mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
		
		ptex_bindless_images_handles_resource.resource->bind(2);
		ptex_parameters_resource.resource->bind(3);

		ptex_material_bth_resource.resource->bind(4);

		ResourceID decal_buffer = GRenderingComponents::decalManager().getGPUBufferResource();
		auto decal_buffer_rsrc = GEngineCore::resourceManager().getSSBO(decal_buffer);
		decal_buffer_rsrc.resource->bind(5);
		
		textureBaking_prgm->setUniform("decal_cnt", GRenderingComponents::decalManager().getComponentCount());
		textureBaking_prgm->setUniform("texture_lod", static_cast<float>(m_bricks[index].m_lod_lvls));
		textureBaking_prgm->setUniform("layers", layers);

		int texture_base_idx = image_handles.size() - ((m_bricks[index].m_ptex_lod_bin_sizes[m_bricks[index].m_lod_lvls - 1] * material_components) / layers);
		textureBaking_prgm->setUniform("texture_base_idx", texture_base_idx);

		GLint primitive_base_idx = 0;
		GLint remaining_dispatches = primitive_cnt;
		GLint max_work_groups_z = 0;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &max_work_groups_z);

		while (remaining_dispatches > 0)
		{
			GLuint dispatchs_cnt = (remaining_dispatches > max_work_groups_z) ? max_work_groups_z : remaining_dispatches;

			textureBaking_prgm->setUniform("primitive_base_idx", primitive_base_idx);
			textureBaking_prgm->dispatchCompute(1, 1, dispatchs_cnt);

			if (remaining_dispatches > max_work_groups_z)
			{
				remaining_dispatches -= max_work_groups_z;
				primitive_base_idx += max_work_groups_z;
			}
			else
			{
				remaining_dispatches = 0;
			}
		}

		// Build vista tiles mipmaps
		for (int i = texture_base_idx; i < m_bricks[index].m_ptex_textures.size(); ++i)
		{
			auto ptex_texture_resource = GEngineCore::resourceManager().getTexture2DArray(m_bricks[index].m_ptex_textures[i]);
			
			ptex_texture_resource.resource->bindTexture();
			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
		}

		//(Re)build mipmaps...seems slow
		//for (auto ptex_texture : m_bricks[index].m_ptex_textures)
		//{
		//	auto ptex_texture_resource = GEngineCore::resourceManager().getTexture2DArray(ptex_texture);
		//
		//	ptex_texture_resource.resource->bindTexture();
		//	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
		//}
		

		// Recompute ptex update buffers to overwrite any computations done asynchrously by tasks spawned before the terrain update was called
		computePatchDistances(index);
		computeTextureTileUpdateList(index);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			// "Do something cop!"
			std::cerr << "GL error during texture baking." << err << std::endl;
		}

		// bake lod texture tiles (intial lod baking differs from updating tiles..)
		//m_bricks[index].m_ptex_ready = true;
		//updateTextureBaking(index);
		
		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
		//{
		//	std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
		//
		//computePatchDistances(index);
		//
		//GEngineCore::taskSchedueler().submitTask([this, index]() {
		//
		//	std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
		//
		//	computeTextureTileUpdateList(index);
		//
		//	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
		//	{
		//		std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
		//		updateTextureTiles(index);
		//	});
		//
		//});
		//});

		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		m_bricks[index].m_ptex_ready = true;

		std::cout << "Texture Baking - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
		
	}

	void LandscapeBrickComponentManager::computePatchDistances(uint index)
	{
		auto start_time = std::chrono::steady_clock::now();

		size_t quad_cnt = m_bricks[index].m_mesh_ptex_params.size();

		m_bricks[index].m_ptex_patch_distances.resize(quad_cnt);

		// get camera transform
		Mat4x4 model_mx = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity));
		Mat4x4 view_mx = GEngineCore::frameManager().getRenderFrame().m_view_matrix;
		Mat4x4 proj_mx = GEngineCore::frameManager().getRenderFrame().m_projection_matrix;

		for (size_t quad_idx = 0; quad_idx < quad_cnt; ++quad_idx)
		{
			uint32_t patch_indices[4];
			Vec4 patch_vertices[4];

			Vec3 midpoint(0.0f);

			for (int i = 0; i < 4; ++i)
			{
				patch_indices[i] = m_bricks[index].m_mesh_index_data[quad_idx * 4 + i];

				patch_vertices[i].x = m_bricks[index].m_mesh_vertex_data[patch_indices[i] * 6 + 0];
				patch_vertices[i].y = m_bricks[index].m_mesh_vertex_data[patch_indices[i] * 6 + 1];
				patch_vertices[i].z = m_bricks[index].m_mesh_vertex_data[patch_indices[i] * 6 + 2];
				patch_vertices[i].w = 1.0;

				//patch_vertices[i] = model_view_mx * patch_vertices[i];

				midpoint += Vec3(patch_vertices[i]);
			}

			midpoint = midpoint / 4.0f;

			LandscapeBrickComponent::PatchInfo patch_info;
			//distances[gID_x] = length(camera_position - midpoint);
			//distances[gID_x] = length((inverse(view_mx)*vec4(0.0,0.0,0.0,1.0)).xyz - midpoint);
			//patch_info.distance = glm::length(Vec3(view_mx * Vec4(midpoint, 1.0))); //TODO check model matrix
			Vec3 camera_position = Vec3(glm::inverse(view_mx)*Vec4(0.0, 0.0, 0.0, 1.0));
			camera_position.x = std::floor(camera_position.x / 4.0f) * 4.0f;
			camera_position.y = std::floor(camera_position.y / 4.0f) * 4.0f;
			camera_position.z = std::floor(camera_position.z / 4.0f) * 4.0f;
			patch_info.midpoint = midpoint;
			patch_info.distance = glm::length(camera_position - midpoint); //TODO check model matrix
			patch_info.tex_index = m_bricks[index].m_mesh_ptex_params[quad_idx].texture_index;
			patch_info.base_slice = m_bricks[index].m_mesh_ptex_params[quad_idx].base_slice;

			// "Frustrum" culling
			bool frustrum_test[4] = { false,false,false,false };

			for (int i = 0; i < 4; ++i)
			{
				// Transform patch vertices to clip space
				//vec4 cs_pos = proj_mx * view_mx * model_mx * patch_vertices[i];

				Vec4 cs_pos = proj_mx * view_mx * patch_vertices[i]; //TODO check model matrix

				cs_pos.w = cs_pos.w * 1.25;

				// Test against frustrum, if outside set very large distance
				if (cs_pos.x < -cs_pos.w || cs_pos.x > cs_pos.w)
					frustrum_test[i] = true;

				if (cs_pos.y < -cs_pos.w || cs_pos.y > cs_pos.w)
					frustrum_test[i] = true;

				if (cs_pos.z < -cs_pos.w || cs_pos.z > cs_pos.w)
					frustrum_test[i] = true;
			}

			if (frustrum_test[0] && frustrum_test[1] && frustrum_test[2] && frustrum_test[3])
				patch_info.distance = 9999.0;


			m_bricks[index].m_ptex_patch_distances[quad_idx] = patch_info;
		}

		auto t_1 = std::chrono::steady_clock::now();
		std::chrono::duration<double, std::milli> time = (t_1 - start_time);
		m_bricks[index].ptex_distance_computation_time += (time.count()) / 1000000.0;
		//std::cout << "Distance computation - " << (time.count()) / 1000000.0 << "ms" << std::endl;


		// GLuint64 t_0, t_1;
		// unsigned int queryID[2];
		// // generate two queries
		// glGenQueries(2, queryID);
		// glQueryCounter(queryID[0], GL_TIMESTAMP);
		// 
		// glMemoryBarrier(GL_ALL_BARRIER_BITS);
		// 
		// this->computePatchDistances_prgm->use();
		// 
		// // bind ptex vertex and index buffer
		// auto ptex_mesh_resource = GEngineCore::resourceManager().getMesh(m_bricks[index].m_ptex_mesh);
		// ptex_mesh_resource.resource->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
		// ptex_mesh_resource.resource->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
		// 
		// 
		// // bind distance values buffer
		// auto patch_distances_SSBO_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_patch_distances_SSBO);
		// if (patch_distances_SSBO_resource.state != READY)
		// 	return;
		// 
		// patch_distances_SSBO_resource.resource->bind(2);
		// 
		// auto ptex_params_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters);
		// ptex_params_resource.resource->bind(3);
		// 
		// // set camera position uniform
		// Mat4x4 model_mx = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(m_bricks[index].m_entity));
		// Mat4x4 view_mx = GEngineCore::frameManager().getRenderFrame().m_view_matrix;
		// Mat4x4 proj_mx = GEngineCore::frameManager().getRenderFrame().m_projection_matrix;
		// //Mat4x4 model_view_mx = view_mx *model_mx;
		// auto camera_postion = glm::inverse(view_mx) * Vec4(0.0,0.0,0.0,1.0);
		// Vec3 camera_postion_uniform_value = Vec3(camera_postion.x, camera_postion.y, camera_postion.z);
		// computePatchDistances_prgm->setUniform("camera_position", camera_postion_uniform_value);
		// computePatchDistances_prgm->setUniform("model_mx", view_mx);
		// computePatchDistances_prgm->setUniform("view_mx", view_mx);
		// computePatchDistances_prgm->setUniform("proj_mx", proj_mx);
		// 
		// 
		// glMemoryBarrier(GL_ALL_BARRIER_BITS);
		// 
		// // dispatch compute
		// uint patch_cnt = ptex_mesh_resource.resource->getIndicesCount() / 4; //Quad primitives -> 4 indices per primitive
		// computePatchDistances_prgm->setUniform("ptex_patch_cnt", patch_cnt);
		// computePatchDistances_prgm->dispatchCompute( static_cast<uint>(std::floor(patch_cnt/32)+1),1,1);
		// 
		// // map distance values buffer to CPU memory
		// m_bricks[index].m_ptex_patch_distances.resize(patch_cnt);
		// patch_distances_SSBO_resource.resource->bind();
		// GLvoid* distance_data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		// memcpy(m_bricks[index].m_ptex_patch_distances.data(), distance_data, 4/*byte*/ * 3/*elements*/ * patch_cnt);
		// glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		// 
		// 
		// glQueryCounter(queryID[1], GL_TIMESTAMP);
		// 
		// // wait until the results are available
		// GLint stopTimerAvailable = 0;
		// while (!stopTimerAvailable)
		// 	glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
		// 
		// // get query results
		// glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		// glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);
		// 
		// //std::cout << "Distance computation - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
		// m_bricks[index].ptex_distance_computation_time += (t_1 - t_0) / 1000000.0;

		// DEBUGGING
		//	for (auto v : m_bricks[index].m_ptex_patch_distances)
		//	{
		//		std::cout << v.distance << std::endl;
		//	}
	}

	void LandscapeBrickComponentManager::computeTextureTileUpdateList(uint index)
	{
		auto t_0 = std::chrono::steady_clock::now();

		// sort distance values, keep track of original indices
		size_t patch_cnt = m_bricks[index].m_ptex_patch_distances.size();
		int max_lod_lvl = m_bricks[index].m_ptex_lod_bin_sizes.size() - 1;
	
		// initialize original index locations
		std::vector<size_t> patch_indices(patch_cnt);
		std::iota(patch_indices.begin(), patch_indices.end(), 0);
	
		// sort indexes based on comparing patch distance values
		std::sort(patch_indices.begin(), patch_indices.end(),
			[this, index](size_t i1, size_t i2) {return m_bricks[index].m_ptex_patch_distances[i1].distance < m_bricks[index].m_ptex_patch_distances[i2].distance; });
	
		// DEBUGING
		//for (int i=0; i< patch_indices.size(); ++i)
		//{
		//	std::cout << m_bricks[index].m_ptex_patch_distances[patch_indices[i]] << " : " << patch_indices[i] << std::endl;
		//}
	
		// classify distance values based on LOD bin sizes
		std::vector<int> classification(patch_cnt);
		size_t remaining_patches = patch_cnt;

		m_bricks[index].m_ptex_latest_availableTiles = m_bricks[index].m_ptex_active_availableTiles;

		//std::vector<float> lod_distance_steps({ 11.5f,23.0f,45.0f,88.0f,175.0f,999999.0f }); // values measured for optimal mipmap level
		std::vector<float> lod_distance_steps({ 12.0f,24.0f,48.0f,96.0f,192.0f,999999.0f }); // values measured for optimal mipmap level
		std::vector<uint> remaining_lod_bin_size = m_bricks[index].m_ptex_lod_bin_sizes;

		int lod_bin = 0;
		for (auto patch_idx : patch_indices)
		{
			while ((remaining_lod_bin_size[lod_bin] == 0) || (m_bricks[index].m_ptex_patch_distances[patch_idx].distance >= lod_distance_steps[lod_bin]))
			{
				if (lod_bin == max_lod_lvl)
					break;

				++lod_bin;
			}

			classification[patch_idx] = lod_bin;

			remaining_lod_bin_size[lod_bin] = remaining_lod_bin_size[lod_bin] -1;
		}
	
		// compare classification with previous classification
		size_t update_patches_cnt = 0;
		std::vector<std::vector<uint>> update_patches_tgt(m_bricks[index].m_ptex_lod_bin_sizes.size());

		// Vista LoD doesnt need to be included for freed and available slots
		std::vector<std::list<LandscapeBrickComponent::TextureSlot>> freed_slots(m_bricks[index].m_ptex_active_availableTiles.size());

		for (uint i = 0; i < classification.size(); ++i)
		{
			int previous_classification = m_bricks[index].m_ptex_active_patch_lod_classification[i];

			if (classification[i] != previous_classification)
			{
				size_t patch_idx = i;
	
				update_patches_tgt[classification[i]].push_back(patch_idx); // add patch to update list with new classification

				if (previous_classification != max_lod_lvl) // if previously not vista LOD add to available texture slots
				{
					LandscapeBrickComponent::TextureSlot free_slot;
					free_slot.tex_index = m_bricks[index].m_ptex_patch_distances[i].tex_index;
					free_slot.base_slice = m_bricks[index].m_ptex_patch_distances[i].base_slice;
					//m_bricks[index].m_ptex_latest_availableTiles[previous_classification].push_back(free_slot);
					freed_slots[previous_classification].push_back(free_slot);

					//TODO assert tex index fits bin index
				}
	
				m_bricks[index].m_ptex_latest_patch_lod_classification[i] = classification[i]; // update stored classification
	
				++update_patches_cnt;
			}
		}

		// Sort update patches based on ?? to encourgage clustered assignment of texture index and base slices
		//	for (int i = 0; i < update_patches_tgt.size(); ++i)
		//	{
		//		std::sort(update_patches_tgt[i].begin(), update_patches_tgt[i].end(),[this,index](const uint & a, const uint & b) -> bool
		//		{
		//			float distance_A = std::floor(m_bricks[index].m_ptex_patch_distances[a].midpoint.x / 4.0f) * 400.0
		//				+ std::floor(m_bricks[index].m_ptex_patch_distances[a].midpoint.y / 4.0f) *40.0f
		//				+ std::floor(m_bricks[index].m_ptex_patch_distances[a].midpoint.z / 4.0f) *4.0f;
		//			float distance_B = std::floor(m_bricks[index].m_ptex_patch_distances[b].midpoint.x / 4.0f) * 400.0
		//				+ std::floor(m_bricks[index].m_ptex_patch_distances[b].midpoint.y / 4.0f) *40.0f
		//				+ std::floor(m_bricks[index].m_ptex_patch_distances[b].midpoint.z / 4.0f) *4.0f;
		//	
		//			return distance_A > distance_B;
		//	
		//			//uint32_t tex_index_A = m_bricks[index].m_ptex_patch_distances[a].tex_index;
		//			//uint32_t tex_index_B = m_bricks[index].m_ptex_patch_distances[b].tex_index;
		//			//uint32_t base_slice_A = m_bricks[index].m_ptex_patch_distances[a].base_slice;
		//			//uint32_t base_slice_B = m_bricks[index].m_ptex_patch_distances[a].base_slice;
		//	
		//			//return (tex_index_A != tex_index_B) ? (tex_index_A > tex_index_B) : (base_slice_A > base_slice_B);
		//		});
		//	}
	
		// copy update patches to continous memory
		m_bricks[index].m_ptex_updatePatches_tgt.clear();
		m_bricks[index].m_ptex_updatePatches_tgt.resize(update_patches_cnt);

		m_bricks[index].m_ptex_update_bin_sizes.clear();
		//m_bricks[index].m_ptex_update_bin_sizes.reserve(m_bricks[index].m_ptex_lod_bin_sizes.size());

		std::ostringstream bin_size_log;

		size_t tgt_copied_elements = 0;
		for (size_t i = 0; i < m_bricks[index].m_ptex_lod_bin_sizes.size(); ++i)
		{
			std::copy(update_patches_tgt[i].begin(), update_patches_tgt[i].end(), m_bricks[index].m_ptex_updatePatches_tgt.begin() + tgt_copied_elements);
	
			tgt_copied_elements += update_patches_tgt[i].size();
	
			bin_size_log << "Update bin size:" << update_patches_tgt[i].size() << std::endl;
	
			m_bricks[index].m_ptex_update_bin_sizes.push_back(update_patches_tgt[i].size());
		}

		// Sort both available and newly freed texture tile slots independent apped freed slots to available slot lists afterwards
		for (int i=0; i<freed_slots.size(); ++i)
		{
			m_bricks[index].m_ptex_latest_availableTiles[i].sort([](const LandscapeBrickComponent::TextureSlot & a, const LandscapeBrickComponent::TextureSlot & b) -> bool
							{
								return (a.tex_index != b.tex_index) ? (a.tex_index > b.tex_index) : (a.base_slice > b.base_slice);
							});

			freed_slots[i].sort([](const LandscapeBrickComponent::TextureSlot & a, const LandscapeBrickComponent::TextureSlot & b) -> bool
							{
								return (a.tex_index != b.tex_index) ? (a.tex_index > b.tex_index) : (a.base_slice > b.base_slice);
							});

			m_bricks[index].m_ptex_latest_availableTiles[i].splice(m_bricks[index].m_ptex_latest_availableTiles[i].end(), freed_slots[i]);
		}
		
		size_t available_tiles_cnt = 0;
		for (auto& tile_list : m_bricks[index].m_ptex_latest_availableTiles)
			available_tiles_cnt += tile_list.size();

		m_bricks[index].m_ptex_availableTiles_uploadBuffer.clear();
		m_bricks[index].m_ptex_availableTiles_uploadBuffer.resize(available_tiles_cnt);
		m_bricks[index].m_ptex_availableTiles_bin_sizes.clear();

		std::ostringstream available_tiles_log;

		size_t copied_elements = 0;
		for (size_t i = 0; i < m_bricks[index].m_ptex_latest_availableTiles.size(); ++i)
		{
			bin_size_log << "Available tiles:" << m_bricks[index].m_ptex_latest_availableTiles[i].size() << std::endl;

			std::copy(m_bricks[index].m_ptex_latest_availableTiles[i].begin(), m_bricks[index].m_ptex_latest_availableTiles[i].end(), m_bricks[index].m_ptex_availableTiles_uploadBuffer.begin() + copied_elements);

			copied_elements += m_bricks[index].m_ptex_latest_availableTiles[i].size();

			//available_tiles_log << "Available tiles:" << m_bricks[index].m_ptex_availableTiles[i].size() << std::endl;

			m_bricks[index].m_ptex_availableTiles_bin_sizes.push_back(m_bricks[index].m_ptex_latest_availableTiles[i].size());
		}

		if(tgt_copied_elements > 0)
			std::cout << bin_size_log.str() << available_tiles_log.str();

		// After available texture tiles are copied to upload buffer, remove as many as will be used by update patches
		for (size_t i = 0; i < m_bricks[index].m_ptex_latest_availableTiles.size(); ++i)
		{
			auto it1 = m_bricks[index].m_ptex_latest_availableTiles[i].begin();
			auto it2 = m_bricks[index].m_ptex_latest_availableTiles[i].begin();
			std::advance(it2, m_bricks[index].m_ptex_update_bin_sizes[i]);
			m_bricks[index].m_ptex_latest_availableTiles[i].erase( it1,it2 );
		}

		//TODO THREAD SAFETY 


		auto t_1 = std::chrono::steady_clock::now();
		std::chrono::duration<double, std::milli> task_time = (t_1 - t_0);
		//std::cout << "Building ptex update lists in: " << task_time.count() << std::endl;

		m_bricks[index].ptex_updateList_computation_time += task_time.count();
		m_bricks[index].ptex_updated_primitives += tgt_copied_elements;
	}
	
	
	void LandscapeBrickComponentManager::updateTextureTiles(uint index)
	{
		GLuint64 t_0, t_1;
		unsigned int queryID[2];
		// generate two queries
		glGenQueries(2, queryID);
		glQueryCounter(queryID[0], GL_TIMESTAMP);

		WeakResource<ShaderStorageBufferObject> ptex_bindless_images_handles_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_image_handles);
		WeakResource<ShaderStorageBufferObject> ptex_parameters_backbuffer_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters_backbuffer);
		WeakResource<ShaderStorageBufferObject> ptex_parameters_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters);
		WeakResource<ShaderStorageBufferObject> ptex_material_bth_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_material_bth);
		WeakResource<ShaderStorageBufferObject> updatePatches_SSBO_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_updatePatches_tgt_SSBO);
		WeakResource<ShaderStorageBufferObject> availableTiles_SSBO_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_availableTiles_SSBO);

		// check availabilty of resources and abort update if any resource is not ready
		bool ptex_update_ready = ptex_bindless_images_handles_resource.state == ResourceState::READY &&
			ptex_parameters_backbuffer_resource.state == ResourceState::READY &&
			ptex_parameters_resource.state == ResourceState::READY &&
			ptex_material_bth_resource.state == ResourceState::READY &&
			updatePatches_SSBO_resource.state == ResourceState::READY &&
			updatePatches_SSBO_resource.state == ResourceState::READY;

		uint update_patches = 0;
		for (auto bin_size : m_bricks[index].m_ptex_update_bin_sizes)
			update_patches += bin_size;

		bool lod_update_required = false;
		for (int i = 0; i < m_bricks[index].m_ptex_update_bin_sizes.size(); ++i)
		{
			lod_update_required = lod_update_required || (m_bricks[index].m_ptex_update_bin_sizes[i] > (i*16));
		}

		//ptex_update_ready = ptex_update_ready && (m_bricks[index].m_ptex_update_bin_sizes.size() > 0);
		ptex_update_ready = ptex_update_ready && (update_patches > 0);

		m_bricks[index].m_ptex_ready = true;

		if (!ptex_update_ready) return;

		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

		// copy previous ptex params to backbuffer
		//ShaderStorageBufferObject::copy(ptex_parameters_resource.resource, ptex_parameters_backbuffer_resource.resource);

		// update active patch lod classification to latest (the result of which should be used in this call)
		m_bricks[index].m_ptex_active_patch_lod_classification = m_bricks[index].m_ptex_latest_patch_lod_classification;
		m_bricks[index].m_ptex_active_availableTiles = m_bricks[index].m_ptex_latest_availableTiles;

		// upload update information to GPU
		updatePatches_SSBO_resource.resource->reload(m_bricks[index].m_ptex_updatePatches_tgt);
		availableTiles_SSBO_resource.resource->reload(m_bricks[index].m_ptex_availableTiles_uploadBuffer);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


		glQueryCounter(queryID[1], GL_TIMESTAMP);

		// wait until the results are available
		GLint stopTimerAvailable = 0;
		while (!stopTimerAvailable)
			glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

		// get query results
		glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
		glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

		m_bricks[index].ptex_tileUpdate_time += (t_1 - t_0) / 1000000.0;

		// TODO per LOD level dispatch computes
		int update_patch_offset = 0;
		int texture_slot_offset = 0;
		int tile_size_multiplier = std::pow(2, m_bricks[index].m_lod_lvls-1);

		for (int i = 0; i < static_cast<int>(m_bricks[index].m_ptex_update_bin_sizes.size()) - 1; ++i)
		{
			// available textures should always be >= update patches per LOD
			assert(m_bricks[index].m_ptex_availableTiles_bin_sizes[i] >= m_bricks[index].m_ptex_update_bin_sizes[i]);

			uint32_t bin_size = m_bricks[index].m_ptex_update_bin_sizes[i];
			float texture_lod = static_cast<float>(i);

			uint32_t remaining_tex_bin_size = m_bricks[index].m_ptex_availableTiles_bin_sizes[i];

			GLuint64 t_0, t_1;
			unsigned int queryID[2];
			// generate two queries
			glGenQueries(2, queryID);
			glQueryCounter(queryID[0], GL_TIMESTAMP);

			// set GLSL program
			updatePtexTiles_prgm->use();

			// Bind vertex and index buffer as storage buffer
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bricks[index].m_surface_mesh->getVboHandle());
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bricks[index].m_surface_mesh->getIboHandle());

			m_bricks[index].m_surface_mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
			m_bricks[index].m_surface_mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);

			ptex_bindless_images_handles_resource.resource->bind(2);
			ptex_parameters_resource.resource->bind(3);

			ptex_material_bth_resource.resource->bind(5);

			updatePatches_SSBO_resource.resource->bind(6);
			availableTiles_SSBO_resource.resource->bind(7);

			ResourceID decal_buffer = GRenderingComponents::decalManager().getGPUBufferResource();
			auto decal_buffer_rsrc = GEngineCore::resourceManager().getSSBO(decal_buffer);
			decal_buffer_rsrc.resource->bind(8);

			updatePtexTiles_prgm->setUniform("decal_cnt", GRenderingComponents::decalManager().getComponentCount());
			updatePtexTiles_prgm->setUniform("texture_lod", texture_lod+1.0f); //TODO more accurate computation of fitting mipmap level for source textures
			updatePtexTiles_prgm->setUniform("update_patch_offset", update_patch_offset);
			updatePtexTiles_prgm->setUniform("texture_slot_offset", texture_slot_offset);

			updatePtexTiles_prgm->dispatchCompute(tile_size_multiplier, tile_size_multiplier, bin_size);
			
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			glQueryCounter(queryID[1], GL_TIMESTAMP);

			// wait until the results are available
			GLint stopTimerAvailable = 0;
			while (!stopTimerAvailable)
				glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

			// get query results
			glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
			glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

			m_bricks[index].ptex_tileUpdate_time += (t_1 - t_0) / 1000000.0;
			
			update_patch_offset += bin_size;
			texture_slot_offset += bin_size;

			remaining_tex_bin_size -= bin_size;

			texture_slot_offset += remaining_tex_bin_size;

			tile_size_multiplier /= 2;
		}

		
		// Assign vista tiles (no recomutation necessary)
		if (m_bricks[index].m_ptex_update_bin_sizes.back() > 0)
		{
			setPtexVistaTiles_prgm->use();

			ptex_parameters_resource.resource->bind(3);
			updatePatches_SSBO_resource.resource->bind(6);

			setPtexVistaTiles_prgm->setUniform("update_patch_offset", update_patch_offset);

			int texture_base_idx = m_bricks[index].m_ptex_textures.size() - (m_bricks[index].m_ptex_lod_bin_sizes.back() * 4) / 2048;
			setPtexVistaTiles_prgm->setUniform("texture_base_idx", texture_base_idx);
			setPtexVistaTiles_prgm->setUniform("vista_patch_cnt", m_bricks[index].m_ptex_update_bin_sizes.back());

			setPtexVistaTiles_prgm->dispatchCompute(1, 1, (m_bricks[index].m_ptex_update_bin_sizes.back() / 32) + 1);

			//	for (auto texture : m_bricks[index].m_ptex_textures)
				//	{
				//		auto tex_rsrc = GEngineCore::resourceManager().getTexture2DArray(texture);
				//		tex_rsrc.resource->updateMipmaps();
				//	}
		}

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// copy updated ptex params to cpu
		size_t byte_size = ptex_parameters_resource.resource->getSize();
		//m_bricks[index].m_mesh_ptex_params.resize(quad_cnt); // 1 set of ptex params per quad
		ptex_parameters_resource.resource->bind();
		GLvoid* ptex_params = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byte_size, GL_MAP_READ_BIT);
		memcpy(m_bricks[index].m_mesh_ptex_params.data(), ptex_params, byte_size);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			

		// update average and maximum time
		m_bricks[index].ptex_distance_computation_avg_time += m_bricks[index].ptex_distance_computation_time;
		m_bricks[index].ptex_distance_computation_max_time = std::max(m_bricks[index].ptex_distance_computation_max_time, m_bricks[index].ptex_distance_computation_time);
		m_bricks[index].ptex_distance_computation_time = 0.0;

		m_bricks[index].ptex_updateList_computation_avg_time += m_bricks[index].ptex_updateList_computation_time;
		m_bricks[index].ptex_updateList_computation_max_time = std::max(m_bricks[index].ptex_updateList_computation_max_time, m_bricks[index].ptex_updateList_computation_time);
		m_bricks[index].ptex_updateList_computation_time = 0.0;

		m_bricks[index].ptex_tileUpdate_avg_time += m_bricks[index].ptex_tileUpdate_time;
		m_bricks[index].ptex_tileUpdate_max_time = std::max(m_bricks[index].ptex_tileUpdate_max_time, m_bricks[index].ptex_tileUpdate_time);
		m_bricks[index].ptex_tileUpdate_time = 0.0;

		m_bricks[index].ptex_updated_primitives_avg += m_bricks[index].ptex_updated_primitives;
		m_bricks[index].ptex_updated_primitives_max = std::max(m_bricks[index].ptex_updated_primitives_max, m_bricks[index].ptex_updated_primitives);
		m_bricks[index].ptex_updated_primitives = 0;

		++m_bricks[index].updates_made;

		// sum up measurements...output average and max time once a whole of 5seconds of update time has been gathered
		if ( (m_bricks[index].ptex_distance_computation_avg_time + m_bricks[index].ptex_updateList_computation_avg_time + m_bricks[index].ptex_tileUpdate_avg_time) > 5000)
		{
			double ptex_update_avg_time = (m_bricks[index].ptex_distance_computation_avg_time + m_bricks[index].ptex_updateList_computation_avg_time + m_bricks[index].ptex_tileUpdate_avg_time) / m_bricks[index].updates_made;
			double ptex_update_max_time = m_bricks[index].ptex_distance_computation_max_time + m_bricks[index].ptex_updateList_computation_max_time + m_bricks[index].ptex_tileUpdate_max_time;
			std::cout << "============================" << std::endl;
			std::cout << "Ptex Distance Computation (avg): " << m_bricks[index].ptex_distance_computation_avg_time/ m_bricks[index].updates_made << std::endl;
			std::cout << "Ptex Distance Computation (max): " << m_bricks[index].ptex_distance_computation_max_time << std::endl;
			std::cout << "Ptex Classification (avg): " << m_bricks[index].ptex_updateList_computation_avg_time / m_bricks[index].updates_made << std::endl;
			std::cout << "Ptex Classification (max): " << m_bricks[index].ptex_updateList_computation_max_time << std::endl;
			std::cout << "Ptex Texture Update (avg): " << m_bricks[index].ptex_tileUpdate_avg_time / m_bricks[index].updates_made << std::endl;
			std::cout << "Ptex Texture Update (max): " << m_bricks[index].ptex_tileUpdate_max_time << std::endl;
			std::cout << "Ptex Update Average Time: " << ptex_update_avg_time << std::endl;
			std::cout << "Ptex Update Max Time: " << ptex_update_max_time << std::endl;
			std::cout << "Ptex Primitives (avg): " << m_bricks[index].ptex_updated_primitives_avg / m_bricks[index].updates_made << std::endl;
			std::cout << "Ptex Primitives (max): " << m_bricks[index].ptex_updated_primitives_max << std::endl;
			std::cout << "============================" << std::endl;

			m_bricks[index].ptex_distance_computation_avg_time = 0.0;
			m_bricks[index].ptex_distance_computation_max_time = 0.0;
			m_bricks[index].ptex_updateList_computation_avg_time = 0.0;
			m_bricks[index].ptex_updateList_computation_max_time = 0.0;
			m_bricks[index].ptex_tileUpdate_avg_time = 0.0;
			m_bricks[index].ptex_tileUpdate_max_time = 0.0;
			m_bricks[index].ptex_updated_primitives_avg = 0;
			m_bricks[index].ptex_updated_primitives_max = 0;

			m_bricks[index].updates_made = 0;
		}

		//TODO add GPU task for mipmap computation
		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
		//{
		//if (m_bricks[index].m_cancel_ptex_update)
		//{
		//	return;
		//}
		{
			WeakResource<ShaderStorageBufferObject> texture_handles_rsrc = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_texture_handles);
			WeakResource<ShaderStorageBufferObject> mipmap_image_handles_rsrc = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_mipmap_image_handles);
			WeakResource<ShaderStorageBufferObject> ptex_params_rsrc = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters);
			WeakResource<ShaderStorageBufferObject> updatePatches_rsrc = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_updatePatches_tgt_SSBO);
			
			texture_handles_rsrc.resource->bind(0);
			mipmap_image_handles_rsrc.resource->bind(1);
			ptex_params_rsrc.resource->bind(2);
			updatePatches_rsrc.resource->bind(3);
			
			int update_patch_offset = 0;
			int tile_size_multiplier = std::pow(2, m_bricks[index].m_lod_lvls - 1);
			
			updatePtexTilesMipmaps_prgm->use();
			
			// only update mipmaps of non-vista level tiles
			for (int i = 0; i < static_cast<int>(m_bricks[index].m_ptex_update_bin_sizes.size()) - 1; ++i)
			{
				uint32_t bin_size = m_bricks[index].m_ptex_update_bin_sizes[i];
			
				updatePtexTilesMipmaps_prgm->setUniform("update_patch_offset", update_patch_offset);
			
				updatePtexTilesMipmaps_prgm->dispatchCompute(tile_size_multiplier, tile_size_multiplier, bin_size);
			
				tile_size_multiplier /= 2;
				update_patch_offset += bin_size;
			}
		}
		//});

		// clear update bin size to avoid as updates of the latest classification have been applied
		m_bricks[index].m_ptex_update_bin_sizes.clear();
		
	}
	

/*
	void LandscapeBrickComponentManager::updateTextureTiles(uint index)
	{
		WeakResource<ShaderStorageBufferObject> ptex_bindless_texture_handles_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_texture_handles);
		WeakResource<ShaderStorageBufferObject> ptex_bindless_images_handles_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_bindless_image_handles);
		WeakResource<ShaderStorageBufferObject> ptex_parameters_backbuffer_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters_backbuffer);
		WeakResource<ShaderStorageBufferObject> ptex_parameters_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_parameters);
		WeakResource<ShaderStorageBufferObject> ptex_material_bth_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_material_bth);
		WeakResource<ShaderStorageBufferObject> updatePatches_SSBO_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_updatePatches_tgt_SSBO);
		WeakResource<ShaderStorageBufferObject> availableTiles_SSBO_resource = GEngineCore::resourceManager().getSSBO(m_bricks[index].m_ptex_availableTiles_SSBO);
	
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	
		// copy previous ptex params to backbuffer
		ShaderStorageBufferObject::copy(ptex_parameters_resource.resource, ptex_parameters_backbuffer_resource.resource);
	
		// upload update information to GPU
		updatePatches_SSBO_resource.resource->reload(m_bricks[index].m_ptex_updatePatches_tgt);
		availableTiles_SSBO_resource.resource->reload(m_bricks[index].m_ptex_availableTiles_uploadBuffer);
	
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
		// TODO per LOD level dispatch computes
		int update_patch_offset = 0;
		int texture_slot_offset = 0;
		int tile_size_multiplier = 32;
	
		for (size_t i = 0; i < m_bricks[index].m_ptex_update_bin_sizes.size() - 1; ++i)
		{
			// available textures should always be >= update patches per LOD
			assert(m_bricks[index].m_ptex_availableTiles_bin_sizes[i] >= m_bricks[index].m_ptex_update_bin_sizes[i]);
	
			uint32_t bin_size = m_bricks[index].m_ptex_update_bin_sizes[i];
			float texture_lod = static_cast<float>(i);
	
			// TODO split bin size depending on amount of texels to avoid single large tasks
	
			uint32_t update_texel_cnt = bin_size * tile_size_multiplier * tile_size_multiplier * 8 * 8;
			int sub_tasks = std::max(1, int(update_texel_cnt / 2000000));
			sub_tasks = 1; // Splitting into sub tasks currently produces buggy surface tiles
	
			uint32_t sub_bin_size = bin_size / sub_tasks;
			uint32_t remaining_bin_size = bin_size;
	
			uint32_t remaining_tex_bin_size = m_bricks[index].m_ptex_availableTiles_bin_sizes[i];
	
			for (int j = 0; j < sub_tasks; ++j)
			{
				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this,
					index,
					texture_lod,
					sub_bin_size,
					remaining_bin_size,
					tile_size_multiplier,
					update_patch_offset,
					texture_slot_offset,
					ptex_bindless_images_handles_resource,
					ptex_bindless_texture_handles_resource,
					ptex_parameters_resource,
					ptex_parameters_backbuffer_resource,
					updatePatches_SSBO_resource,
					ptex_material_bth_resource,
					availableTiles_SSBO_resource]()
				{
					GLuint64 t_0, t_1;
					unsigned int queryID[2];
					// generate two queries
					glGenQueries(2, queryID);
					glQueryCounter(queryID[0], GL_TIMESTAMP);
	
					// Bind all SSBOs
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bricks[index].m_surface_mesh->getVboHandle());
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bricks[index].m_surface_mesh->getIboHandle());
					ptex_bindless_images_handles_resource.resource->bind(2);
					ptex_parameters_resource.resource->bind(3);
					ptex_material_bth_resource.resource->bind(5);
					updatePatches_SSBO_resource.resource->bind(6);
					availableTiles_SSBO_resource.resource->bind(7);
					ptex_bindless_texture_handles_resource.resource->bind(8);

					updatePtexTilesDisplacement_prgm->use();

					updatePtexTilesDisplacement_prgm->setUniform("texture_lod", texture_lod + 1.0f); //TODO more accurate computation of fitting mipmap level for source textures
					updatePtexTilesDisplacement_prgm->setUniform("update_patch_offset", update_patch_offset);
					updatePtexTilesDisplacement_prgm->setUniform("texture_slot_offset", texture_slot_offset);

					uint32_t task_bin_size = std::min(sub_bin_size, remaining_bin_size);
					updatePtexTilesDisplacement_prgm->dispatchCompute(tile_size_multiplier, tile_size_multiplier, task_bin_size);

					glMemoryBarrier(GL_ALL_BARRIER_BITS);

					// set GLSL program
					updatePtexTilesTextures_prgm->use();
	
					updatePtexTilesTextures_prgm->setUniform("texture_lod", texture_lod + 1.0f); //TODO more accurate computation of fitting mipmap level for source textures
					updatePtexTilesTextures_prgm->setUniform("update_patch_offset", update_patch_offset);
					updatePtexTilesTextures_prgm->setUniform("texture_slot_offset", texture_slot_offset);
	
					task_bin_size = std::min(sub_bin_size, remaining_bin_size);
					updatePtexTilesTextures_prgm->dispatchCompute(tile_size_multiplier, tile_size_multiplier, task_bin_size);
	
					glMemoryBarrier(GL_ALL_BARRIER_BITS);
	


					glQueryCounter(queryID[1], GL_TIMESTAMP);
	
					// wait until the results are available
					GLint stopTimerAvailable = 0;
					while (!stopTimerAvailable)
						glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
	
					// get query results
					glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
					glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);
	
					//std::cout << "Updating " << task_bin_size << " surface " << tile_size_multiplier * 8 << "x" << tile_size_multiplier * 8 << " patches in ";
					//std::cout << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
				});
	
				update_patch_offset += std::min(sub_bin_size, remaining_bin_size);
				texture_slot_offset += std::min(sub_bin_size, remaining_bin_size);
	
				remaining_tex_bin_size -= std::min(sub_bin_size, remaining_bin_size);
				remaining_bin_size -= sub_bin_size;
			}
	
			texture_slot_offset += remaining_tex_bin_size;
	
			tile_size_multiplier /= 2;
		}
	
	
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index, ptex_parameters_resource, updatePatches_SSBO_resource, update_patch_offset]()
		{
			// Assign vista tiles (no recomutation necessary)
			if (m_bricks[index].m_ptex_update_bin_sizes.back() > 0)
			{
				setPtexVistaTiles_prgm->use();
	
				ptex_parameters_resource.resource->bind(3);
				updatePatches_SSBO_resource.resource->bind(6);
	
				setPtexVistaTiles_prgm->setUniform("update_patch_offset", update_patch_offset);
	
				int texture_base_idx = m_bricks[index].m_ptex_textures.size() - (m_bricks[index].m_ptex_lod_bin_sizes.back() * 4) / 2048;
				setPtexVistaTiles_prgm->setUniform("texture_base_idx", texture_base_idx);
				setPtexVistaTiles_prgm->setUniform("vista_patch_cnt", m_bricks[index].m_ptex_update_bin_sizes.back());
	
				setPtexVistaTiles_prgm->dispatchCompute(1, 1, (m_bricks[index].m_ptex_update_bin_sizes.back() / 32) + 1);
			}
	
			m_ptex_update_in_progress.clear();
		});
	
	}
*/

	void LandscapeBrickComponentManager::updateTextureBaking(uint index)
	{
		// don't start additional update task if some task are even already working on the data
		//{
		//	std::unique_lock<std::mutex> ptex_lock(m_ptex_update, std::try_to_lock);
		//	if (!ptex_lock.owns_lock()) return;
		//}

		if (!m_bricks[index].m_ptex_ready)
			return;
		
		m_bricks[index].m_ptex_ready = false;

		GEngineCore::taskSchedueler().submitTask([this, index]() {

			std::unique_lock<std::mutex> ptex_lock(m_ptex_update);

			computePatchDistances(index);

			computeTextureTileUpdateList(index);

			//	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
			//	{
			//		std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
			//	
			//		updateTextureTiles(index);
			//	});

		});
	}

	void LandscapeBrickComponentManager::fillSurfaceMeshGaps(uint index)
	{
		// TOOD Find pair of neighbouring bricks

		// TODO Find relevant textures

		// TODO Find required resolution

		// TODO per pair

			// TODO Dispatch compute call to count required triangles

			// TODO Resize mesh buffer

			// TODO Create mesh from compute shader
	}

	void LandscapeBrickComponentManager::addDebugVolume(uint index)
	{
		//////////////////////
		// Create Debug Volume
		//////////////////////

		glUseProgram(0);

		m_bricks[index].m_normals->bindTexture();
		std::vector<float> volume_data(m_bricks[index].m_normals->getWidth()
										* m_bricks[index].m_normals->getHeight()
										* m_bricks[index].m_normals->getDepth() * 4);
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, volume_data.data());
		TextureLayout debug_volume_descriptor(GL_RGBA32F, m_bricks[index].m_normals->getWidth(), m_bricks[index].m_normals->getHeight(), m_bricks[index].m_normals->getDepth(), GL_RGBA, GL_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});


		//cube_indices.bindTexture();
		//histogram_pyramid[5]->bindTexture();
		//m_bricks[index].m_surface->bindTexture();
		//std::vector<GLhalf> volume_data( m_bricks[index].m_surface->getWidth() * m_bricks[index].m_surface->getHeight() * m_bricks[index].m_surface->getDepth() );
		//glGetTexImage(GL_TEXTURE_3D,0,GL_RED,GL_HALF_FLOAT,volume_data.data());
		//TextureDescriptor debug_volume_descriptor(GL_R16F,m_bricks[index].m_surface->getWidth(),m_bricks[index].m_surface->getHeight(),m_bricks[index].m_surface->getDepth(),GL_RED,GL_HALF_FLOAT);

		//m_bricks[index].m_noise_params->bindTexture();
		//std::vector<GLhalf> volume_data( 2 * m_bricks[index].m_noise_params->getWidth()*m_bricks[index].m_noise_params->getHeight()*m_bricks[index].m_noise_params->getDepth());
		//glGetTexImage(GL_TEXTURE_3D, 0, GL_RG, GL_HALF_FLOAT, volume_data.data());
		//TextureDescriptor debug_volume_descriptor(GL_RG16F, m_bricks[index].m_noise_params->getWidth(), m_bricks[index].m_noise_params->getHeight(), m_bricks[index].m_noise_params->getDepth(), GL_RG, GL_HALF_FLOAT);

		float x = m_bricks[index].m_dimensions.x / 2.0f;
		float y = m_bricks[index].m_dimensions.y / 2.0f;
		float z = m_bricks[index].m_dimensions.z / 2.0f;
		std::vector<float> debug_volume_bg_vertices({ -x,-y,-z,0.0,0.0,0.0,
			-x,y,-z,0.0,1.0,0.0,
			x,y,-z,1.0,1.0,0.0,
			x,-y,-z,1.0,0.0,0.0,
			-x,-y,z,0.0,0.0,1.0,
			-x,y,z,0.0,1.0,1.0,
			x,y,z,1.0,1.0,1.0,
			x,-y,z,1.0,0.0,1.0, });
		std::vector<uint> debug_volume_bg_indices({ 0,2,1,0,3,2, //back face
			0,1,4,1,5,4,
			4,5,7,7,5,6,
			7,6,3,3,2,6,
			5,1,6,6,1,2,
			4,7,0,0,7,3 });
		VertexLayout debug_volume_bg_vertexDesc(24, { VertexLayout::Attribute(GL_FLOAT,3,false,0),VertexLayout::Attribute(GL_FLOAT,3,false,3 * sizeof(GLfloat)) });

		GRenderingComponents::volumeManager().addComponent(m_bricks[index].m_entity, "brick"+std::to_string(m_bricks[index].m_entity.id())+"_volume",
			volume_data, debug_volume_descriptor,
			debug_volume_bg_vertices, debug_volume_bg_indices, debug_volume_bg_vertexDesc, GL_TRIANGLES,
			Vec3(-x, -y, -z), Vec3(x, y, z),false);
	}

	void LandscapeBrickComponentManager::updateDebugVolume(uint index)
	{
		glUseProgram(0);

		Texture3D* active_debug_field;

		switch (m_bricks[index].m_debugField_selection)
		{
		case Datafield::NORMAL:
			active_debug_field = m_bricks[index].m_normals;
			break;
		case Datafield::GRADIENT:
			active_debug_field = m_bricks[index].m_gradients;
			break;
		case Datafield::NOISE:
			active_debug_field = m_bricks[index].m_noise_params;
			break;
		case Datafield::SURFACE:
			active_debug_field = m_bricks[index].m_surface;
			break;
		case Datafield::SURFACE_BOUNDARY:
			active_debug_field = m_bricks[index].m_surface_boundaryRegion;
			break;
		default:
			active_debug_field = m_bricks[index].m_normals;
			break;
		}

		active_debug_field->bindTexture();
		std::vector<float> volume_data(active_debug_field->getWidth()*
			active_debug_field->getHeight()*
			active_debug_field->getDepth() * 4);
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, volume_data.data());
		TextureLayout debug_volume_descriptor(GL_RGBA32F, active_debug_field->getWidth(), active_debug_field->getHeight(), active_debug_field->getDepth(), GL_RGBA, GL_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
			std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
			std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

		GRenderingComponents::volumeManager().updateComponent(m_bricks[index].m_entity, volume_data, debug_volume_descriptor);
	}

	void LandscapeBrickComponentManager::exportMesh(uint index, std::string export_filepath)
	{
		// use transformFeedback to write tesselated terrain surface to GPU-buffer

		// allocate buffer size
		size_t byte_size = m_bricks[index].m_surface_mesh->getIndicesCount() * 3 * sizeof(float) * 2 * 10; //Last multiplication factor is the amount of tessellated triangles
		glBindBuffer(GL_ARRAY_BUFFER, transformFeedback_terrainBuffer);
		glBufferData(GL_ARRAY_BUFFER, byte_size, 0, GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnable(GL_RASTERIZER_DISCARD);
		
		transformFeedback_terrainOutput_prgm->use();

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, transformFeedback_terrainBuffer);
		glBeginTransformFeedback(GL_TRIANGLES);

		m_bricks[index].m_surface_mesh->draw();

		glEndTransformFeedback();
		glDisable(GL_RASTERIZER_DISCARD);


		// read GPU-buffer to CPU memory
		float* mesh_data = new float[byte_size / sizeof(float)];
		
		glBindBuffer(GL_ARRAY_BUFFER, transformFeedback_terrainBuffer);
		glGetBufferSubData(GL_ARRAY_BUFFER, 0, byte_size, mesh_data);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		std::cout << "Vertex 0: " << mesh_data[0] << " " << mesh_data[1] << " " << mesh_data[2] << std::endl;

		// create output obj file from GPU-buffer
		std::ofstream file(export_filepath, std::ios::out | std::ios::binary);

		file << "# Space-Lion Terrain Export\n";
		file << "o Terrain\n";

		for (uint i = 0; i < (byte_size / 4); i = i + 6)
		{
			file << "v " << mesh_data[i] << " " << mesh_data[i + 1] << " " << mesh_data[i + 2] << "\n";
		}

		for (uint i = 0; i < (byte_size / 4); i = i + 6)
		{
			file << "vn " << mesh_data[i+3] << " " << mesh_data[i + 4] << " " << mesh_data[i + 5] << "\n";
		}

		file << "usemtl None\n";
		file << "s off\n";

		uint vertex_counter = 1;
		for (uint i = 0; i < (byte_size / 4); i = i + 6*3)
		{
			file << "f " << vertex_counter<<"//"<<vertex_counter << " " << vertex_counter+1<< "//" << vertex_counter+1<< " " << vertex_counter+2<< "//" << vertex_counter+2<< "\n";

			vertex_counter = vertex_counter + 3;
		}

		file.close();
	}

	void LandscapeBrickComponentManager::setDebugField(uint index, Datafield field_selection)
	{
		if (field_selection == m_bricks[index].m_debugField_selection)
			return;

		m_bricks[index].m_debugField_selection = field_selection;

		updateDebugVolume(index);
	}

	void LandscapeBrickComponentManager::setSurfaceMaterial(uint index, Material* mat)
	{
		m_bricks[index].m_surface_material = mat;

		GRenderingComponents::staticMeshManager().updateComponent(m_bricks[index].m_entity,mat);
	}

	void LandscapeBrickComponentManager::setVoxelizeFeatureCurves(bool voxelize)
	{
		m_voxelize_featureCurves = voxelize;
	}

	void LandscapeBrickComponentManager::setVoxelizeFeatureMeshes(bool voxelize)
	{
		m_voxelize_featureMeshes = voxelize;
	}

	void LandscapeBrickComponentManager::setVoxelizeHeightmaps(bool voxelize)
	{
		m_voxelize_heightmaps = voxelize;
	}

	void LandscapeBrickComponentManager::updateShaderPrograms()
	{
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this]
		{
			this->voxelize_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_gather_c.glsl" }, "lcsp_voxelize").resource;
			this->voxelize_mesh_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelize_mesh_gather_c.glsl" }, "lcsp_voxelize_mesh").resource;
			this->voxelize_heightmapMesh_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelize_heightmapMesh_gather_c.glsl" }, "lcsp_voxelize_heightmap").resource;
			this->average_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_average_c.glsl" }, "lcsp_voxelize_average").resource;
			this->reset_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/voxelization_reset_c.glsl" }, "lcsp_reset").resource;
			this->buildGuidance_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/buildGuidanceField_c.glsl" }, "lcsp_buildGuidance").resource;

			this->buildNoiseField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/buildNoiseField_c.glsl" }, "lcsp_buildNoiseField").resource;
			this->copyNoiseField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/copyTexture3D_RGBA16_c.glsl" }, "lcsp_copyNoiseField").resource;

			//this->surfacePropagation_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_pull_c.glsl" }).get();
			this->surfacePropagationInit_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_initBoundaryRegion_c.glsl" }, "lcsp_surfacePropagationInit").resource;
			this->surfacePropagation_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/propagate_distApprox_c.glsl" }, "lcsp_surfacePropagation").resource;
			this->copySurfaceField_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/copyTexture3D_R16_c.glsl" }, "lcsp_copySurfaceField").resource;
			this->smooth_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/seperatedGaussian3d_c.glsl" }, "lcsp_smooth").resource;

			this->classifyVoxels_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_classify_c.glsl" }, "lcsp_classifyVoxels").resource;

			// Generate prgms for different datatypes on different levels by insertig define files
			this->buildHpLvl_R8_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, ("#define SRC_R8UI\n#define TGT_R8UI\n"), "lcsp_buildHpLvl_R8").resource;
			this->buildHpLvl_R8toR16_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R8UI\n#define TGT_R16UI\n", "lcsp_buildHpLvl_R8toR16").resource;
			this->buildHpLvl_R16_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R16UI\n#define TGT_R16UI\n", "lcsp_buildHpLvl_R16").resource;
			this->buildHpLvl_R16toR32_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R16UI\n#define TGT_R32UI\n", "lcsp_buildHpLvl_R16to32").resource;
			this->buildHpLvl_R32_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl" }, "#define SRC_R32UI\n#define TGT_R32UI\n", "lcsp_buildHpLvl_R32").resource;

			this->generateTriangles_prgms = new GLSLProgram*[5];
			this->generateTriangles_prgms[0] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 6\n", "lcsp_generateTriangles_L6").resource;
			this->generateTriangles_prgms[1] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 7\n", "lcsp_generateTriangles_L7").resource;
			this->generateTriangles_prgms[2] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 8\n", "lcsp_generateTriangles_L8").resource;
			this->generateTriangles_prgms[3] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 9\n", "lcsp_generateTriangles_L9").resource;
			this->generateTriangles_prgms[4] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 10\n", "lcsp_generateTriangles_L10").resource;

			// Create surface nets programs
			this->surfaceNets_classify_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/surfaceNets_classify_c.glsl" }, "surfaceNets_classify").resource;
			this->surfaceNets_generateQuads_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/surfaceNets_generateQuads_c.glsl" }, "surfaceNets_generateQuads").resource;
			this->computePtexNeighbours_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/computePtexNeighbours_c.glsl" }, "landscape_computePtexNeighbours").resource;
			this->computePatchDistances_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/computePatchDistances_c.glsl" }, "landscape_computePatchDistances").resource;
			this->updateTextureTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updateTextureTiles_c.glsl" }, "landscape_updateTextureTiles").resource;
			this->textureBaking_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/textureBaking_c.glsl" }, "landscape_textureBaking").resource;
			this->setInitialLODTextureTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/set_initial_LOD_textureTiles_c.glsl" }, "landscape_initialLODTextureTiles").resource;
			this->setPtexVistaTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/setPtexVistaTiles_c.glsl" }, "landscape_setPtexVistaTiles").resource;
			this->updatePtexTiles_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTiles_c.glsl" }, "landscape_updatePtexTiles").resource;
			this->updatePtexTilesDisplacement_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesDisplacement_c.glsl" }, "lscp_updatePtexTilesDisplacement").resource;
			this->updatePtexTilesTextures_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/updatePtexTilesTextures_c.glsl" }, "lscp_updatePtexTilesTextures").resource;
		});
	}


	StaticLandscapeComponentManager::StaticLandscapeComponentManager()
	{
		//GEngineCore::renderingPipeline().addPerFrameInterfaceGpuTask(std::bind(&StaticLandscapeComponentManager::drawDebugInterface, this));
	}

	StaticLandscapeComponentManager::~StaticLandscapeComponentManager()
	{
	}

	uint StaticLandscapeComponentManager::getIndex(Entity e)
	{
		auto search = m_index_map.find(e.id());

		assert((search != m_index_map.end()));

		return search->second;
	}

	std::pair<bool, uint> StaticLandscapeComponentManager::getIndex(uint entity_id)
	{
		auto search = m_index_map.find(entity_id);

		if(search != m_index_map.end())
			return std::pair<bool,uint>(true,search->second);
		else
			return std::pair<bool, uint>(false, -1);
	}

	Entity StaticLandscapeComponentManager::getEntity(uint index)
	{
		if (index >= m_landscapes.size())
			return GEngineCore::entityManager().invalidEntity();

		return m_landscapes[index].m_entity;
	}

	void StaticLandscapeComponentManager::addComponent(Entity entity,
														Vec3 world_position,
														Vec3 dimensions,
														uint num_bricks_x,
														uint num_bricks_y,
														uint num_bricks_z,
														uint brick_resX,
														uint brick_resY,
														uint brick_resZ)
	{
		m_landscapes.push_back(StaticLandscapeComponent(entity,dimensions));
		GCoreComponents::transformManager().addComponent(entity,world_position);

		uint landscape_idx = static_cast<uint>(m_landscapes.size() - 1);

		m_index_map.insert(std::pair<uint,uint>( entity.id() , landscape_idx));

		Vec3 landscape_origin = world_position - (dimensions/2.0f);

		for(uint z=0; z<num_bricks_z; z++)
		{
			for(uint y=0; y<num_bricks_y; y++)
			{
				for(uint x=0; x<num_bricks_x; x++)
				{
					Entity new_brick_entity = GEngineCore::entityManager().create();

					Vec3 brick_dimensions = Vec3(dimensions.x/(float)num_bricks_x,
												dimensions.y/(float)num_bricks_y,
												dimensions.z/(float)num_bricks_z);

					Vec3 brick_position = landscape_origin
											+ brick_dimensions/2.0f
											+ brick_dimensions * Vec3(x,y,z);

					/*
					// From Hacker's Delight...get the previous power of two
					unsigned int res_x = static_cast<unsigned int>(floor(brick_dimensions.x));
					res_x |= res_x >> 1;
					res_x |= res_x >> 2;
					res_x |= res_x >> 4;
					res_x |= res_x >> 8;
					res_x |= res_x >> 16;
					res_x -= (res_x >>1);

					unsigned int res_y = static_cast<unsigned int>(floor(brick_dimensions.y));
					res_y |= res_y >> 1;
					res_y |= res_y >> 2;
					res_y |= res_y >> 4;
					res_y |= res_y >> 8;
					res_y |= res_y >> 16;
					res_y -= (res_y >> 1);

					unsigned int res_z = static_cast<unsigned int>(floor(brick_dimensions.z));
					res_z |= res_z >> 1;
					res_z |= res_z >> 2;
					res_z |= res_z >> 4;
					res_z |= res_z >> 8;
					res_z |= res_z >> 16;
					res_z -= (res_z >> 1);

					std::cout << "Res X:" << res_x << std::endl;
					std::cout << "Res_Y:" << res_y << std::endl;
					std::cout << "Res_Z:" << res_z << std::endl;
					

					GLandscapeComponents::brickManager().addComponent(new_brick_entity,brick_position,brick_dimensions, res_x +1, res_y +1, res_z +1);
					*/

					GLandscapeComponents::brickManager().addComponent(new_brick_entity, brick_position, brick_dimensions, brick_resX + 1, brick_resY + 1, brick_resZ + 1);

					m_landscapes.back().m_bricks.push_back(new_brick_entity);
				}
			}
		}

		// got to make sure that neighbours are set after resource creation...quick hack: let the graphics thread do it


		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, num_bricks_x, num_bricks_y, num_bricks_z] {
			// set neighbour bricks
			uint current_brick = 0;
			for (uint z = 0; z < num_bricks_z; z++)
			{
				for (uint y = 0; y < num_bricks_y; y++)
				{
					for (uint x = 0; x < num_bricks_x; x++)
					{
						uint brick_idx = GLandscapeComponents::brickManager().getIndex(m_landscapes.back().m_bricks[current_brick]);
						// west neighbour
						if (x == 0)
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::WEST, m_landscapes.back().m_bricks[current_brick]);
						else
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::WEST, m_landscapes.back().m_bricks[current_brick-1]);

						// east neighbour
						if (x == num_bricks_x-1)
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::EAST, m_landscapes.back().m_bricks[current_brick]);
						else
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::EAST, m_landscapes.back().m_bricks[current_brick + 1]);

						// lower neighbour
						if (y == 0)
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::DOWN, m_landscapes.back().m_bricks[current_brick]);
						else
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::DOWN, m_landscapes.back().m_bricks[current_brick - num_bricks_x]);

						// upper neighbour
						if (y == num_bricks_y-1)
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::UP, m_landscapes.back().m_bricks[current_brick]);
						else
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::UP, m_landscapes.back().m_bricks[current_brick + num_bricks_x]);

						// south neighbour
						if (z == 0)
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::SOUTH, m_landscapes.back().m_bricks[current_brick]);
						else
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::SOUTH, m_landscapes.back().m_bricks[current_brick - (num_bricks_x*num_bricks_y)]);

						// north neighbour
						if (z == num_bricks_z-1)
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::NORTH, m_landscapes.back().m_bricks[current_brick]);
						else
							GLandscapeComponents::brickManager().setNeighbour(brick_idx, LandscapeBrickComponentManager::NeighbourDirection::NORTH, m_landscapes.back().m_bricks[current_brick + (num_bricks_x*num_bricks_y)]);

						current_brick++;
					}
				}
			}

		});

	}

	void StaticLandscapeComponentManager::deleteComponent(uint index)
	{
		for(auto& featureCurve : m_landscapes[index].m_featureCurves)
			GLandscapeComponents::featureCurveManager().deleteComponent(GLandscapeComponents::featureCurveManager().getIndex(featureCurve));

		for(auto& brick : m_landscapes[index].m_bricks)
			GLandscapeComponents::brickManager().deleteComponent(GLandscapeComponents::brickManager().getIndex(brick));

		// TODO remove landscape from list
	}

	Entity StaticLandscapeComponentManager::addFeatureCurve(bool is_surface_seed, Vec3 position, Quat orientation)
	{
		Entity featureCurve = GEngineCore::entityManager().create();
		GLandscapeComponents::featureCurveManager().addComponent(featureCurve,position,orientation, is_surface_seed);

		uint curve_idx = GLandscapeComponents::featureCurveManager().getIndex(featureCurve);

		for (auto& lcsp : m_landscapes)
		{
			lcsp.m_featureCurves.push_back(featureCurve);

			for (auto& brick_entity : lcsp.m_bricks)
			{
				uint brick_idx = GLandscapeComponents::brickManager().getIndex(brick_entity);

				// check if new feature curve lies within brick
				Vec3 brick_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(brick_entity));
				Vec3 brick_dimension = GLandscapeComponents::brickManager().getDimension(brick_idx);
				Vec3 brick_lower_corner = brick_position - brick_dimension / 2.0f;
				Vec3 brick_upper_corner = brick_position + brick_dimension / 2.0f;

				Vec3 curve_lower_corner = GLandscapeComponents::featureCurveManager().getLowerCorner(curve_idx);
				Vec3 curve_upper_corner = GLandscapeComponents::featureCurveManager().getUpperCorner(curve_idx);

				if ((curve_upper_corner.x > brick_lower_corner.x)
					&& (curve_upper_corner.y > brick_lower_corner.y)
					&& (curve_upper_corner.z > brick_lower_corner.z)
					&& (curve_lower_corner.x < brick_upper_corner.x)
					&& (curve_lower_corner.y < brick_upper_corner.y)
					&& (curve_lower_corner.z < brick_upper_corner.z))
				{
					GLandscapeComponents::brickManager().addFeatureCurve(brick_idx, featureCurve);
				}
			}

		}

		return featureCurve;
	}

	Entity StaticLandscapeComponentManager::addFeatureMesh(const std::string& mesh_path, Vec3 position, Quat orientation)
	{
		Entity new_entity = GEngineCore::entityManager().create();

		GCoreComponents::transformManager().addComponent(new_entity, position, orientation);

		GLandscapeComponents::featureMeshManager().addComponent(new_entity, mesh_path);
		uint mesh_idx = GLandscapeComponents::featureMeshManager().getIndex(new_entity);

		for (auto& lcsp : m_landscapes)
		{
			lcsp.m_featureMeshes.push_back(new_entity);

			for (auto& brick_entity : lcsp.m_bricks)
			{
				uint brick_idx = GLandscapeComponents::brickManager().getIndex(brick_entity);

				// check if new feature curve lies within brick
				Vec3 brick_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(brick_entity));
				Vec3 brick_dimension = GLandscapeComponents::brickManager().getDimension(brick_idx);
				Vec3 brick_lower_corner = brick_position - brick_dimension / 2.0f;
				Vec3 brick_upper_corner = brick_position + brick_dimension / 2.0f;

				Vec3 mesh_lower_corner = GLandscapeComponents::featureMeshManager().getLowerCorner(mesh_idx);
				Vec3 mesh_upper_corner = GLandscapeComponents::featureMeshManager().getUpperCorner(mesh_idx);

				if ((mesh_upper_corner.x > brick_lower_corner.x)
					&& (mesh_upper_corner.y > brick_lower_corner.y)
					&& (mesh_upper_corner.z > brick_lower_corner.z)
					&& (mesh_lower_corner.x < brick_upper_corner.x)
					&& (mesh_lower_corner.y < brick_upper_corner.y)
					&& (mesh_lower_corner.z < brick_upper_corner.z))
				{
					GLandscapeComponents::brickManager().addFeatureMesh(brick_idx, new_entity);
				}
			}

		}

		return new_entity;
	}

	Entity StaticLandscapeComponentManager::importHeightmap(const std::string& heightmap_path, Vec3 position, Quat orientation)
	{
		// check file ending for early exit
		if (heightmap_path.substr(heightmap_path.length() - 3) != "ppm")
		{
			std::cout << "Heightmap format not supported. Please supply a ppm file." << std::endl;
			return GEngineCore::entityManager().create(); //return dummy entity...think of something more clever
		}

		// read ppm image
		std::vector<uint8_t> image_data;
		TextureLayout image_layout;
		ResourceLoading::loadPpmImage(heightmap_path, image_data, image_layout);

		// generate mesh based on heightmap
		std::vector<float> vertex_data;
		std::vector<uint32_t> index_data;

		float min_y = std::numeric_limits<float>::max();
		float max_y = std::numeric_limits<float>::min();

		for (int z = 0; z < image_layout.height; ++z)
		{
			for (int x = 0; x < image_layout.width; ++x)
			{
				int i = (x + (image_layout.height - (z+1)) * image_layout.width)*3;

				vertex_data.push_back(static_cast<float>(x));
				vertex_data.push_back(static_cast<float>(image_data[i]));
				vertex_data.push_back(static_cast<float>(z));
				

				min_y = std::min(min_y, static_cast<float>(image_data[i]));
				max_y = std::max(max_y, static_cast<float>(image_data[i]));

				//compute normals
				//	int bx = std::max(0, x - 1) + z*image_layout.height;
				//	int fx = std::min(image_layout.width - 1, x + 1) + z*image_layout.height;
				//	int bz = x + std::max(0, z - 1)*image_layout.height;
				//	int fz = x + std::min(image_layout.width - 1, z + 1)*image_layout.height;
				//	
				//	float dx = static_cast<float>(image_data[fx] - image_data[bx]) / 2.0f;
				//	float dz = static_cast<float>(image_data[fz] - image_data[bz]) / 2.0f;
				//	
				//	Vec3 normal = glm::normalize(glm::cross(Vec3(1.0f, dx, 0.0), Vec3(0.0f, dz, 1.0f)));
				//	
				//	vertex_data.push_back(normal.x);
				//	vertex_data.push_back(normal.z);
				//	vertex_data.push_back(normal.z);
			}
		}

		// transform height value s.t. min_z=0 and scale by 0.65
		for (int i = 1; i < vertex_data.size(); i = i + 3)
		{
			vertex_data[i] += -min_y;
			vertex_data[i] *= 0.65f;
		}

		max_y = (max_y - min_y)*0.65f;
		min_y = 0.0;

		for (int z = 0; z < image_layout.height - 1; ++z)
		{
			for (int x = 0; x < image_layout.width - 1; ++x)
			{
				int i = x + z*image_layout.height;

				index_data.push_back(i);
				index_data.push_back(i + image_layout.width);
				index_data.push_back(i + 1);
				index_data.push_back(i + image_layout.width);
				index_data.push_back(i + 1 + image_layout.width);
				index_data.push_back(i + 1);
			}
		}

		// Create new entity and add Transform- and FeatureMesh- Components
		Entity fm_entity = GEngineCore::entityManager().create();
		GCoreComponents::transformManager().addComponent(fm_entity, position, orientation);
		//VertexLayout vertex_layout = VertexLayout(6 * sizeof(float), { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0), VertexLayout::Attribute(GL_FLOAT, 3, GL_FALSE, 3 * sizeof(float)) });
		VertexLayout vertex_layout = VertexLayout(3 * sizeof(float), { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0)});
		GLandscapeComponents::featureMeshManager().addComponent(fm_entity, heightmap_path, Vec3(0.0,0.0,0.0), Vec3(static_cast<float>(image_layout.width),max_y,static_cast<float>(image_layout.height)), vertex_data, index_data, vertex_layout, GL_TRIANGLES);
		uint mesh_idx = GLandscapeComponents::featureMeshManager().getIndex(fm_entity);

		// update landscapes
		for (auto& lcsp : m_landscapes)
		{
			lcsp.m_heightmaps.push_back(fm_entity);

			for (auto& brick_entity : lcsp.m_bricks)
			{
				uint brick_idx = GLandscapeComponents::brickManager().getIndex(brick_entity);

				// check if new feature curve lies within brick
				Vec3 brick_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(brick_entity));
				Vec3 brick_dimension = GLandscapeComponents::brickManager().getDimension(brick_idx);
				Vec3 brick_lower_corner = brick_position - brick_dimension / 2.0f;
				Vec3 brick_upper_corner = brick_position + brick_dimension / 2.0f;

				Vec3 mesh_lower_corner = GLandscapeComponents::featureMeshManager().getLowerCorner(mesh_idx);
				Vec3 mesh_upper_corner = GLandscapeComponents::featureMeshManager().getUpperCorner(mesh_idx);

				if ((mesh_upper_corner.x > brick_lower_corner.x)
					&& (mesh_upper_corner.y > brick_lower_corner.y)
					&& (mesh_upper_corner.z > brick_lower_corner.z)
					&& (mesh_lower_corner.x < brick_upper_corner.x)
					&& (mesh_lower_corner.y < brick_upper_corner.y)
					&& (mesh_lower_corner.z < brick_upper_corner.z))
				{
					GLandscapeComponents::brickManager().addHeightmapMesh(brick_idx, fm_entity);
				}
			}

		}

		return fm_entity;
	}

	void StaticLandscapeComponentManager::updateBrickFeatureCurves(uint index)
	{
		std::cout << "'allo" << std::endl;

		// m*n complexity...it's not very effective
		for (auto& brick_entity : m_landscapes[index].m_bricks)
		{
			uint brick_idx = GLandscapeComponents::brickManager().getIndex(brick_entity);

			GLandscapeComponents::brickManager().clearFeatureCurves(brick_idx);

			Vec3 brick_position = GCoreComponents::transformManager().getWorldPosition(brick_entity);
			Vec3 brick_dimension = GLandscapeComponents::brickManager().getDimension(brick_idx);
			Vec3 brick_lower_corner = brick_position - brick_dimension / 2.0f;
			Vec3 brick_upper_corner = brick_position + brick_dimension / 2.0f;

			for (auto& featureCurve_entity : m_landscapes[index].m_featureCurves)
			{	
				uint curve_idx = GLandscapeComponents::featureCurveManager().getIndex(featureCurve_entity);

				Vec3 curve_lower_corner = GLandscapeComponents::featureCurveManager().getLowerCorner(curve_idx);
				Vec3 curve_upper_corner = GLandscapeComponents::featureCurveManager().getUpperCorner(curve_idx);

				if ((curve_upper_corner.x > brick_lower_corner.x)
					&& (curve_upper_corner.y > brick_lower_corner.y)
					&& (curve_upper_corner.z > brick_lower_corner.z)
					&& (curve_lower_corner.x < brick_upper_corner.x)
					&& (curve_lower_corner.y < brick_upper_corner.y)
					&& (curve_lower_corner.z < brick_upper_corner.z))
				{
					GLandscapeComponents::brickManager().addFeatureCurve(brick_idx, featureCurve_entity);

					std::cout << "Brick " << brick_entity.id() << " has feature curve " << featureCurve_entity.id() << std::endl;
				}
			}
		}
	}

	void StaticLandscapeComponentManager::updateBricks(uint index)
	{
		//	for (auto& brick : m_landscapes[index].m_bricks)
		//	{
		//		uint brick_idx = GLandscapeComponents::brickManager().getIndex(brick);
		//		GLandscapeComponents::brickManager().updateBrick(brick_idx);
		//	}

		if (m_landscapes[index].m_ready == false)
			return;

		m_landscapes[index].m_ready = false;

		// Update per bricks feature curve list

		for (auto& brick_entity : m_landscapes[index].m_bricks)
		{
			uint brick_idx = GLandscapeComponents::brickManager().getIndex(brick_entity);

			GLandscapeComponents::brickManager().clearFeatureCurves(brick_idx);

			// check if new feature curve lies within brick
			Vec3 brick_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(brick_entity));
			Vec3 brick_dimension = GLandscapeComponents::brickManager().getDimension(brick_idx);
			Vec3 brick_lower_corner = brick_position - brick_dimension / 2.0f;
			Vec3 brick_upper_corner = brick_position + brick_dimension / 2.0f;

			for (auto& featureCurve_entity : m_landscapes[index].m_featureCurves)
			{
				uint curve_idx = GLandscapeComponents::featureCurveManager().getIndex(featureCurve_entity);

				Vec3 curve_lower_corner = GLandscapeComponents::featureCurveManager().getLowerCorner(curve_idx);
				Vec3 curve_upper_corner = GLandscapeComponents::featureCurveManager().getUpperCorner(curve_idx);

				if ((curve_upper_corner.x > brick_lower_corner.x)
					&& (curve_upper_corner.y > brick_lower_corner.y)
					&& (curve_upper_corner.z > brick_lower_corner.z)
					&& (curve_lower_corner.x < brick_upper_corner.x)
					&& (curve_lower_corner.y < brick_upper_corner.y)
					&& (curve_lower_corner.z < brick_upper_corner.z))
				{
					GLandscapeComponents::brickManager().addFeatureCurve(brick_idx, featureCurve_entity);
				}
			}
		}
		
		if (m_landscapes[index].m_bricks.size() == 1)
			GLandscapeComponents::brickManager().updateBrick(GLandscapeComponents::brickManager().getIndex(m_landscapes[index].m_bricks.back()));
		else
			GLandscapeComponents::brickManager().updateBricks(m_landscapes[index].m_bricks);

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { setReady(index); });
	}
	
	void StaticLandscapeComponentManager::updateAll()
	{
		for (uint i = 0; i < m_landscapes.size(); i++)
		{
			updateBricks(i);
		}
	}

	void StaticLandscapeComponentManager::updateCorrespondingLandscape(Entity modified_feature_curve)
	{
		uint lscp_idx = 0;
		for (auto& lscp : m_landscapes)
		{
			for (auto& feature_curve : lscp.m_featureCurves)
			{
				if (feature_curve == modified_feature_curve)
				{
					updateBrickFeatureCurves(lscp_idx);
					updateBricks(lscp_idx);
				}
			}

			for (auto& feature_mesh : lscp.m_featureMeshes)
			{
				if (feature_mesh == modified_feature_curve)
				{
					updateBricks(lscp_idx);
				}
			}

			++lscp_idx;
		}
	}

	bool StaticLandscapeComponentManager::isReady(uint index)
	{
		return m_landscapes[index].m_ready;
	}

	void StaticLandscapeComponentManager::setReady(uint index)
	{
		m_landscapes[index].m_ready = true;
	}

	std::vector<Entity>& StaticLandscapeComponentManager::getFeatureCurveList(uint index)
	{
		return m_landscapes[index].m_featureCurves;
	}

	std::vector<Entity>& StaticLandscapeComponentManager::getBrickList(uint index)
	{
		return m_landscapes[index].m_bricks;
	}

	Vec3 StaticLandscapeComponentManager::getDimension(Entity e)
	{
		uint lcsp_idx = getIndex(e);

		return m_landscapes[lcsp_idx].m_dimensions;
	}

	std::vector<Entity> StaticLandscapeComponentManager::getListofLandscapeEntities()
	{
		std::vector<Entity> rtn;

		for (auto& lcsp : m_landscapes)
		{
			rtn.push_back(lcsp.m_entity);
		}

		return rtn;
	}

	std::vector<Entity> StaticLandscapeComponentManager::getListofBrickEntities()
	{
		std::vector<Entity> rtn;

		for (auto& lcsp : m_landscapes)
		{
			for (auto entities : lcsp.m_bricks)
			{
				rtn.push_back(entities);
			}
		}

		return rtn;
	}

	std::vector<Entity> StaticLandscapeComponentManager::getListofFeatureCurveEntities()
	{
		std::vector<Entity> rtn;

		for (auto& lcsp : m_landscapes)
		{
			for (auto entities : lcsp.m_featureCurves)
			{
				rtn.push_back(entities);
			}
		}

		return rtn;
	}

	std::vector<Entity> StaticLandscapeComponentManager::getListofFeatureMeshEntities()
	{
		std::vector<Entity> rtn;

		for (auto& lcsp : m_landscapes)
		{
			for (auto entities : lcsp.m_featureMeshes)
			{
				rtn.push_back(entities);
			}
		}

		return rtn;
	}

	std::vector<Entity> StaticLandscapeComponentManager::getListofHeightmapEntities()
	{
		std::vector<Entity> rtn;

		for (auto& lcsp : m_landscapes)
		{
			for (auto entities : lcsp.m_heightmaps)
			{
				rtn.push_back(entities);
			}
		}

		return rtn;
	}

	void StaticLandscapeComponentManager::exportLandscapeMesh(Entity landscape_entity, std::string filepath)
	{
		uint lcsp_idx = getIndex(landscape_entity);

		for (auto& brick : m_landscapes[lcsp_idx].m_bricks)
		{
			uint brick_idx = GLandscapeComponents::brickManager().getIndex(brick);

			GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, filepath] { GLandscapeComponents::brickManager().exportMesh(brick_idx, filepath); });
		}
	}

	/*
	void StaticLandscapeComponentManager::drawDebugInterface()
	{
		if (!GTools::editorUI().showLandscapeProperties())
			return;

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_MenuBar;
		bool p_open = true;
		if (!ImGui::Begin("Landscape Component Info", &p_open, window_flags))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		//TODO landscape selection
		std::vector<std::string> landscape_list(m_landscapes.size());
		std::vector<const char*> lndscp_char_list;

		for (uint i = 0; i < m_landscapes.size(); i++)
		{
			landscape_list[i] = "Landscape_"+std::to_string(i);
			lndscp_char_list.push_back(landscape_list[i].c_str());
		}

		static int lnscp_item = 0;
		ImGui::Combo("Landscape", &lnscp_item, lndscp_char_list.data(), m_landscapes.size());

		//TODO brick selection
		int brick_cnt = 0;
		std::vector<const char*> brick_char_list;
		if (m_landscapes.size() > 0)
		{
			brick_cnt = m_landscapes[lnscp_item].m_bricks.size();
			std::vector<std::string> brick_list(brick_cnt);
			for (uint i = 0; i < m_landscapes[lnscp_item].m_bricks.size(); i++)
			{
				brick_list[i] = "Brick_" + std::to_string(i);
				brick_char_list.push_back(brick_list[i].c_str());
			}
		}
		static int brick_item = 0;
		ImGui::Combo("Brick", &brick_item, brick_char_list.data(), brick_cnt);

		ImGui::Separator();

		if (m_landscapes.size() > 0)
		{
			//TODO show information
			const char* debug_field_items[] = { "none", "normal", "gradient", "noise", "surface", "surface_boundary" };
			static int debug_field_item = 0;
			ImGui::Combo("Debug field data", &debug_field_item, debug_field_items, 6);

			uint brick_idx = GLandscapeComponents::brickManager().getIndex(m_landscapes[lnscp_item].m_bricks[brick_item]);

			switch (debug_field_item)
			{
			case 0:
				GRenderingComponents::volumeManager().setVisibility(m_landscapes[lnscp_item].m_bricks[brick_item], false);
				break;
			case 1:
				GRenderingComponents::volumeManager().setVisibility(m_landscapes[lnscp_item].m_bricks[brick_item], true);
				GLandscapeComponents::brickManager().setDebugField(brick_idx, LandscapeBrickComponentManager::Datafield::NORMAL);
				break;
			case 2:
				GRenderingComponents::volumeManager().setVisibility(m_landscapes[lnscp_item].m_bricks[brick_item], true);
				GLandscapeComponents::brickManager().setDebugField(brick_idx, LandscapeBrickComponentManager::Datafield::GRADIENT);
				break;
			case 3:
				GRenderingComponents::volumeManager().setVisibility(m_landscapes[lnscp_item].m_bricks[brick_item], true);
				GLandscapeComponents::brickManager().setDebugField(brick_idx, LandscapeBrickComponentManager::Datafield::NOISE);
				break;
			case 4:
				GRenderingComponents::volumeManager().setVisibility(m_landscapes[lnscp_item].m_bricks[brick_item], true);
				GLandscapeComponents::brickManager().setDebugField(brick_idx, LandscapeBrickComponentManager::Datafield::SURFACE);
				break;
			case 5:
				GRenderingComponents::volumeManager().setVisibility(m_landscapes[lnscp_item].m_bricks[brick_item], true);
				GLandscapeComponents::brickManager().setDebugField(brick_idx, LandscapeBrickComponentManager::Datafield::SURFACE_BOUNDARY);
				break;
			default:
				break;
			}


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
					mat = GEngineCore::resourceManager().createMaterial("../resources/materials/landscape_coastline.slmtl", surface_material_info.shader_filepaths, surface_material_info.texture_filepaths).get();
					GLandscapeComponents::brickManager().setSurfaceMaterial(brick_idx, mat);
					break;
				case 1:
					surface_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_rocky_desert.slmtl");
					mat = GEngineCore::resourceManager().createMaterial("../resources/materials/landscape_rocky_desert.slmtl", surface_material_info.shader_filepaths, surface_material_info.texture_filepaths).get();
					GLandscapeComponents::brickManager().setSurfaceMaterial(brick_idx, mat);
					break;
				case 2:
					surface_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_snow_mountain.slmtl");
					mat = GEngineCore::resourceManager().createMaterial("../resources/materials/landscape_snow_mountain.slmtl", surface_material_info.shader_filepaths, surface_material_info.texture_filepaths).get();
					GLandscapeComponents::brickManager().setSurfaceMaterial(brick_idx, mat);
					break;
				default:
					break;
				}
			}
		}

		ImGui::End();

	}
	*/
}