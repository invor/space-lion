#include "BSplineComponent.hpp"

#include <algorithm>

#include "utility.hpp"

#include "TransformComponentManager.hpp"

Vec3 EngineCore::Common::BSplineComponentManager::recursiveDeBoor(size_t idx, uint k, uint i, float x) const
{
    if (k == 0)
    {
        auto& transform_mngr = m_world.get<EngineCore::Common::TransformComponentManager>();
        size_t transform_idx = transform_mngr.getIndex(m_data[idx].m_control_vertices[i]);
        return transform_mngr.getPosition(transform_idx);
    }
    else
    {
        float u_i = m_data[idx].m_knotvector[i];
        float u_in1k = m_data[idx].m_knotvector[i + m_data[idx].m_degree + 1 - k];

        float alpha_k_i = (x - u_i) / (u_in1k - u_i);

        return ((1.0f - alpha_k_i)*recursiveDeBoor(idx, k - 1, i - 1, x) + alpha_k_i*recursiveDeBoor(idx, k - 1, i, x));
    }
}

EngineCore::Common::BSplineComponentManager::BSplineComponentManager(WorldState& world)
    : m_world(world)
{
}

void EngineCore::Common::BSplineComponentManager::addComponent(Entity entity)
{
    auto& transform_mngr = m_world.get<EngineCore::Common::TransformComponentManager>();

    size_t idx = m_data.size();

    addIndex(entity.id(), idx);

    Entity cv_0 = m_world.accessEntityManager().create();
    transform_mngr.addComponent(cv_0, Vec3(-1.0, 0.0, 0.0));
    Entity cv_1 = m_world.accessEntityManager().create();
    transform_mngr.addComponent(cv_1, Vec3(1.0, 0.0, 0.0));

    m_data.push_back(ComponentData(entity));
    m_data[idx].m_degree = 1;
    m_data[idx].m_knotvector = {0.0f,0.0f,1.0f,1.0f};
    m_data[idx].m_control_vertices = {cv_0,cv_1};
}

void EngineCore::Common::BSplineComponentManager::addComponent(Entity entity, std::vector<ControlVertex> const& control_vertices)
{
    size_t idx = m_data.size();

    addIndex(entity.id(), idx);

    m_data.push_back(ComponentData(entity));
    m_data[idx].m_control_vertices = control_vertices;

    m_data[idx].m_degree = std::min( 3u, static_cast<uint>(m_data[idx].m_control_vertices.size())-1);

    m_data[idx].m_knotvector.resize(m_data[idx].m_control_vertices.size() + m_data[idx].m_degree + 1);

    // multiplicity depending on degree
    float knot_value = 0.0;
    for (uint i = 0; i < m_data[idx].m_knotvector.size(); ++i)
    {
        m_data[idx].m_knotvector[i] = knot_value;

        if ((i >= m_data[idx].m_degree) && (i < m_data[idx].m_knotvector.size() - 1 - m_data[idx].m_degree)){
            ++knot_value;
        }
    }
}

void EngineCore::Common::BSplineComponentManager::insertControlVertex(size_t component_idx, ControlVertex insert_location, ControlVertex control_vertex, bool insert_after)
{
    assert(component_idx < m_data.size());

    std::unique_lock<std::shared_mutex> lock(m_data_mutex);

    int cv_idx = -1;
    for (int i = 0; i < m_data[component_idx].m_control_vertices.size(); ++i)
    {
        if (insert_location == m_data[component_idx].m_control_vertices[i])
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
            m_data[component_idx].m_control_vertices.insert(m_data[component_idx].m_control_vertices.begin() + cv_idx + 1, control_vertex);
        else
            m_data[component_idx].m_control_vertices.insert(m_data[component_idx].m_control_vertices.begin() + cv_idx, control_vertex);

        // TODO clean-up knot vector adjustment

        // Adjust knot vector and degree
        if (m_data[component_idx].m_control_vertices.size() > 2 && m_data[component_idx].m_control_vertices.size() < 5)
        {
            size_t idx = m_data[component_idx].m_control_vertices.size() - 1;
            m_data[component_idx].m_knotvector[idx]--;

            float end_value = m_data[component_idx].m_knotvector.back();
            m_data[component_idx].m_knotvector.push_back(end_value);
            m_data[component_idx].m_knotvector.push_back(end_value);

            m_data[component_idx].m_degree++;

        }
        else if (m_data[component_idx].m_control_vertices.size() > 4)
        {
            size_t start_index = m_data[component_idx].m_control_vertices.size();
            size_t end_index = m_data[component_idx].m_knotvector.size();

            for (size_t i = start_index; i < end_index; i++)
            {
                m_data[component_idx].m_knotvector[i]++;
            }

            float end_value = m_data[component_idx].m_knotvector.back();
            m_data[component_idx].m_knotvector.push_back(end_value);
        }
    }
}

void EngineCore::Common::BSplineComponentManager::insertControlVertex(Entity entity, ControlVertex insert_location, ControlVertex control_vertex, bool insert_after)
{
    auto query = getIndex(entity);

    // if spline is found
    if (!query.empty())
    {
        insertControlVertex(query.front(), insert_location, control_vertex, insert_after);
    }
}

Vec3 EngineCore::Common::BSplineComponentManager::computeCurvePoint(size_t component_idx, float u) const
{
    u = std::min(1.0f, std::max(0.0f, u)); // clamp u to valid range

    // calculate i for recursive De Boor
    size_t cv_cnt = m_data[component_idx].m_control_vertices.size();
    size_t knotvector_length = m_data[component_idx].m_knotvector.size();
    uint degree = m_data[component_idx].m_degree;

    float knot_value = u * (cv_cnt - degree); // map to knotvector value range

    uint i = degree;
    for (; i < knotvector_length; ++i)
    {
        if (m_data[component_idx].m_knotvector[i + 1] >= knot_value)
            break;
    }

    return recursiveDeBoor(component_idx, degree, i, knot_value);
}

Vec3 EngineCore::Common::BSplineComponentManager::computeCurvePoint(Entity spline, float u) const
{
    auto query = getIndex(spline);

    Vec3 retval(0.0);

    // if spline is found
    if (!query.empty())
    {
        retval = computeCurvePoint(query.front(), u);
    }
    else
    {
        //TODO std:cerr<<"Entity is not a spline."<<std::endl;
    }

    return retval;
}

Vec3 EngineCore::Common::BSplineComponentManager::computeCurveTangent(size_t component_idx, float u) const
{
    Vec3 p1 = computeCurvePoint(component_idx, u + 0.02f);
    Vec3 p2 = computeCurvePoint(component_idx, u - 0.02f);

    return glm::normalize(p1 - p2);
}

Vec3 EngineCore::Common::BSplineComponentManager::computeCurveTangent(Entity spline, float u) const
{
    auto query = getIndex(spline);

    Vec3 retval(1.0, 0.0, 0.0);

    // if spline is found
    if (!query.empty())
    {
        retval = computeCurveTangent(query.front(), u);
    }
    else
    {
        //TODO std:cerr<<"Entity is not a spline."<<std::endl;
    }

    return retval;
}

Entity EngineCore::Common::BSplineComponentManager::getLastControlVertex(Entity spline) const
{
    auto query = getIndex(spline);

    auto retval = m_world.accessEntityManager().invalidEntity();

    // if spline is found
    if (!query.empty())
    {
        retval = m_data[query.front()].m_control_vertices.back();
    }
    else
    {
        //TODO std:cerr<<"Entity is not a spline."<<std::endl;
    }

    return retval;
}