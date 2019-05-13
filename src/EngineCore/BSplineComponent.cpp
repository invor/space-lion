#include "BSplineComponent.hpp"

#include <algorithm>

#include "utility.hpp"

#include "GlobalEngineCore.hpp"

#include "GlobalCoreComponents.hpp"
#include "TransformComponent.hpp"

Vec3 BSplineComponentManager::recursiveDeBoor(uint index, uint k, uint i, float x) const
{
	if (k == 0)
	{
		uint transform_idx = GCoreComponents::transformManager().getIndex(m_data[index].m_control_vertices[i]);

		return GCoreComponents::transformManager().getPosition(transform_idx);
	}
	else
	{
		float u_i = m_data[index].m_knotvector[i];
		float u_in1k = m_data[index].m_knotvector[i + m_data[index].m_degree + 1 - k];

		float alpha_k_i = (x - u_i) / (u_in1k - u_i);

		return ((1.0f - alpha_k_i)*recursiveDeBoor(index, k - 1, i - 1, x) + alpha_k_i*recursiveDeBoor(index, k - 1, i, x));
	}
}

void BSplineComponentManager::addComponent(Entity spline)
{
	uint idx = m_data.size();
	m_index_map.insert(std::pair<uint, uint>(spline.id(), idx));

	Entity cv_0 = GEngineCore::entityManager().create();
	GCoreComponents::transformManager().addComponent(cv_0, Vec3(-1.0, 0.0, 0.0));
	Entity cv_1 = GEngineCore::entityManager().create();
	GCoreComponents::transformManager().addComponent(cv_1, Vec3(1.0, 0.0, 0.0));

	m_data.push_back(Data(spline));
	m_data[idx].m_degree = 1;
	m_data[idx].m_knotvector = {0.0f,0.0f,1.0f,1.0f};
	m_data[idx].m_control_vertices = {cv_0,cv_1};
}

void BSplineComponentManager::addComponent(Entity spline, std::vector<ControlVertex> const& control_vertices)
{
	uint idx = m_data.size();
	m_index_map.insert(std::pair<uint, uint>(spline.id(), idx));

	m_data.push_back(Data(spline));
	m_data[idx].m_control_vertices = control_vertices;

	m_data[idx].m_degree = std::min( static_cast<size_t>(3), m_data[idx].m_control_vertices.size()-1);

	m_data[idx].m_knotvector.resize(m_data[idx].m_control_vertices.size() + m_data[idx].m_degree + 1);

	// multiplicity depending on degree
	float knot_value = 0.0;
	for (int i = 0; i < m_data[idx].m_knotvector.size(); i++)
	{
		m_data[idx].m_knotvector[i] = knot_value;

		if ((i >= m_data[idx].m_degree) && (i < m_data[idx].m_knotvector.size() - 1 - m_data[idx].m_degree))
			++knot_value;
	}
}

std::pair<bool, uint> BSplineComponentManager::getIndex(Entity spline) const
{
	return utility::entityToIndex(spline.id(), m_index_map);
}

void BSplineComponentManager::insertControlVertex(Entity spline, ControlVertex insert_location, ControlVertex control_vertex, bool insert_after)
{
	auto query = getIndex(spline);

	// if spline is found
	if (query.first)
	{
		uint spline_idx = getIndex(spline).second;
		int cv_idx = -1;
		for (int i = 0; i < m_data[spline_idx].m_control_vertices.size(); ++i)
		{
			if (insert_location == m_data[spline_idx].m_control_vertices[i])
			{
				cv_idx = i;
				break;
			}
		}

		// if insert location is found
		if (cv_idx != -1)
		{
			// Insert control vertex into vector
			if (insert_after)
				m_data[spline_idx].m_control_vertices.insert(m_data[spline_idx].m_control_vertices.begin() + cv_idx + 1, control_vertex);
			else
				m_data[spline_idx].m_control_vertices.insert(m_data[spline_idx].m_control_vertices.begin() + cv_idx, control_vertex);

			// TODO clean-up knot vector adjustment

			// Adjust knot vector and degree
			if (m_data[spline_idx].m_control_vertices.size() > 2 && m_data[spline_idx].m_control_vertices.size() < 5)
			{
				size_t idx = m_data[spline_idx].m_control_vertices.size() - 1;
				m_data[spline_idx].m_knotvector[idx]--;

				float end_value = m_data[spline_idx].m_knotvector.back();
				m_data[spline_idx].m_knotvector.push_back(end_value);
				m_data[spline_idx].m_knotvector.push_back(end_value);

				m_data[spline_idx].m_degree++;

			}
			else if (m_data[spline_idx].m_control_vertices.size() > 4)
			{
				size_t start_index = m_data[spline_idx].m_control_vertices.size();
				size_t end_index = m_data[spline_idx].m_knotvector.size();

				for (size_t i = start_index; i<end_index; i++)
				{
					m_data[spline_idx].m_knotvector[i]++;
				}

				float end_value = m_data[spline_idx].m_knotvector.back();
				m_data[spline_idx].m_knotvector.push_back(end_value);
			}
		}
	}
}

Vec3 BSplineComponentManager::computeCurvePoint(uint spline_idx, float u) const
{
	u = std::min(1.0f, std::max(0.0f, u)); // clamp u to valid range

	// calculate i for recursive De Boor
	size_t cv_cnt = m_data[spline_idx].m_control_vertices.size();
	size_t knotvector_length = m_data[spline_idx].m_knotvector.size();
	uint degree = m_data[spline_idx].m_degree;

	float knot_value = u * (cv_cnt - degree); // map to knotvector value range

	uint i = degree;
	for (; i < knotvector_length; ++i)
	{
		if (m_data[spline_idx].m_knotvector[i + 1] >= knot_value)
			break;
	}

	return recursiveDeBoor(spline_idx, degree, i, knot_value);
}

Vec3 BSplineComponentManager::computeCurvePoint(Entity spline, float u) const
{
	auto query = getIndex(spline);

	// if spline is found
	if (query.first)
	{
		computeCurvePoint(query.second, u);
	}
	else
	{
		//TODO std:cerr<<"Entity is not a spline."<<std::endl;
		return Vec3(0.0);
	}
}

Vec3 BSplineComponentManager::computeCurveTangent(uint spline_idx, float u) const
{
	Vec3 p1 = computeCurvePoint(spline_idx, u + 0.02f);
	Vec3 p2 = computeCurvePoint(spline_idx, u - 0.02f);

	return glm::normalize(p1 - p2);
}

Vec3 BSplineComponentManager::computeCurveTangent(Entity spline, float u) const
{
	auto query = getIndex(spline);

	// if spline is found
	if (query.first)
	{
		return computeCurveTangent(query.second, u);
	}
	else
	{
		//TODO std:cerr<<"Entity is not a spline."<<std::endl;
		return Vec3(1.0,0.0,0.0);
	}
}

Entity BSplineComponentManager::getLastControlVertex(Entity spline) const
{
	auto query = getIndex(spline);

	// if spline is found
	if (query.first)
	{
		return m_data[query.second].m_control_vertices.back();
	}
	else
	{
		//TODO return "invalid entity"
		//TODO std:cerr<<"Entity is not a spline."<<std::endl;
		return GEngineCore::entityManager().create();
	}
}