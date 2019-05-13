#include "TerrainLoader.hpp"

#include "GlobalCoreComponents.hpp"
#include "TransformComponent.hpp"

void Landscape::storeLandscape(Entity landscape_entity, std::string file_path)
{
	LandscapeStorageHeader lcsp_header;
	std::vector<FeatureCurveStorageHeader> fc_headers;
	std::vector<ControlVertexStorage> cvs;
	std::vector<ConstraintPointStorage>	cps;

	// Get Landscape information
	uint landscape_idx = GLandscapeComponents::landscapeManager().getIndex(landscape_entity);

	std::vector<Entity>& feature_curves = GLandscapeComponents::landscapeManager().getFeatureCurveList(landscape_idx);
	std::vector<Entity>& bricks = GLandscapeComponents::landscapeManager().getBrickList(landscape_idx);

	//TODO: WRITE INFORMATION TO LCSP HEADER
	lcsp_header.num_feature_curves = static_cast<uint>(feature_curves.size());
	lcsp_header.position = GCoreComponents::transformManager().getWorldPosition(landscape_entity);
	lcsp_header.size = GLandscapeComponents::landscapeManager().getDimension(landscape_entity);

	// Access cv and cp vector
	for (auto& fc : feature_curves)
	{
		FeatureCurveComponentManager::FeatureCurveComponent& feature_curve = GLandscapeComponents::featureCurveManager().getCurve(fc);
		Vec3 featureCurve_position = GCoreComponents::transformManager().getWorldPosition(feature_curve.m_entity);

		fc_headers.push_back(FeatureCurveStorageHeader(featureCurve_position, static_cast<uint>(feature_curve.m_controlVertices.size()), static_cast<uint>(feature_curve.m_contraintPoints.size())));
		
		for (auto& cv : feature_curve.m_controlVertices)
		{
			uint cv_transform_idx = GCoreComponents::transformManager().getIndex(cv.m_entity);
			Vec3 cv_pos = GCoreComponents::transformManager().getPosition(cv_transform_idx);
			cvs.push_back(ControlVertexStorage(cv_pos));
		}

		for (auto& cp : feature_curve.m_contraintPoints)
		{
			cps.push_back(ConstraintPointStorage());

			cps.back().curve_position = cp.m_curve_position;
			cps.back().gradient_0 = cp.m_gradient_0;
			cps.back().gradient_1 = cp.m_gradient_1;
			cps.back().noise_amplitude = cp.m_noise_amplitude;
			cps.back().noise_roughness = cp.m_noise_roughness;
			cps.back().material_0 = cp.m_material_0;
			cps.back().material_1 = cp.m_material_1;
		}
	}

	// Compute overall required memory
	size_t byte_size = 0;
	byte_size += sizeof(LandscapeStorageHeader);
	byte_size += sizeof(FeatureCurveStorageHeader) * fc_headers.size();
	byte_size += sizeof(ControlVertexStorage) * cvs.size();
	byte_size += sizeof(ConstraintPointStorage) * cps.size();

	// allocate void* buffer and copy stuff
	void* buffer = new uint8_t[byte_size];

	long offset = 0;
	std::memcpy(buffer, &lcsp_header, sizeof(LandscapeStorageHeader));
	offset += sizeof(LandscapeStorageHeader);

	std::memcpy((uint8_t*)buffer + offset, fc_headers.data(), sizeof(FeatureCurveStorageHeader) * fc_headers.size());
	offset += sizeof(FeatureCurveStorageHeader) * static_cast<long>(fc_headers.size());

	std::memcpy((uint8_t*)buffer + offset, cvs.data(), sizeof(ControlVertexStorage) * cvs.size());
	offset += sizeof(ControlVertexStorage) * static_cast<long>(cvs.size());

	std::memcpy((uint8_t*)buffer + offset, cps.data(), sizeof(ConstraintPointStorage) * cps.size());

	// Write to disc
	std::string output_filepath = file_path;
	output_filepath.append(".sll");
	std::ofstream file(output_filepath, std::ios::out | std::ios::binary);
	file.write((char*)buffer, byte_size);

	delete buffer;
}

void Landscape::loadLandscape(std::string filepath)
{
	// check last three characters for expected file ending

	size_t filepath_length = filepath.size();

	if (filepath_length <= 3)
	{
		std::cerr << "Unexpected file format." << std::endl;
		return;
	}

	std::string file_ending = filepath.substr(filepath_length - 3, filepath_length-1);

	std::cout << file_ending << std::endl;

	if (file_ending == "slt")
		parseLegacyLandscape(filepath);
	else if (file_ending == "sll")
		parseLandscape(filepath);
	else
	{
		std::cerr << "Unexpected file format." << std::endl;
		return;
	}
}

void Landscape::parseLegacyLandscape(std::string filepath)
{
	std::ifstream file(filepath, std::ios::in | std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return;

	long byte_size = static_cast<long>(file.tellg());
	void* buffer = new uint8_t[byte_size];

	file.seekg(std::ios_base::beg);
	file.read((char*)buffer, byte_size);

	LandscapeStorageHeader lcsp_header = *(LandscapeStorageHeader*)buffer;

	std::vector<FeatureCurveStorageHeader> fc_headers(lcsp_header.num_feature_curves);
	std::memcpy(fc_headers.data(), (uint8_t*)buffer + sizeof(LandscapeStorageHeader), sizeof(FeatureCurveStorageHeader)*lcsp_header.num_feature_curves);

	FeatureCurveStorageHeader test = *(FeatureCurveStorageHeader*)((float*)((uint8_t*)buffer + sizeof(LandscapeStorageHeader)));

	// compute number of control vertices and constraint points
	uint num_cvs = 0;
	uint num_cps = 0;
	for (auto& fc_header : fc_headers)
	{
		num_cvs += fc_header.num_control_vertices;
		num_cps += fc_header.num_constraint_points;
	}

	std::vector<ControlVertexStorage> cvs(num_cvs);
	std::memcpy(cvs.data(), (uint8_t*)buffer + sizeof(LandscapeStorageHeader) + sizeof(FeatureCurveStorageHeader)*lcsp_header.num_feature_curves, sizeof(ControlVertexStorage)*num_cvs);

	std::vector<LegacyConstraintPointStorage> cps(num_cps);
	std::memcpy(cps.data(),
		(uint8_t*)buffer + sizeof(LandscapeStorageHeader)
		+ sizeof(FeatureCurveStorageHeader)*lcsp_header.num_feature_curves
		+ sizeof(ControlVertexStorage)*num_cvs,
		sizeof(LegacyConstraintPointStorage)*num_cps);


	// create a new landscape from the information
	//	Entity lscp_entity = GEngineCore::entityManager().create();
	//	GLandscapeComponents::landscapeManager().addComponent(lscp_entity,lcsp_header.position,lcsp_header.size);
	//	uint lscp_idx = GLandscapeComponents::landscapeManager().getIndex(lscp_entity);

	uint cv_offset = 0;
	uint cp_offset = 0;
	for (auto& fc_header : fc_headers)
	{
		// Add Feature Curve
		Entity featureCurve_entity = GLandscapeComponents::landscapeManager().addFeatureCurve(true, fc_header.position);
		uint featureCuve_idx = GLandscapeComponents::featureCurveManager().getIndex(featureCurve_entity);
		// Set position of intial control vertices
		FeatureCurveComponentManager::FeatureCurveComponent& featureCurve = GLandscapeComponents::featureCurveManager().getCurve(featureCurve_entity);
		GCoreComponents::transformManager().setPosition(featureCurve.m_controlVertices[0].m_entity, cvs[cv_offset].position);
		GCoreComponents::transformManager().setPosition(featureCurve.m_controlVertices[1].m_entity, cvs[cv_offset + 1].position);

		for (uint i = 2; i < fc_header.num_control_vertices; i++)
		{
			GLandscapeComponents::featureCurveManager().addControlVertex(featureCurve_entity, cvs[cv_offset + i].position);
		}

		cv_offset += fc_header.num_control_vertices;

		// Set values of intial constraint points
		featureCurve.m_contraintPoints[0].m_gradient_0 = cps[cp_offset].gradient_0;
		featureCurve.m_contraintPoints[0].m_gradient_1 = cps[cp_offset].gradient_1;
		featureCurve.m_contraintPoints[0].m_noise_amplitude = cps[cp_offset].noise_amplitude;
		featureCurve.m_contraintPoints[0].m_noise_roughness = cps[cp_offset].noise_roughness;

		featureCurve.m_contraintPoints[1].m_gradient_0 = cps[cp_offset + fc_header.num_constraint_points - 1].gradient_0;
		featureCurve.m_contraintPoints[1].m_gradient_1 = cps[cp_offset + fc_header.num_constraint_points - 1].gradient_1;
		featureCurve.m_contraintPoints[1].m_noise_amplitude = cps[cp_offset + fc_header.num_constraint_points - 1].noise_amplitude;
		featureCurve.m_contraintPoints[1].m_noise_roughness = cps[cp_offset + fc_header.num_constraint_points - 1].noise_roughness;

		for (uint i = 2; i < fc_header.num_constraint_points; i++)
		{
			Entity cp_entity = GLandscapeComponents::featureCurveManager().addConstraintPoint(featureCuve_idx, cps[cp_offset + i - 1].curve_position, cps[cp_offset + i - 1].gradient_0, cps[cp_offset + i - 1].gradient_1);
			GLandscapeComponents::featureCurveManager().setConstraintPointNoiseAmplitude(cp_entity, cps[cp_offset + i - 1].noise_amplitude);
			GLandscapeComponents::featureCurveManager().setConstraintPointNoiseRoughness(cp_entity, cps[cp_offset + i - 1].noise_roughness);
		}

		cp_offset += fc_header.num_constraint_points;

		GLandscapeComponents::featureCurveManager().updateCurve(featureCuve_idx);
	}
}

void Landscape::parseLandscape(std::string filepath)
{
	std::ifstream file(filepath, std::ios::in | std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return;

	long byte_size = static_cast<long>(file.tellg());
	void* buffer = new uint8_t[byte_size];

	file.seekg(std::ios_base::beg);
	file.read((char*)buffer, byte_size);

	LandscapeStorageHeader lcsp_header = *(LandscapeStorageHeader*)buffer;

	std::vector<FeatureCurveStorageHeader> fc_headers(lcsp_header.num_feature_curves);
	std::memcpy(fc_headers.data(), (uint8_t*)buffer + sizeof(LandscapeStorageHeader), sizeof(FeatureCurveStorageHeader)*lcsp_header.num_feature_curves);

	FeatureCurveStorageHeader test = *(FeatureCurveStorageHeader*)((float*)((uint8_t*)buffer + sizeof(LandscapeStorageHeader)));

	// compute number of control vertices and constraint points
	uint num_cvs = 0;
	uint num_cps = 0;
	for (auto& fc_header : fc_headers)
	{
		num_cvs += fc_header.num_control_vertices;
		num_cps += fc_header.num_constraint_points;
	}

	std::vector<ControlVertexStorage> cvs(num_cvs);
	std::memcpy(cvs.data(), (uint8_t*)buffer + sizeof(LandscapeStorageHeader) + sizeof(FeatureCurveStorageHeader)*lcsp_header.num_feature_curves, sizeof(ControlVertexStorage)*num_cvs);

	std::vector<ConstraintPointStorage> cps(num_cps);
	std::memcpy(cps.data(),
		(uint8_t*)buffer + sizeof(LandscapeStorageHeader)
		+ sizeof(FeatureCurveStorageHeader)*lcsp_header.num_feature_curves
		+ sizeof(ControlVertexStorage)*num_cvs,
		sizeof(ConstraintPointStorage)*num_cps);


	// create a new landscape from the information
	//	Entity lscp_entity = GEngineCore::entityManager().create();
	//	GLandscapeComponents::landscapeManager().addComponent(lscp_entity,lcsp_header.position,lcsp_header.size);
	//	uint lscp_idx = GLandscapeComponents::landscapeManager().getIndex(lscp_entity);

	uint cv_offset = 0;
	uint cp_offset = 0;
	for (auto& fc_header : fc_headers)
	{
		// Add Feature Curve
		Entity featureCurve_entity = GLandscapeComponents::landscapeManager().addFeatureCurve(true, fc_header.position);
		uint featureCuve_idx = GLandscapeComponents::featureCurveManager().getIndex(featureCurve_entity);
		// Set position of intial control vertices
		FeatureCurveComponentManager::FeatureCurveComponent& featureCurve = GLandscapeComponents::featureCurveManager().getCurve(featureCurve_entity);
		GCoreComponents::transformManager().setPosition(featureCurve.m_controlVertices[0].m_entity, cvs[cv_offset].position);
		GCoreComponents::transformManager().setPosition(featureCurve.m_controlVertices[1].m_entity, cvs[cv_offset + 1].position);

		for (uint i = 2; i < fc_header.num_control_vertices; i++)
		{
			GLandscapeComponents::featureCurveManager().addControlVertex(featureCurve_entity, cvs[cv_offset + i].position);
		}

		cv_offset += fc_header.num_control_vertices;

		// Set values of intial constraint points
		featureCurve.m_contraintPoints[0].m_gradient_0 = cps[cp_offset].gradient_0;
		featureCurve.m_contraintPoints[0].m_gradient_1 = cps[cp_offset].gradient_1;
		featureCurve.m_contraintPoints[0].m_noise_amplitude = cps[cp_offset].noise_amplitude;
		featureCurve.m_contraintPoints[0].m_noise_roughness = cps[cp_offset].noise_roughness;
		featureCurve.m_contraintPoints[0].m_material_0 = cps[cp_offset].material_0;
		featureCurve.m_contraintPoints[0].m_material_1 = cps[cp_offset].material_1;

		featureCurve.m_contraintPoints[1].m_gradient_0 = cps[cp_offset + fc_header.num_constraint_points - 1].gradient_0;
		featureCurve.m_contraintPoints[1].m_gradient_1 = cps[cp_offset + fc_header.num_constraint_points - 1].gradient_1;
		featureCurve.m_contraintPoints[1].m_noise_amplitude = cps[cp_offset + fc_header.num_constraint_points - 1].noise_amplitude;
		featureCurve.m_contraintPoints[1].m_noise_roughness = cps[cp_offset + fc_header.num_constraint_points - 1].noise_roughness;
		featureCurve.m_contraintPoints[1].m_material_0 = cps[cp_offset + fc_header.num_constraint_points - 1].material_0;
		featureCurve.m_contraintPoints[1].m_material_1 = cps[cp_offset + fc_header.num_constraint_points - 1].material_1;

		for (uint i = 2; i < fc_header.num_constraint_points; i++)
		{
			Entity cp_entity = GLandscapeComponents::featureCurveManager().addConstraintPoint(featureCuve_idx, cps[cp_offset + i - 1].curve_position, cps[cp_offset + i - 1].gradient_0, cps[cp_offset + i - 1].gradient_1);
			GLandscapeComponents::featureCurveManager().setConstraintPointNoiseAmplitude(cp_entity, cps[cp_offset + i - 1].noise_amplitude);
			GLandscapeComponents::featureCurveManager().setConstraintPointNoiseRoughness(cp_entity, cps[cp_offset + i - 1].noise_roughness);
			GLandscapeComponents::featureCurveManager().setConstrainPointMaterialIDs(cp_entity, cps[cp_offset + i - 1].material_0, cps[cp_offset + i - 1].material_1);
		}

		cp_offset += fc_header.num_constraint_points;

		GLandscapeComponents::featureCurveManager().updateCurve(featureCuve_idx);
	}
}