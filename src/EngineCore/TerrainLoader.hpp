#include "GlobalCoreComponents.hpp"
#include "GlobalLandscapeComponents.hpp"

#ifndef TerrainLoader_hpp
#define TerrainLoader_hpp

#include "Terrain.hpp"

#include <fstream>

namespace Landscape
{
	struct LegacyConstraintPointStorage
	{
		float curve_position;

		Vec3 gradient_0;			///< first gradient contraint 
		Vec3 gradient_1;			///< second gradient constraint

		float noise_amplitude;	///< noise amplitude constraint
		float noise_roughness;	///< noise roughness constraint
	};

	struct ConstraintPointStorage
	{
		float	curve_position;
		Vec3	gradient_0;			///< first gradient contraint 
		Vec3	gradient_1;			///< second gradient constraint
		float	noise_amplitude;	///< noise amplitude constraint
		float	noise_roughness;	///< noise roughness constraint
		int		material_0;			///< lefthand material index
		int		material_1;			///< righthand material index
	};

	struct ControlVertexStorage
	{
		ControlVertexStorage() {}
		ControlVertexStorage(Vec3 pos) : position(pos) {}

		Vec3 position;
	};

	struct FeatureCurveStorageHeader
	{
		FeatureCurveStorageHeader() : position(), num_control_vertices(0), num_constraint_points(0) {}
		FeatureCurveStorageHeader(Vec3 pos, uint num_cvs, uint num_cps) : position(pos), num_control_vertices(num_cvs), num_constraint_points(num_cps) {}

		Vec3 position;

		uint num_control_vertices;
		uint num_constraint_points;
	};

	struct LandscapeStorageHeader
	{
		Vec3 position;
		Vec3 size;
		uint bricks[3];

		uint num_feature_curves;
	};

	void storeLandscape(Entity landscape_entity, std::string file_path);

	void loadLandscape(std::string filepath);

	void parseLegacyLandscape(std::string filepath);

	void parseLandscape(std::string filepath);
}

#endif // !TerrainLoader_hpp