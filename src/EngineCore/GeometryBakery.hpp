#ifndef GeometryBakery_hpp
#define GeometryBakery_hpp

#include <tuple>
#include <vector>

#include "types.hpp"
#include "VertexLayout.hpp"

typedef std::vector<uint8_t> VertexData;
typedef std::vector<uint32_t> IndexData;

namespace GeometryBakery
{

	/**
	* \brief Creates and return triangle geometry
	*/
	std::tuple<VertexData,IndexData,VertexLayout> createTriangle();

	/**
	* \brief Creates and return plane (quad) geometry
	*/
	std::tuple<VertexData, IndexData, VertexLayout> createPlane(float width, float height);

	/**
	* \brief Creates and returns unit box geometry
	*/
	std::tuple<VertexData, IndexData, VertexLayout> createBox();

	/**
	* \brief Create an ico sphere mesh
	* \param subdivisions Control the subdivions of the sphere
	* \return Returns shared pointer to the mesh
	*/
	std::tuple<VertexData, IndexData, VertexLayout> createIcoSphere(uint subdivions);

}

#endif // !GeometryBakery_hpp
