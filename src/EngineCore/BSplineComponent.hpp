#ifndef BSplineComponent_hpp
#define BSPlineComponent_hpp

#include <unordered_map>

#include "EntityManager.hpp"

class BSplineComponentManager
{
private:
	typedef Entity ControlVertex;

	struct Data
	{
		Data(Entity e) : m_entity(e), m_degree(0) {}

		Entity m_entity;

		uint m_degree;

		std::vector<float> m_knotvector;
		std::vector<ControlVertex> m_control_vertices;
	};

	std::vector<Data> m_data;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint, uint> m_index_map;

	/** Recursive De Boor algorithm as seen on Wikipedia */
	Vec3 recursiveDeBoor(uint index, uint k, uint i, float x) const;

public:

	void addComponent(Entity spline);

	void addComponent(Entity spline, std::vector<ControlVertex> const& control_vertices);

	std::pair<bool, uint> getIndex(Entity spline) const;

	/**
	* Insert a new control vertex to a specified location in the control vertex array of the specified spline curve
	* @param spline The Bspline curve that the control vertex is inserted in
	* @param insert_location Insert location given by a control vertex of the spline
	* @param control_vertex The control vertex that is added to the spline
	* @param insert_behind Specifies whether the new control vertex will be added before or after the given insert location
	*/
	void insertControlVertex(Entity spline, ControlVertex insert_location, ControlVertex control_vertex, bool insert_after = true);

	Vec3 computeCurvePoint(uint spline_idx, float u) const;

	Vec3 computeCurvePoint(Entity spline, float u) const;

	Vec3 computeCurveTangent(uint spline_idx, float u) const;

	Vec3 computeCurveTangent(Entity spline, float u) const;

	Entity getLastControlVertex(Entity spline) const;
};

#endif // !BSplineComponent_hpp
