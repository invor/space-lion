#include "FeatureCurveComponent.hpp"

#include <algorithm>

#include "utility.hpp"

#include "GlobalCoreComponents.hpp"
#include "BSplineComponent.hpp"
#include "TransformComponent.hpp"

namespace Landscape
{

	//	std::pair<Vec3, Vec3> FeatureCurveComponentManager::interpolateCurveBitangents(uint curve_idx, float u)
	//	{
	//		u = std::max(0.0f, std::min(1.0f, u));
	//	
	//		// find CPs around curve point
	//		uint cp_idx = 0;
	//		while (m_data[curve_idx].m_constraint_points[cp_idx].m_curve_position < u)
	//		{
	//			cp_idx;
	//		}
	//	
	//		Vec3 b_rotation_axis = Vec3(1.0f);
	//		float b_rotation_angle = 1.0f;
	//		Vec3 f_rotation_axis = Vec3(1.0f);
	//		float f_rotation_angle = 1.0f;
	//	
	//		// compute length of section between CPs (in curve parameter space)
	//		float curve_section_length = m_data[curve_idx].m_constraint_points[cp_idx].m_curve_position
	//			- m_data[curve_idx].m_constraint_points[cp_idx - 1].m_curve_position;
	//	
	//		// compute interpolation factor
	//		float alpha = (u - m_data[curve_idx].m_constraint_points[cp_idx - 1].m_curve_position) / curve_section_length;
	//	
	//		// Transform gradients from surrounding cps to curve position, than interpolate
	//		uint bspline_idx = GCoreComponents::bSplineManager().getIndex(m_data[curve_idx].m_entity).second;
	//		Vec3 tangent_b = GCoreComponents::bSplineManager().computeCurveTangent(bspline_idx, m_data[curve_idx].m_constraint_points[cp_idx - 1].m_curve_position);
	//		Vec3 tangent_f = GCoreComponents::bSplineManager().computeCurveTangent(bspline_idx, m_data[curve_idx].m_constraint_points[cp_idx].m_curve_position);
	//		Vec3 tangent_c = GCoreComponents::bSplineManager().computeCurveTangent(bspline_idx, u);
	//	
	//		float angle_b = dot(tangent_b, tangent_c);
	//		Vec3 axis_b = glm::cross(tangent_b, tangent_c);
	//	
	//		float angle_f = dot(tangent_f, tangent_c);
	//		Vec3 axis_f = glm::cross(tangent_f, tangent_c);
	//	
	//		Vec3 bt_lh_0 = m_data[curve_idx].m_constraint_points[cp_idx - 1].m_lefthand_bitangent;
	//		Vec3 bt_rh_0 = m_data[curve_idx].m_constraint_points[cp_idx - 1].m_righthand_bitangent;
	//		Vec3 bt_lh_1 = m_data[curve_idx].m_constraint_points[cp_idx].m_lefthand_bitangent;
	//		Vec3 bt_rh_1 = m_data[curve_idx].m_constraint_points[cp_idx].m_righthand_bitangent;
	//	
	//		if (abs(angle_b) < 0.99)
	//		{
	//			b_rotation_axis = axis_b;
	//			b_rotation_angle = angle_b;
	//		}
	//	
	//		Quat rotation_b = glm::angleAxis(acos(b_rotation_angle), glm::normalize(b_rotation_axis));
	//	
	//		bt_lh_0 = glm::normalize(glm::mat3(glm::toMat4(rotation_b)) * bt_lh_0);
	//		bt_rh_0 = glm::normalize(glm::mat3(glm::toMat4(rotation_b)) * bt_rh_0);
	//	
	//		if (abs(angle_f) < 0.99)
	//		{
	//			f_rotation_axis = axis_f;
	//			f_rotation_angle = angle_f;
	//		}
	//	
	//		Quat rotation_f = glm::angleAxis(acos(f_rotation_angle), glm::normalize(f_rotation_axis));
	//	
	//		bt_rh_1 = glm::normalize(glm::mat3(glm::toMat4(rotation_f)) * bt_rh_1);
	//		bt_lh_1 = glm::normalize(glm::mat3(glm::toMat4(rotation_f)) * bt_lh_1);
	//	
	//		Vec3 bitangen_lh = alpha * bt_lh_1 + (1.0f - alpha) * bt_lh_0;
	//		Vec3 bitangen_rh = alpha * bt_rh_1 + (1.0f - alpha) * bt_rh_0;
	//	
	//		// project gradient into plane given by tangent
	//		bitangen_lh = (bitangen_lh - (glm::dot(tangent_c, bitangen_lh) * tangent_c));
	//		bitangen_rh = (bitangen_rh - (glm::dot(tangent_c, bitangen_rh) * tangent_c));
	//	
	//		return std::pair<Vec3, Vec3>(bitangen_lh, bitangen_rh);
	//	}
	//	
	//	std::tuple<float, float, int, int> FeatureCurveComponentManager::interpolateCurveSurfaceProperties(uint curve_idx, float u)
	//	{
	//		u = std::max(0.0f, std::min(1.0f, u));
	//	
	//		// find CPs around curve point
	//		uint cp_idx = 0;
	//		while (m_data[curve_idx].m_constraint_points[cp_idx].m_curve_position < u)
	//		{
	//			cp_idx;
	//		}
	//	
	//		// compute length of section between CPs (in curve parameter space)
	//		float curve_section_length = m_data[curve_idx].m_constraint_points[cp_idx].m_curve_position
	//			- m_data[curve_idx].m_constraint_points[cp_idx - 1].m_curve_position;
	//	
	//		// compute interpolation factor
	//		float alpha = (u - m_data[curve_idx].m_constraint_points[cp_idx - 1].m_curve_position) / curve_section_length;
	//	
	//		float noise_amplitude = (1.0f - alpha) * m_data[curve_idx].m_constraint_points[cp_idx - 1].m_noise_amplitude + alpha * m_data[curve_idx].m_constraint_points[cp_idx + 1].m_noise_amplitude;
	//		float noise_roughness = (1.0f - alpha) * m_data[curve_idx].m_constraint_points[cp_idx - 1].m_noise_roughness + alpha * m_data[curve_idx].m_constraint_points[cp_idx + 1].m_noise_roughness;
	//		
	//		//just take over material from lower cp
	//		int lh_material = m_data[curve_idx].m_constraint_points[cp_idx - 1].m_lefthand_material;
	//		int rh_material = m_data[curve_idx].m_constraint_points[cp_idx - 1].m_righthand_material;
	//	
	//		return { noise_amplitude,noise_roughness,lh_material,rh_material };
	//	}
	//	
	//	std::tuple<VertexData, IndexData, VertexLayout> FeatureCurveComponentManager::computeCurveProxyMesh(uint curve_idx)
	//	{
	//		std::vector<uint8_t> vertex_data;
	//		std::vector<uint32_t> index_data;
	//		VertexLayout vertex_layout;
	//		
	//		// TODO compute geometry
	//	
	//		return { vertex_data,index_data,vertex_layout };
	//	}
	//	
	//	std::tuple<VertexData, IndexData, VertexLayout> FeatureCurveComponentManager::computeBitangentProxyMesh(Entity bitangent)
	//	{
	//		std::tuple<std::vector<float>, std::vector<uint32_t>, VertexLayout> rtn;
	//	
	//		auto cp_query = utility::entityToIndex(bitangent.id(), m_bitangent_eID_to_cp_eID);
	//		if (cp_query.first)
	//		{
	//			auto curve_query = utility::entityToIndex(cp_query.second, m_cp_eID_to_curve_idx);
	//	
	//			if (curve_query.first)
	//			{
	//				for (auto& cp : m_data[curve_query.second].m_constraint_points)
	//				{
	//					if (cp.m_entity.id() == cp_query.second)
	//					{
	//						Vec3 bitangent_vector = cp.m_lh_bitangent_entity == bitangent ? cp.m_lefthand_bitangent : cp.m_righthand_bitangent;
	//	
	//						Vec3 v0 = Vec3(0.0);
	//						Vec3 v1 = 1.5f * m_data[curve_query.second].m_ribbon_width * bitangent_vector;
	//						std::vector<float> bitangent_interface_vertices({
	//							v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
	//							v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0});
	//						
	//						std::vector<float> bitangent_select_vertices({ v0.x,v0.y,v0.z, v1.x,v1.y,v1.z});
	//						
	//						//TODO insert selection mesh
	//						std::vector<uint32_t> bitangent_indices({ 0,1 });
	//						
	//						VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
	//							VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });
	//	
	//						break;
	//					}
	//				}
	//			}
	//		}
	//	
	//		Vec3 v0 = Vec3(0.0);
	//		//			Vec3 v1 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * constraintPoint.m_gradient_0;
	//		//			Vec3 v2 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * constraintPoint.m_gradient_1;
	//		//			std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
	//		//				v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0,
	//		//				v2.x,v2.y,v2.z,1.0,1.0,0.0,1.0 });
	//		//
	//		//			//std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,0.47f,0.125f,0.125f,1.0f,
	//		//			//	v1.x,v1.y,v1.z,0.47f,0.125f,0.125f,1.0f,
	//		//			//	v2.x,v2.y,v2.z,0.47f,0.125f,0.125f,1.0f });
	//		//
	//		//			std::vector<float> gradient_select_vertices({ v0.x,v0.y,v0.z, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z });
	//		//
	//		//			//TODO insert selection mesh
	//		//			std::vector<uint> lh_gradient_indices({ 0,1 });
	//		//
	//		//			VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
	//		//				VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });
	//		//
	//		//			GRenderingComponents::interfaceMeshManager().updateComponent(constraintPoint.m_lefthand_gradient,
	//		//				gradient_interface_vertices,
	//		//				lh_gradient_indices,
	//		//				vertex_description,
	//		//				GL_LINES);
	//		//
	//		//			VertexLayout picking_vertex_description(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });
	//		//
	//		//			GRenderingComponents::pickingManager().updateComponentProxyGeometry(constraintPoint.m_lefthand_gradient,
	//		//				gradient_select_vertices,
	//		//				lh_gradient_indices,
	//		//				picking_vertex_description,
	//		//				GL_LINES);
	//		//
	//		//
	//		//			std::vector<uint> rh_gradient_indices({ 0,2 });
	//		//
	//		//			GRenderingComponents::interfaceMeshManager().updateComponent(constraintPoint.m_righthand_gradient,
	//		//				gradient_interface_vertices,
	//		//				rh_gradient_indices,
	//		//				vertex_description,
	//		//				GL_LINES);
	//		//
	//		//			GRenderingComponents::pickingManager().updateComponentProxyGeometry(constraintPoint.m_righthand_gradient,
	//		//				gradient_select_vertices,
	//		//				rh_gradient_indices,
	//		//				picking_vertex_description,
	//		//				GL_LINES);
	//	}
	//	
	//	void FeatureCurveComponentManager::addComponent(Entity entity, bool is_surface_seed)
	//	{
	//		std::unique_lock<std::mutex>(m_dataAccess_mutex);
	//	
	//		//Create proxy mesh for voxelization
	//		ResourceID proxy_mesh = GEngineCore::resourceManager().createMeshAsync("eID" + entity.id(), {}, {}, VertexLayout(), GL_TRIANGLES);
	//	
	//		m_data.push_back(Data(entity,is_surface_seed, proxy_mesh));
	//		//GCoreComponents::transformManager().addComponent(entity, position, orientation);
	//		uint idx = static_cast<uint>(m_data.size() - 1);
	//		m_curve_eID_to_idx.insert(std::pair<uint, uint>(entity.id(), idx));
	//	
	//		// Add inital constraint points at the beginning and end of the curve
	//		addConstraintPoint(idx, 0.0f, Vec3(0.0, 0.0, -1.0), Vec3(0.0, -1.0, 0.0));
	//		addConstraintPoint(idx, 1.0f, Vec3(0.0, 0.0, -1.0), Vec3(0.0, -1.0, 0.0));
	//	
	//		// Build intial mesh data
	//		auto mesh_data = computeCurveProxyMesh(idx);
	//		//GEngineCore::resourceManager().updateMeshAsync(proxy_mesh,mesh_data.first,mesh_data.second);
	//	
	//		//		#if EDITOR_MODE // preprocessor definition
	//		//		
	//		//				VertexLayout vertex_description(60, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
	//		//					VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 3),
	//		//					VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 6),
	//		//					VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 9),
	//		//					VertexLayout::Attribute(GL_FLOAT,2,GL_FALSE,sizeof(GLfloat) * 12),
	//		//					VertexLayout::Attribute(GL_FLOAT,1,GL_FALSE,sizeof(GLfloat) * 14) });
	//		//		
	//		//				GRenderingComponents::interfaceMeshManager().addComponent<std::vector<float>, std::vector<uint>>(entity,
	//		//					"feature_curve" + std::to_string(idx),
	//		//					"../resources/materials/editor/interface_curve.slmtl",
	//		//					m_featureCurves.back().m_mesh_vertices,
	//		//					m_featureCurves.back().m_mesh_indices,
	//		//					vertex_description,
	//		//					GL_TRIANGLES);
	//		//		
	//		//				// add selectable component
	//		//				GRenderingComponents::pickingManager().addComponent<std::vector<float>, std::vector<uint>>(entity,
	//		//					"feature_curve_" + std::to_string(idx) + "_selectProxy",
	//		//					"../resources/materials/editor/picking.slmtl",
	//		//					m_featureCurves.back().m_mesh_vertices,
	//		//					m_featureCurves.back().m_mesh_indices,
	//		//					vertex_description,
	//		//					GL_TRIANGLES);
	//		//				GTools::selectManager().addComponent(entity, std::bind(&Editor::LandscapeTools::activateFeatureCurveTools, &GTools::landscapeTool(), entity),
	//		//					std::bind(&Editor::LandscapeTools::deactivateFeatureCurveTools, &GTools::landscapeTool()));
	//		//		#endif
	//	}
	//	
	//	void FeatureCurveComponentManager::deleteComponent(uint idx)
	//	{
	//	
	//	}
	//	
	//	std::pair<bool, uint> FeatureCurveComponentManager::getIndex(Entity entity) const
	//	{
	//		return utility::entityToIndex(entity.id(), m_curve_eID_to_idx);
	//	}
	//	
	//	inline void FeatureCurveComponentManager::setRibbonWidth(uint curve_idx, float ribbon_width)
	//	{
	//		m_data[curve_idx].m_ribbon_width = ribbon_width;
	//	}
	//	
	//	void FeatureCurveComponentManager::setRibbonWidth(float ribbon_width)
	//	{
	//		for (auto& curve_data : m_data)
	//			curve_data.m_ribbon_width = ribbon_width;
	//	}
	//	
	//	void FeatureCurveComponentManager::updateCurve(uint curve_idx)
	//	{
	//		// recompute all constraint point positions
	//		for (auto& constraint_point : m_data[curve_idx].m_constraint_points)
	//		{
	//			Vec3 old_tangent = constraint_point.m_tangent;
	//	
	//			Vec3 new_position = GCoreComponents::bSplineManager().computeCurvePoint(m_data[curve_idx].m_entity, constraint_point.m_curve_position);
	//			GCoreComponents::transformManager().setPosition(constraint_point.m_entity, new_position);
	//			//TODO connect bitangent entites to parent cp in transform manager?
	//			GCoreComponents::transformManager().setPosition(constraint_point.m_lh_bitangent_entity, new_position);
	//			GCoreComponents::transformManager().setPosition(constraint_point.m_rh_bitangent_entity, new_position);
	//	
	//			// compute curve tangent around new cp position
	//			Vec3 tangent = GCoreComponents::bSplineManager().computeCurveTangent(m_data[curve_idx].m_entity, constraint_point.m_curve_position);
	//			constraint_point.m_tangent = tangent;
	//	
	//			float angle = dot(old_tangent, tangent);
	//			Vec3 axis = glm::cross(old_tangent, tangent);
	//	
	//			if (angle < 0.99f)
	//			{
	//				Quat rotation = glm::angleAxis(acos(angle), glm::normalize(axis));
	//	
	//				constraint_point.m_lefthand_bitangent = glm::normalize(Vec3(glm::toMat4(rotation) * Vec4(constraint_point.m_lefthand_bitangent, 1.0)));
	//				constraint_point.m_righthand_bitangent = glm::normalize(Vec3(glm::toMat4(rotation) * Vec4(constraint_point.m_righthand_bitangent, 1.0)));
	//			}
	//	
	//			// better safe than sorry, so project gradient into plane given by tangent
	//			constraint_point.m_lefthand_bitangent = (constraint_point.m_lefthand_bitangent - (glm::dot(tangent, constraint_point.m_lefthand_bitangent) * tangent));
	//			constraint_point.m_righthand_bitangent = (constraint_point.m_righthand_bitangent - (glm::dot(tangent, constraint_point.m_righthand_bitangent) * tangent));
	//	
	//			/*
	//			//TODO fix gradient project or -more likely- switch to a different approach
	//	
	//			Vec3 ref = Vec3(0.0, 1.0, 0.0);
	//			if (glm::dot(tangent, ref) > 0.7)
	//			ref = Vec3(0.0, 0.0, 1.0);
	//	
	//			ref = glm::cross(tangent, ref);
	//			ref = glm::cross(tangent, ref);
	//	
	//			// project gradient vectors onto plane defined by the tangent (screws up because gradient can switch sides if curve curves too much)
	//			constraintPoint.m_gradient_0 = constraintPoint.m_gradient_0 - ( glm::dot(tangent,constraintPoint.m_gradient_0) * tangent);
	//			constraintPoint.m_gradient_1 = constraintPoint.m_gradient_1 - ( glm::dot(tangent,constraintPoint.m_gradient_1) * tangent);
	//	
	//			constraintPoint.m_gradient_0 = glm::normalize(constraintPoint.m_gradient_0);
	//			constraintPoint.m_gradient_1 = glm::normalize(constraintPoint.m_gradient_1);
	//			*/
	//	
	//			// TODO move to function dedicated to (re)building constraint point interface mesh
	//			//#if EDITOR_MODE // preprocessor definition
	//			//
	//			//			Vec3 v0 = Vec3(0.0);
	//			//			Vec3 v1 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * constraintPoint.m_gradient_0;
	//			//			Vec3 v2 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * constraintPoint.m_gradient_1;
	//			//			std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
	//			//				v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0,
	//			//				v2.x,v2.y,v2.z,1.0,1.0,0.0,1.0 });
	//			//
	//			//			//std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,0.47f,0.125f,0.125f,1.0f,
	//			//			//	v1.x,v1.y,v1.z,0.47f,0.125f,0.125f,1.0f,
	//			//			//	v2.x,v2.y,v2.z,0.47f,0.125f,0.125f,1.0f });
	//			//
	//			//			std::vector<float> gradient_select_vertices({ v0.x,v0.y,v0.z, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z });
	//			//
	//			//			//TODO insert selection mesh
	//			//			std::vector<uint> lh_gradient_indices({ 0,1 });
	//			//
	//			//			VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
	//			//				VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });
	//			//
	//			//			GRenderingComponents::interfaceMeshManager().updateComponent(constraintPoint.m_lefthand_gradient,
	//			//				gradient_interface_vertices,
	//			//				lh_gradient_indices,
	//			//				vertex_description,
	//			//				GL_LINES);
	//			//
	//			//			VertexLayout picking_vertex_description(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });
	//			//
	//			//			GRenderingComponents::pickingManager().updateComponentProxyGeometry(constraintPoint.m_lefthand_gradient,
	//			//				gradient_select_vertices,
	//			//				lh_gradient_indices,
	//			//				picking_vertex_description,
	//			//				GL_LINES);
	//			//
	//			//
	//			//			std::vector<uint> rh_gradient_indices({ 0,2 });
	//			//
	//			//			GRenderingComponents::interfaceMeshManager().updateComponent(constraintPoint.m_righthand_gradient,
	//			//				gradient_interface_vertices,
	//			//				rh_gradient_indices,
	//			//				vertex_description,
	//			//				GL_LINES);
	//			//
	//			//			GRenderingComponents::pickingManager().updateComponentProxyGeometry(constraintPoint.m_righthand_gradient,
	//			//				gradient_select_vertices,
	//			//				rh_gradient_indices,
	//			//				picking_vertex_description,
	//			//				GL_LINES);
	//			//#endif
	//		}
	//	
	//		// recompute mesh data
	//		auto mesh_data = computeCurveProxyMesh(curve_idx);
	//		//GEngineCore::resourceManager().updateMeshAsync(proxy_mesh,mesh_data.first,mesh_data.second);
	//	
	//		//TODO move
	//		//	#if EDITOR_MODE // preprocessor definition
	//		//	
	//		//			VertexLayout vertex_description(60, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
	//		//				VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 3),
	//		//				VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 6),
	//		//				VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,sizeof(GLfloat) * 9),
	//		//				VertexLayout::Attribute(GL_FLOAT,2,GL_FALSE,sizeof(GLfloat) * 12),
	//		//				VertexLayout::Attribute(GL_FLOAT,1,GL_FALSE,sizeof(GLfloat) * 14) });
	//		//	
	//		//			GRenderingComponents::interfaceMeshManager().updateComponent(m_featureCurves[index].m_entity,
	//		//				m_featureCurves[index].m_mesh_vertices,
	//		//				m_featureCurves[index].m_mesh_indices,
	//		//				vertex_description,
	//		//				GL_TRIANGLES);
	//		//	
	//		//			GRenderingComponents::pickingManager().updateComponentProxyGeometry(m_featureCurves[index].m_entity,
	//		//				m_featureCurves[index].m_mesh_vertices,
	//		//				m_featureCurves[index].m_mesh_indices,
	//		//				vertex_description,
	//		//				GL_TRIANGLES);
	//		//	#endif
	//	}
	//	
	//	inline bool FeatureCurveComponentManager::isSurfaceSeed(uint curve_idx) const
	//	{
	//		return m_data[curve_idx].m_is_surface_seed;
	//	}
	//	
	//	inline ResourceID FeatureCurveComponentManager::getProxyMesh(uint curve_idx) const
	//	{
	//		return m_data[curve_idx].m_proxy_mesh;
	//	}
	//	
	//	Entity FeatureCurveComponentManager::getCurveFromConstraintPoint(Entity cp_entity) const
	//	{
	//		auto query = utility::entityToIndex(cp_entity.id(), m_cp_eID_to_curve_idx);
	//	
	//		if (query.first)
	//			return m_data[query.second].m_entity;
	//		else
	//			return GEngineCore::entityManager().invalidEntity();
	//	}
	//	
	//	Entity FeatureCurveComponentManager::getConstraintPointFromBitangent(Entity bitangent_entity) const
	//	{
	//		auto query = utility::entityToIndex(bitangent_entity.id(), m_bitangent_eID_to_cp_eID);
	//	
	//		if (query.first)
	//			return m_data[query.second].m_entity;
	//		else
	//			return GEngineCore::entityManager().invalidEntity();
	//	}
	//	
	//	Entity FeatureCurveComponentManager::addConstraintPoint(uint curve_idx, float curve_position)
	//	{
	//		auto bitangents = interpolateCurveBitangents(curve_idx, curve_position);
	//	
	//		return addConstraintPoint(curve_idx, curve_position, bitangents.first, bitangents.second);
	//	}
	//	
	//	Entity FeatureCurveComponentManager::addConstraintPoint(Entity curve_entity, float curve_position)
	//	{
	//		auto query = getIndex(curve_entity);
	//	
	//		if (std::get<0>(query))
	//		{
	//			return addConstraintPoint(std::get<1>(query), curve_position);
	//		}
	//		else
	//		{
	//			return GEngineCore::entityManager().invalidEntity();
	//		}
	//	}
	//	
	//	Entity FeatureCurveComponentManager::addConstraintPoint(uint curve_idx, float curve_position, Vec3 lefthand_bitangent, Vec3 righthand_bitangent)
	//	{
	//		Entity fc_entity = m_data[curve_idx].m_entity;
	//		Vec3 curve_point = GCoreComponents::bSplineManager().computeCurvePoint(fc_entity, curve_position);
	//	
	//		Entity cp_entity = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(cp_entity, curve_point, Quat(), Vec3(1.0));
	//		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(cp_entity), fc_entity);
	//	
	//		Entity lh_gradient_entity = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(lh_gradient_entity, curve_point, Quat(), Vec3(1.0));
	//		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(lh_gradient_entity), fc_entity);
	//	
	//		Entity rh_gradient_entity = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(rh_gradient_entity, curve_point, Quat(), Vec3(1.0));
	//		GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(rh_gradient_entity), fc_entity);
	//	
	//		// Gather data for Constraint Point
	//		auto surface_properties = interpolateCurveSurfaceProperties(curve_idx, curve_position);
	//		FeatureCurveComponentManager::ConstraintPoint cp(cp_entity,
	//			lefthand_bitangent,
	//			righthand_bitangent,
	//			curve_position,
	//			lh_gradient_entity,
	//			rh_gradient_entity,
	//			std::get<0>(surface_properties),
	//			std::get<1>(surface_properties),
	//			std::get<2>(surface_properties),
	//			std::get<3>(surface_properties));
	//	
	//		m_data[curve_idx].m_constraint_points.push_back(cp);
	//	
	//		m_cp_eID_to_curve_idx.insert(std::pair<uint, uint>(cp_entity.id(), curve_idx));
	//		m_bitangent_eID_to_cp_eID.insert(std::pair<uint, uint>(lh_gradient_entity.id(), cp_entity.id()));
	//		m_bitangent_eID_to_cp_eID.insert(std::pair<uint, uint>(rh_gradient_entity.id(), cp_entity.id()));
	//	
	//		// sorting the constraint points by curve_position simplfies computations below
	//		// but this might have to be improved later on
	//		std::sort(m_data[curve_idx].m_constraint_points.begin(), m_data[curve_idx].m_constraint_points.end(),
	//			[](const FeatureCurveComponentManager::ConstraintPoint& a, const FeatureCurveComponentManager::ConstraintPoint& b) { return a.m_curve_position < b.m_curve_position; });
	//	
	//	
	//		// TODO: Add cp mesh (curve mesh should not have changed)
	//	
	//		//#if EDITOR_MODE // preprocessor definition
	//		//
	//		//		Vec3 v0 = Vec3(0.0);
	//		//		Vec3 v1 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * cp.m_gradient_0;
	//		//		Vec3 v2 = v0 + 1.5f * m_featureCurves[index].m_ribbon_width * cp.m_gradient_1;
	//		//		std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,1.0,1.0,0.0,1.0,
	//		//			v1.x,v1.y,v1.z,1.0,1.0,0.0,1.0,
	//		//			v2.x,v2.y,v2.z,1.0,1.0,0.0,1.0 });
	//		//
	//		//		//std::vector<float> gradient_interface_vertices({ v0.x,v0.y,v0.z,0.47f,0.125f,0.125f,1.0f,
	//		//		//	v1.x,v1.y,v1.z,0.47f,0.125f,0.125f,1.0f,
	//		//		//	v2.x,v2.y,v2.z,0.47f,0.125f,0.125f,1.0f });
	//		//
	//		//		std::vector<float> gradient_select_vertices({ v0.x,v0.y,v0.z, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z });
	//		//
	//		//		//TODO insert selection mesh
	//		//		std::vector<uint> lh_gradient_indices({ 0,1 });
	//		//
	//		//		VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
	//		//			VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3) });
	//		//
	//		//		GRenderingComponents::interfaceMeshManager().addComponent(lh_gradient_entity,
	//		//			"cp_" + std::to_string(cp.m_entity.id()) + "_lh_gradients",
	//		//			"../resources/materials/editor/interface_cv.slmtl",
	//		//			gradient_interface_vertices,
	//		//			lh_gradient_indices,
	//		//			vertex_description,
	//		//			GL_LINES);
	//		//
	//		//		VertexLayout picking_vertex_description(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });
	//		//
	//		//		GRenderingComponents::pickingManager().addComponent(lh_gradient_entity,
	//		//			"cp_" + std::to_string(cp.m_entity.id()) + "_lh_gradient",
	//		//			"../resources/materials/editor/picking_cp.slmtl",
	//		//			gradient_select_vertices,
	//		//			lh_gradient_indices,
	//		//			picking_vertex_description,
	//		//			GL_LINES);
	//		//		//[cp]() { GTools::featureCurveGradientManipulator().activate(cp.m_lefthand_gradient, []() {}); },
	//		//		//[cp]() { GTools::featureCurveGradientManipulator().deactivate(); });
	//		//		GTools::selectManager().addComponent(lh_gradient_entity,
	//		//			[lh_gradient_entity]() { GTools::landscapeTool().activateConstraintPointTools(lh_gradient_entity); },
	//		//			[lh_gradient_entity]() { GTools::landscapeTool().deactivateConstraintPointTools(); });
	//		//
	//		//		std::vector<uint> rh_gradient_indices({ 0,2 });
	//		//
	//		//		GRenderingComponents::interfaceMeshManager().addComponent(rh_gradient_entity,
	//		//			"cp_" + std::to_string(cp.m_entity.id()) + "_rh_gradients",
	//		//			"../resources/materials/editor/interface_cv.slmtl",
	//		//			gradient_interface_vertices,
	//		//			rh_gradient_indices,
	//		//			vertex_description,
	//		//			GL_LINES);
	//		//
	//		//		GRenderingComponents::pickingManager().addComponent(rh_gradient_entity,
	//		//			"cp_" + std::to_string(cp.m_entity.id()) + "_rh_gradient",
	//		//			"../resources/materials/editor/picking_cp.slmtl",
	//		//			gradient_select_vertices,
	//		//			rh_gradient_indices,
	//		//			picking_vertex_description,
	//		//			GL_LINES);
	//		//		//[cp]() { GTools::featureCurveGradientManipulator().activate(cp.m_righthand_gradient, []() {}); },
	//		//		//[cp]() { GTools::featureCurveGradientManipulator().deactivate(); });
	//		//		GTools::selectManager().addComponent(rh_gradient_entity,
	//		//			[rh_gradient_entity]() { GTools::landscapeTool().activateConstraintPointTools(rh_gradient_entity); },
	//		//			[rh_gradient_entity]() { GTools::landscapeTool().deactivateConstraintPointTools(); });
	//		//#endif
	//	
	//		return cp_entity;
	//	}
	//	
	//	void FeatureCurveComponentManager::setConstraintPointCurvePosition(Entity cp_entity, float curve_position)
	//	{
	//		// TODO
	//	}
	//	
	//	void FeatureCurveComponentManager::setConstraintPointBitangent(Entity bitangent_entity, Vec3 new_bitangent)
	//	{
	//		auto cp_query = utility::entityToIndex(bitangent_entity.id(), m_bitangent_eID_to_cp_eID);
	//	
	//		if (std::get<0>(cp_query))
	//		{
	//			auto curve_query = utility::entityToIndex(std::get<1>(cp_query), m_cp_eID_to_curve_idx);
	//	
	//			if (std::get<0>(curve_query))
	//			{
	//				for (auto& cp : m_data[std::get<1>(curve_query)].m_constraint_points)
	//				{
	//					if (cp.m_entity.id() == std::get<1>(cp_query))
	//					{
	//						if (cp.m_lh_bitangent_entity == bitangent_entity)
	//						{
	//							cp.m_lefthand_bitangent = new_bitangent;
	//						}
	//						else if (cp.m_rh_bitangent_entity == bitangent_entity)
	//						{
	//							cp.m_righthand_bitangent = new_bitangent;
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//	
	//	void FeatureCurveComponentManager::setConstraintPointNoiseAmplitude(Entity cp_entity, float amplitude)
	//	{
	//		auto query = utility::entityToIndex(cp_entity.id(), m_cp_eID_to_curve_idx);
	//	
	//		if (std::get<0>(query))
	//		{
	//			for (auto& cp : m_data[std::get<1>(query)].m_constraint_points)
	//			{
	//				if (cp.m_entity == cp_entity)
	//				{
	//					cp.m_noise_amplitude = amplitude;
	//				}
	//			}
	//		}
	//	}
	//	
	//	void FeatureCurveComponentManager::setConstraintPointNoiseRoughness(Entity cp_entity, float roughness)
	//	{
	//		auto query = utility::entityToIndex(cp_entity.id(), m_cp_eID_to_curve_idx);
	//	
	//		if (std::get<0>(query))
	//		{
	//			for (auto& cp : m_data[std::get<1>(query)].m_constraint_points)
	//			{
	//				if (cp.m_entity == cp_entity)
	//				{
	//					cp.m_noise_roughness = roughness;
	//				}
	//			}
	//		}
	//	}
	//	
	//	void FeatureCurveComponentManager::setConstraintPointMaterials(Entity cp_entity, int lefthand_material, int righthand_material)
	//	{
	//		auto query = utility::entityToIndex(cp_entity.id(), m_cp_eID_to_curve_idx);
	//	
	//		if (std::get<0>(query))
	//		{
	//			for (auto& cp : m_data[std::get<1>(query)].m_constraint_points)
	//			{
	//				if (cp.m_entity == cp_entity)
	//				{
	//					cp.m_lefthand_material = lefthand_material;
	//					cp.m_righthand_material = righthand_material;
	//				}
	//			}
	//		}
	//	}
	//	
	//	std::pair<float, float> FeatureCurveComponentManager::getConstraintPointNoise(Entity cp_entity)
	//	{
	//		auto query = utility::entityToIndex(cp_entity.id(), m_cp_eID_to_curve_idx);
	//	
	//		if (std::get<0>(query))
	//		{
	//			for (auto& cp : m_data[std::get<1>(query)].m_constraint_points)
	//			{
	//				if (cp.m_entity == cp_entity)
	//				{
	//					return { cp.m_noise_amplitude, cp.m_noise_roughness };
	//				}
	//			}
	//		}
	//	
	//		return { -1.0f,-1.0f };
	//	}
	//	
	//	std::pair<int, int> FeatureCurveComponentManager::getConstraintPointMaterialIDs(Entity cp_entity)
	//	{
	//		auto query = utility::entityToIndex(cp_entity.id(), m_cp_eID_to_curve_idx);
	//	
	//		if (std::get<0>(query))
	//		{
	//			for (auto& cp : m_data[std::get<1>(query)].m_constraint_points)
	//			{
	//				if (cp.m_entity == cp_entity)
	//				{
	//					return { cp.m_lefthand_material, cp.m_righthand_material };
	//				}
	//			}
	//		}
	//	
	//		return { -1,-1 };
	//	}
	//	
	//	float FeatureCurveComponentManager::getConstraintPointCurvePosition(Entity cp_entity)
	//	{
	//		auto query = utility::entityToIndex(cp_entity.id(), m_cp_eID_to_curve_idx);
	//	
	//		if (std::get<0>(query))
	//		{
	//			for(auto& cp : m_data[std::get<1>(query)].m_constraint_points)
	//			{
	//				if (cp.m_entity == cp_entity)
	//				{
	//					return cp.m_curve_position;
	//				}
	//			}
	//		}
	//		
	//		return 0.0f;
	//	}
	//	
	//	Vec3 FeatureCurveComponentManager::getConstraintPointTangent(Entity cp_entity)
	//	{
	//		auto query = m_cp_eID_to_curve_idx.find(cp_entity.id());
	//	
	//		// TODO handle invald cp entity id
	//		assert(search != m_cp_index_map.end());
	//	
	//		uint idx = query->second;
	//	
	//		for (auto& cp : m_data[idx].m_constraint_points)
	//		{
	//			if (cp.m_entity == cp_entity)
	//			{
	//				return GCoreComponents::bSplineManager().computeCurveTangent(m_data[idx].m_entity, cp.m_curve_position);
	//			}
	//		}
	//	
	//		return Vec3();
	//	}

}
