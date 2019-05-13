#include "GlobalCoreComponents.hpp"
#include "GlobalLandscapeComponents.hpp"
#include "GlobalTools.hpp"

#ifndef Terrain_hpp
#define Terrain_hpp

#include "glowl.h"

#include "GlobalRenderingComponents.hpp"
#include "PickingComponent.hpp"

#include "EntityManager.hpp"
#include "DeferredRenderingPipeline.hpp"

#include "LandscapeEditorTools.hpp"

#include "ResourceLoading.hpp"

#include <array>
#include <atomic>
#include <mutex>
#include <unordered_map>

namespace Landscape
{

	// idea: landscape volume set to dynamic moves bricks around and recomputes values
	//		take boundary conditions from already existing neighbours
	//		use low resolution textures as intial solution for computing higher res (possibly store low res on CPU to save VRAM)
	//		 landscape volume set to static stores volume data on CPU to avoid costly recalculations


	class FeatureMeshComponentManager
	{
	private:
		struct Data
		{
			Data(Entity e, std::string mesh_path) 
				: m_entity(e),
				m_lower_corner(Vec3(0.0f)),
				m_upper_corner(Vec3(0.0f)),
				m_mesh_path(mesh_path),
				m_vertex_layout(VertexLayout(0, {})),
				m_mesh(nullptr) {}

			Entity					m_entity;

			Vec3					m_lower_corner;
			Vec3					m_upper_corner;

			std::string				m_mesh_path;

			std::vector<uint8_t>	m_vertex_data;
			std::vector<uint32_t>	m_index_data;
			VertexLayout			m_vertex_layout;

			Mesh*					m_mesh;
		};

		std::vector<Data> m_data;

		/** Map for fast entity to component index conversion */
		std::unordered_map<uint, uint> m_index_map;

	public:
		uint getIndex(Entity entity) const;

		void addComponent(Entity entity, std::string mesh_path);

		template<typename VertexContainer, typename IndexContainer>
		void addComponent(Entity entity,
			const std::string& name,
			const Vec3 lower_bb_corner,
			const Vec3 upper_bb_corner,
			const VertexContainer& vertices,
			const IndexContainer& indices,
			const VertexLayout& vertex_layout,
			GLenum mesh_type)
		{
			// TODO add multithread protection !
			uint idx = static_cast<uint>(m_data.size());
			m_index_map.insert(std::pair<uint, uint>(entity.id(), idx));

			m_data.push_back(Data(entity, name));

			//TODO set properties
			m_data.back().m_lower_corner = lower_bb_corner;
			m_data.back().m_upper_corner = upper_bb_corner;
			m_data.back().m_vertex_layout = vertex_layout;

			size_t byte_size = vertices.size() * sizeof(VertexContainer::value_type);

			m_data.back().m_vertex_data.resize(byte_size);
			std::memcpy(m_data.back().m_vertex_data.data(), vertices.data(), byte_size);

			m_data.back().m_index_data.resize(indices.size());
			std::copy(indices.begin(), indices.end(), m_data.back().m_index_data.begin());

			// Create GPU resources
			GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx, mesh_type]() {

				FeatureMeshComponentManager::Data& component_data = m_data[idx];

				component_data.m_mesh = GEngineCore::resourceManager().createMesh(component_data.m_mesh_path,
					component_data.m_vertex_data,
					component_data.m_index_data,
					component_data.m_vertex_layout,
					mesh_type).resource;

				// add interface mesh....TODO maybe use a different material
				GRenderingComponents::interfaceMeshManager().addComponent(component_data.m_entity, "../resources/materials/editor/interface_curve.slmtl", m_data.back().m_mesh);

				// add select component
				GRenderingComponents::pickingManager().addComponent<std::vector<uint8_t>, std::vector<uint32_t>>(component_data.m_entity,
					"feature_mesh_" + std::to_string(idx) + "_selectProxy",
					"../resources/materials/editor/picking.slmtl",
					m_data.back().m_vertex_data,
					m_data.back().m_index_data,
					m_data.back().m_vertex_layout,
					GL_TRIANGLES);
				GTools::selectManager().addComponent(component_data.m_entity, std::bind(&Editor::LandscapeTools::activateFeatureMeshTool, &GTools::landscapeTool(), component_data.m_entity),
					std::bind(&Editor::LandscapeTools::deactivateFeatureMeshTool, &GTools::landscapeTool()));

				//TODO clear mesh data from CPU memory?
			});
		}

		Vec3 getLowerCorner(uint index) const;
		Vec3 getUpperCorner(uint index) const;

		Mesh const * const getMesh(uint index) const;
	};

	/**
	 * Class managing landscape feature curves
	 */
	class FeatureCurveComponentManager
	{
	public:
		struct FeatureCurveComponent
		{
			struct ControlVertex
			{
				ControlVertex(Entity entity)
					: m_entity(entity) {}

				Entity m_entity;
			};

			struct ConstraintPoint
			{
				ConstraintPoint(Entity entity, Entity lh_gradient_entity, Entity rh_gradient_entity, float curve_position)
					: m_entity(entity),
					m_lefthand_gradient(lh_gradient_entity),
					m_righthand_gradient(rh_gradient_entity),
					m_curve_position(curve_position),
					m_noise_amplitude(0.4f),
					m_noise_roughness(10.0f),
					m_material_0(0),
					m_material_1(0) {}

				Entity	m_entity;					///< entity that constraint point is associated with

				Entity	m_lefthand_gradient;		///< entity that lefthand bitangent vector is associated with
				Entity	m_righthand_gradient;		///< entity that righthand bitangent vector is associated with

				float	m_curve_position;			///< constraint point position on the curve (in normalized curve parameter space)

				Vec3	m_tangent;					///< curve tangent at constraint point location (note: not guaranteed to be up-to-date)
				Vec3	m_gradient_0;				///< lefthand bitangent vector constraint 
				Vec3	m_gradient_1;				///< righthand bitangent vector constraint

				float	m_noise_amplitude;			///< noise amplitude constraint
				float	m_noise_roughness;			///< noise roughness constraint

				int		m_material_0;				///< lefthand material index
				int		m_material_1;				///< righthand material index
			};


			FeatureCurveComponent(Entity entity);
			~FeatureCurveComponent();

			Entity m_entity;

			uint degree;

			std::vector<ControlVertex> m_controlVertices;
			std::vector<float> m_knots;
			std::vector<ConstraintPoint> m_contraintPoints;

			bool m_is_surface_seed;

			Vec3 m_lower_corner;
			Vec3 m_upper_corner;

			float m_ribbon_width;
			float m_mesh_pointDistance;
			std::vector<float> m_mesh_curvePoints;
			std::vector<float> m_mesh_vertices;
			std::vector<uint> m_mesh_indices;

			ShaderStorageBufferObject* m_mesh_vertices_ssbo;
			ShaderStorageBufferObject* m_mesh_indices_ssbo;

			ResourceID m_segement_textures;
		};
	private:
	
		std::vector<FeatureCurveComponent> m_featureCurves;
	
		/** Mapping from curve entity to curve index */
		std::unordered_map<uint,uint> m_curve_index_map;

		// Mappings used to find the feature curve that needs to be updated if a CV or CP is selected and manipulated
		/** Mapping from control vertex entity to index of curve that the CV belongs to */
		std::unordered_map<uint,uint> m_cv_index_map;
		/** Mapping from constrain point entity to index of curve that the CP belongs to */
		std::unordered_map<uint,uint> m_cp_index_map;
		/** Mapping from gradient entity id to constraint point entity id */
		std::unordered_map<uint, Entity> m_gradient_cp_entity_map;

		/** Mutex to protext feature curve storage vector during multi-threaded access */
		mutable std::mutex m_dataAccess_mutex;
		
		/** Recursive De Boor algorithm as seen on Wikipedia */
		Vec3 recursiveDeBoor(uint index, uint k, uint i, float x);

		Vec3 computeCurveTangent(uint index, float u);

		std::tuple<Vec3,Vec3> computeCurveGradients(uint index, float u);

		/** Recomputes the mesh data  */
		void recomputeCurveMesh(uint index);

		/** Buffer mesh data to the GPU. Only call this from an OpenGL thread! */
		void bufferMeshData(uint index);

		void bakeSegementTextures(uint index);

	public:
	
		/** Add a new feature curve */
		void addComponent(Entity entity, Vec3 position, Quat orientation, bool is_surface_seed);
	
		/** Deletes a specific feature curve, as well as all control and constraint points associated with it and frees entities */
		void deleteComponent(uint index);

		/** Returns the index of a given entity */
		uint getIndex(Entity entity) const;

		FeatureCurveComponent& getCurve(Entity entity);

		Entity getCurveFromControlVertex(Entity cv_entity);

		Entity getCurveFromConstraintPoint(Entity cp_entity);

		Entity getConstraintPointFromGradient(Entity gradient);

		Vec3 getConstraintPointTangent(Entity constraint_point);
	
		/** Adds a control vertex to a given feature curve. This automatically creates a new entity for the CV and required components */
		void addControlVertex(uint index, Vec3 position);

		void addControlVertex(Entity curve, Vec3 position);

		Entity insertControlVertex(Entity curve, Entity insert_point_cv, Vec3 position, bool insert_behind);
	
		/** Adds a constraint point to a given feature curve. This automatically creates a new entity for the CP and required components */
		Entity addConstraintPoint(uint index, float curve_position);

		Entity addConstraintPoint(Entity curve_entity, float curve_position);

		Entity addConstraintPoint(uint index, float curve_position, Vec3 gradient0, Vec3 gradient1);

		void setConstraintPointCurvePosition(Entity cp_entity, float curve_position);

		float getConstraintPointCurvePosition(Entity cp_entity);

		/** */
		void setConstraintPointGradient(Entity gradient_entity, Vec3 new_gradient);

		/** Set the one of the gradients of a constraint point given by its entity id */
		void setConstraintPointGradient(Entity cp_entity, Vec3 gradient, int gradient_idx);

		/** Set the one of the gradients of a constraint pointgiven by its current index in the constraint point array */
		void setConstraintPointGradient(uint curve_idx, uint cp_idx, Vec3 gradient, int gradient_idx);

		void setConstraintPointGradient(Entity curve, uint cp_idx, Vec3 gradient, int gradient_idx);

		void setConstraintPointNoise(uint curve_idx, uint cp_idx, float amplitude, float roughness);

		void setConstraintPointNoise(Entity curve, uint cp_idx, float amplitude, float roughness);

		void setConstraintPointNoiseAmplitude(Entity cp, float amplitude);

		float getConstraintPointNoiseAmplitude(Entity cp);

		void setConstraintPointNoiseRoughness(Entity cp, float roughness);

		float getConstraintPointNoiseRoughness(Entity cp);

		void setConstrainPointMaterialIDs(Entity cp, int matID_0, int matID_1);

		std::pair<int, int> getConstraintPointMaterialIDs(Entity cp);

		Entity getPreviousCV(Entity cv);

		Entity getNextCV(Entity cv);

		/** Set the ribbon width of all feature curves to a new value */
		void setRibbonWidth(float ribbon_width);

		/** Recomputes constraint point positions and mesh data */
		void updateCurve(uint index);

		/** Recomputes constraint point positions and mesh data of the curve associated with a given control vertex */
		void updateCorrespondingCurve(uint cv_entity_id);

		/** Uses De Boor's Algorithm to compute the 3d position of a point on the curve  given its parameterized position */
		Vec3 calculateCurvePoint(uint index, float curve_position);

		Vec3 getLowerCorner(uint index);

		Vec3 getUpperCorner(uint index);

		bool isSurfaceSeed(uint index);

		ShaderStorageBufferObject* getVertexBuffer(uint index);

		ShaderStorageBufferObject* getIndexBuffer(uint index);

		std::vector<FeatureCurveComponent::ControlVertex> getControlVertices(Entity e) const;

		std::vector<FeatureCurveComponent::ConstraintPoint> getConstraintPoints(Entity e) const;
	};

	/** 
	 * Class managing all landscape bricks.
	 */
	class LandscapeBrickComponentManager
	{
	public:
		enum NeighbourDirection { EAST, WEST, DOWN, UP, SOUTH, NORTH };
		enum Datafield { NORMAL, GRADIENT, NOISE, SURFACE, SURFACE_BOUNDARY };

	private:
		struct LandscapeBrickComponent
		{
			LandscapeBrickComponent(Entity entity, Vec3 dimensions, uint res_x, uint res_y, uint res_z)
				: m_entity(entity), m_dimensions(dimensions), m_res_x(res_x), m_res_y(res_y), m_res_z(res_z),
				m_normals(nullptr), m_normals_backbuffer(nullptr), m_head(nullptr), m_guidancefield_data(nullptr),
				m_gradients(nullptr), m_gradients_backbuffer(nullptr),
				m_noise_params(nullptr), m_surface(nullptr), m_surface_backbuffer(nullptr), m_surface_boundaryRegion(nullptr),
				m_upper_neighbour(entity),
				m_lower_neighbour(entity),
				m_south_neighbour(entity),
				m_north_neighbour(entity),
				m_west_neighbour(entity),
				m_east_neighbour(entity),
				m_debugField_selection(Datafield::NORMAL),
				m_lod_lvls(0),
				m_ptex_ready(true),
				m_cancel_ptex_update(false),
				m_ptex_mesh(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_textures(),
				m_ptex_material(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_material_bth(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_parameters(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_parameters_backbuffer(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_bindless_texture_handles(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_bindless_image_handles(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_bindless_mipmap_image_handles(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_tiles_per_edge(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_updatePatches_tgt_SSBO(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_updatePatches_src_SSBO(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_patch_distances_SSBO(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_availableTiles_SSBO(GEngineCore::resourceManager().getInvalidResourceID()),
				m_ptex_decal_list_rsrc(GEngineCore::resourceManager().getInvalidResourceID()),
				ptex_distance_computation_time(0.0),
				ptex_distance_computation_avg_time(0.0),
				ptex_distance_computation_max_time(0.0),
				ptex_updateList_computation_time(0.0),
				ptex_updateList_computation_avg_time(0.0),
				ptex_updateList_computation_max_time(0.0),
				ptex_tileUpdate_time(0.0),
				ptex_tileUpdate_avg_time(0.0),
				ptex_tileUpdate_max_time(0.0),
				updates_made(0),
				ptex_updated_primitives(0),
				ptex_updated_primitives_avg(0),
				ptex_updated_primitives_max(0) {}

			Entity m_entity;

			Vec3 m_dimensions;
			uint m_res_x, m_res_y, m_res_z;

			/** Volume containing surface normals. Guidance field for building the surface. */
			Texture3D* m_normals;
			Texture3D* m_normals_backbuffer;

			/** Volume containing surface gradients. Part of the guidance field for building the surface. */
			Texture3D* m_gradients;
			Texture3D* m_gradients_backbuffer;

			ShaderStorageBufferObject* m_head;
			ShaderStorageBufferObject* m_guidancefield_data;
			GLuint m_counter_buffer;

			/** 3D field of 2D containing surface noise parameters */
			Texture3D* m_noise_params;

			/** 3D field of 2D entries containing surface signed distance field and surface propagation weights */
			Texture3D* m_surface;
			Texture3D* m_surface_backbuffer;
			Texture3D* m_surface_boundaryRegion;

			// Boundary conditions of brick
			Texture3D* m_upper_boundary[4];
			Texture3D* m_lower_boundary[4];
			Texture3D* m_northern_boundary[4];
			Texture3D* m_southern_boundary[4];
			Texture3D* m_western_boundary[4];
			Texture3D* m_eastern_boundary[4];

			Entity m_upper_neighbour;
			Entity m_lower_neighbour;
			Entity m_south_neighbour;
			Entity m_north_neighbour;
			Entity m_west_neighbour;
			Entity m_east_neighbour;

			/** Mesh representation of computed surface */
			Mesh* m_surface_mesh;
			Material* m_surface_material;
			Material* m_shadowCaster_material;

			ResourceID				m_ptex_mesh;
			std::vector<ResourceID> m_ptex_textures;
			ResourceID				m_ptex_material;
			ResourceID				m_ptex_material_bth; // ptex surface material bindless texture handles
			ResourceID				m_ptex_parameters;
			ResourceID				m_ptex_parameters_backbuffer;

			ResourceID				m_ptex_bindless_texture_handles;
			ResourceID				m_ptex_bindless_image_handles;
			ResourceID				m_ptex_bindless_mipmap_image_handles;
			ResourceID				m_ptex_tiles_per_edge;

			/** CPU-side storage of computed ptex surface mesh */
			std::vector<float>		m_mesh_vertex_data;
			std::vector<uint32_t>	m_mesh_index_data;

			struct PtexParameters
			{
				int32_t ngbr_ptex_param_indices[4]; // index into PtexParameters buffers
				uint32_t tile_classification;

				// Save space by storing index into buffer with all texture handles instead of storing 64bit handles
				// (for texture arrays handle is shared by many faces)
				uint32_t texture_index; // index into texture indirection buffer (where all bindless texture handles are stored)
				uint32_t base_slice; // slice in texture array
			};
			std::vector<PtexParameters> m_mesh_ptex_params;


			// Level of detail ptex tile management prototype

			int m_lod_lvls;
			bool m_ptex_ready;
			bool m_cancel_ptex_update;

			std::vector<uint>	m_ptex_updatePatches_tgt;
			std::vector<uint>	m_ptex_updatePatches_src;
			ResourceID			m_ptex_updatePatches_tgt_SSBO; // SSBO containing the tile IDs that need to be updated ordered by update target tile size
			ResourceID			m_ptex_updatePatches_src_SSBO; // SSBO containing the tile IDs that need to be updated ordered by previous tile size
			std::vector<uint>	m_ptex_update_bin_sizes;

			struct PatchInfo
			{
				Vec3 midpoint;
				float distance;
				uint32_t tex_index;
				uint32_t base_slice;
			};

			struct TextureSlot
			{
				uint32_t tex_index;
				uint32_t base_slice;
			};

			ResourceID				m_ptex_patch_distances_SSBO;
			std::vector<PatchInfo>	m_ptex_patch_distances;
			std::vector<uint>		m_ptex_lod_bin_sizes; // store number of texture tiles per LOD level
			std::vector<uint>		m_ptex_active_patch_lod_classification; // store currently active (i.e. rendered) LOD classification of all patches
			std::vector<uint>		m_ptex_latest_patch_lod_classification; // store lastest LOD classification of all patches

			std::vector<std::list<TextureSlot>>	m_ptex_active_availableTiles;
			std::vector<std::list<TextureSlot>>	m_ptex_latest_availableTiles;

			std::vector<TextureSlot>			m_ptex_availableTiles_uploadBuffer;
			std::vector<uint>					m_ptex_availableTiles_bin_sizes;
			ResourceID							m_ptex_availableTiles_SSBO;

			struct Decals {
				uint decal_cnt;
				uint decal_idx_0;
				uint decal_idx_1;
				uint decal_idx_2;
				uint decal_idx_3;
				uint decal_idx_4;
				uint decal_idx_5;
				uint decal_idx_6;
			};
			std::vector<Decals> m_ptex_decal_list;
			ResourceID			m_ptex_decal_list_rsrc; // buffer with a per-primitive list of decals

			double ptex_distance_computation_time;
			double ptex_distance_computation_avg_time;
			double ptex_distance_computation_max_time;
			double ptex_updateList_computation_time;
			double ptex_updateList_computation_avg_time;
			double ptex_updateList_computation_max_time;
			double ptex_tileUpdate_time;
			double ptex_tileUpdate_avg_time;
			double ptex_tileUpdate_max_time;
			uint updates_made;
			uint ptex_updated_primitives;
			uint ptex_updated_primitives_avg;
			uint ptex_updated_primitives_max;
			

			Datafield m_debugField_selection;

			/** List of features curves located within the volume brick */
			std::vector<Entity> m_featureCurves;

			std::vector<Entity> m_featureMeshes;

			std::vector<Entity> m_heightmaps;
		};

		std::vector<LandscapeBrickComponent> m_bricks;
	
		std::unordered_map<uint,uint> m_index_map;

		bool m_voxelize_featureCurves;
		bool m_voxelize_featureMeshes;
		bool m_voxelize_heightmaps;

		mutable std::mutex m_ptex_update;

		void expandFeatureCurves(uint index);

		/******************************************************
		* GPU methods (only call on render thread!)
		*****************************************************/

		GLSLProgram* voxelize_prgm;
		GLSLProgram* voxelize_mesh_prgm;
		GLSLProgram* voxelize_heightmapMesh_prgm;
		GLSLProgram* average_prgm;
		GLSLProgram* reset_prgm;
		GLSLProgram* buildGuidance_prgm;
		GLSLProgram* buildNoiseField_prgm;
		GLSLProgram* copyNoiseField_prgm;
		GLSLProgram* surfacePropagationInit_prgm;
		GLSLProgram* surfacePropagation_prgm;
		GLSLProgram* copySurfaceField_prgm;
		GLSLProgram* smooth_prgm;
		GLSLProgram* classifyVoxels_prgm;
		GLSLProgram* buildHpLvl_R8_prgm;
		GLSLProgram* buildHpLvl_R8toR16_prgm;
		GLSLProgram* buildHpLvl_R16_prgm;
		GLSLProgram* buildHpLvl_R16toR32_prgm;
		GLSLProgram* buildHpLvl_R32_prgm;
		GLSLProgram** generateTriangles_prgms;

		GLSLProgram* surfaceNets_classify_prgm;
		GLSLProgram* surfaceNets_generateQuads_prgm;
		GLSLProgram* computePtexNeighbours_prgm;
		GLSLProgram* computePatchDistances_prgm;
		GLSLProgram* updateTextureTiles_prgm; // deprecated
		GLSLProgram* updatePtexTiles_prgm;
		GLSLProgram* updatePtexTilesMipmaps_prgm;
		GLSLProgram* textureBaking_prgm;
		GLSLProgram* setInitialLODTextureTiles_prgm;
		GLSLProgram* setPtexVistaTiles_prgm;
		GLSLProgram* updatePtexTilesDisplacement_prgm; // future work
		GLSLProgram* updatePtexTilesTextures_prgm; // future work

		GLSLProgram* transformFeedback_terrainOutput_prgm;
		GLuint transformFeedback_terrainBuffer;

		void createGpuResources(uint index);
		void updateGpuResources(uint index);
		void resetFields(uint index);
		void voxelizeFeatureCurves(uint index);
		void voxelizeFeatureMeshes(uint index);
		void voxelizeHeightmapMeshes(uint index);
		void averageVoxelization(uint index);
		void computeGuidanceField(uint index, uint iterations = 0);
		void computeNoiseField(uint index, uint interations = 0);
		void computeSurfacePropagation(uint index, uint iterations = 0);
		void smoothSurfaceField(uint index);
		void computeSurfaceMesh(uint index);
		void computeNaiveSurfaceNetsMesh(uint index);
		void bakeSurfaceTextures(uint index);

		void computePatchDistances(uint index);
		void computeTextureTileUpdateList(uint index);
		void updateTextureTiles(uint index);
		
		void fillSurfaceMeshGaps(uint index);
		void addDebugVolume(uint index);
		void updateDebugVolume(uint index);

	public:
		LandscapeBrickComponentManager();
		~LandscapeBrickComponentManager();

		//LandscapeBrickComponentManager(LandscapeBrickComponentManager const& cpy) = delete;

		void addComponent(Entity entity, Vec3 position, Vec3 dimension, uint res_x, uint res_y, uint res_z);

		void deleteComponent(uint index);
		
		uint getIndex(Entity entity);

		void addFeatureCurve(uint brick_index, Entity feature_curve_entity);

		void addFeatureMesh(uint brick_index, Entity feature_mesh_entity);

		void addHeightmapMesh(uint brick_index, Entity heightmap_entity);

		void clearFeatureCurves(uint brick_index);

		bool isEmpty(uint brick_index);

		void setNeighbour(uint index, NeighbourDirection direction, Entity neighbour);

		void updateBrick(uint index);

		void updateNoiseAndMaterial(uint index);

		void updateTextureBaking(uint index);//TODO move

		template<typename EntityContainer>
		void updateBricks(EntityContainer brick_entities)
		{
			std::vector<uint> brick_indices;
			std::vector<uint> empty_bricks;
			std::vector<uint> brick_iterations;
			std::vector<uint> empty_brick_iterations;
			uint max_iterations_cnt = 0;

			for (auto brick : brick_entities)
			{
				uint brick_idx = getIndex(brick);

				if (!isEmpty(brick_idx))
				{
					brick_indices.push_back(brick_idx);
					brick_iterations.push_back( static_cast<uint>(std::sqrt(m_bricks[brick_idx].m_res_x*m_bricks[brick_idx].m_res_x +
															m_bricks[brick_idx].m_res_y*m_bricks[brick_idx].m_res_y +
															m_bricks[brick_idx].m_res_z*m_bricks[brick_idx].m_res_z) ) );

					max_iterations_cnt = std::max(max_iterations_cnt, brick_iterations.back());
				}
				else
				{
					empty_bricks.push_back(brick_idx);
					empty_brick_iterations.push_back( static_cast<uint>( std::sqrt(m_bricks[brick_idx].m_res_x*m_bricks[brick_idx].m_res_x +
																m_bricks[brick_idx].m_res_y*m_bricks[brick_idx].m_res_y +
																m_bricks[brick_idx].m_res_z*m_bricks[brick_idx].m_res_z) ) );
				}
			}

			// Reset fields of bricks (both non-empty and empty)
			for (auto brick_idx : brick_indices)
				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().resetFields(brick_idx); });
			for (auto brick_idx : empty_bricks)
				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().resetFields(brick_idx); });

			// Voxelize Feature Curves in non-empty bricks
			for (auto brick_idx : brick_indices)
				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().voxelizeFeatureCurves(brick_idx); });

			// Compute guidance and noise fields
			for (uint i = 0; i < max_iterations_cnt /2; i++)
			{
				for (auto brick_idx : brick_indices)
				{
					GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeGuidanceField(brick_idx,2); });
					GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeNoiseField(brick_idx,2); });
				}
			}
			//	for (auto brick_idx : brick_indices)
			//	{
			//		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeGuidanceField(brick_idx, per_brick_iterations); });
			//		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeNoiseField(brick_idx, per_brick_iterations); });
			//	}


			// Compute surface propagation
			// Note: surface propagation does 2 iterations internally
			for (uint i = 0; i < max_iterations_cnt/2; i++)
			{
				for (auto brick_idx : brick_indices)
				{
					GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeSurfacePropagation(brick_idx,1); });
				}
			}
			//for (auto brick_idx : brick_indices)
			//{
			//	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeSurfacePropagation(brick_idx, per_brick_iterations / 2); });
			//}

			// Smooth result of surface propagation
			for (auto& brick_idx : brick_indices)
				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().smoothSurfaceField(brick_idx); });

			// Compute surface mesh
			for (auto& brick_idx : brick_indices)
				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeSurfaceMesh(brick_idx); });

			//for (auto& brick_idx : brick_indices)
			//	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().addDebugVolume(brick_idx);
			//																						GLandscapeComponents::landscapeManager().setReady(brick_idx); });

			// Fill bricks without feature curves after all others have been computed
			for (uint i = 0; i < max_iterations_cnt / 2; i++) //TODO use number of actually required iterations
			{
				for (auto brick_idx : empty_bricks)
				{
					GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeGuidanceField(brick_idx, 2); });
					GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeNoiseField(brick_idx, 2); });
				}
			}

			// Note: surface propagation does 2 iterations internally
			for (uint i = 0; i < max_iterations_cnt / 2; i++) //TODO use number of actually required iterations
			{
				for (auto brick_idx : empty_bricks)
				{
					GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().computeSurfacePropagation(brick_idx, 1); });
				}
			}

			for (auto& brick_idx : empty_bricks)
				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().smoothSurfaceField(brick_idx); });

			for (auto& brick_idx : empty_bricks)
				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx]
			{
				GLandscapeComponents::brickManager().computeSurfaceMesh(brick_idx);
			});

		}

		void updateCorrespondingBricks(Entity modified_feature_curve);

		void exportMesh(uint index, std::string export_filepath);

		Vec3 getDimension(uint index);

		void setResolution(uint index, uint resX, uint resY, uint resZ);

		Vec3 getResolution(uint index);

		void setDebugField(uint index, Datafield field_selection);

		void setSurfaceMaterial(uint index, Material* mat);

		void setVoxelizeFeatureCurves(bool voxelize);
		void setVoxelizeFeatureMeshes(bool voxelize);
		void setVoxelizeHeightmaps(bool voxelize);

		void updateShaderPrograms();
	};


	/**
	 * Class managing static landscape components.
	 * Position and dimension of static landscape are static, i.e. are not changed based
	 * on camera movement. Each component simply stores lists of all feature curves and all landscape bricks
	 * that belong to this landscape, as well as the landscapes dimensions. The actual landscape data
	 * is stored in the landscape bricks.
	 * Bricks are created on creation of a new landscape, feature curve can be added afterwards
	 */
	class StaticLandscapeComponentManager
	{		
	private:
	
		struct StaticLandscapeComponent
		{
			StaticLandscapeComponent(Entity entity, Vec3 dimension)
				: m_entity(entity), m_dimensions(dimension), m_ready(true), m_surface_material(GEngineCore::resourceManager().getInvalidResourceID())
			{
				MaterialInfo surface_ptex_material_info = ResourceLoading::parseMaterial("../resources/materials/landscape_ptex.slmtl");
				m_surface_material = GEngineCore::resourceManager().createMaterialAsync("landscape_ptex_material", surface_ptex_material_info.shader_filepaths, surface_ptex_material_info.texture_filepaths);
			}

			Entity m_entity;

			Vec3 m_dimensions;

			std::vector<Entity> m_featureCurves;
			std::vector<Entity> m_featureMeshes;
			std::vector<Entity> m_heightmaps;

			std::vector<Entity> m_bricks;

			ResourceID m_surface_material;

			bool m_ready;
		};
	
		std::vector<StaticLandscapeComponent> m_landscapes;
	
		std::unordered_map<uint,uint> m_index_map;

		//void drawDebugInterface();
	
	public:
		StaticLandscapeComponentManager();
		~StaticLandscapeComponentManager();
	
		/** Add a new landscape component */
		void addComponent(Entity entity,
							Vec3 world_position = Vec3(0.0f),
							Vec3 dimensions = Vec3(256.0f,128.0f,256.0f),
							uint num_bricks_x = 1,
							uint num_bricks_y = 1,
							uint num_bricks_z = 1,
							uint brick_resX = 32,
							uint brick_resY = 16,
							uint brick_rexZ = 32);
	
		/** Delete a component at the given index.
		 *	Caution: Deleting elements invalidates indices that were polled before the call to this function!
		 */
		void deleteComponent(uint index);
	
		uint getIndex(Entity e);

		std::pair<bool, uint> getIndex(uint entity_id);

		Entity getEntity(uint index);

		Entity addFeatureCurve(bool is_surface_seed = true, Vec3 position = Vec3(0.0), Quat orientation = Quat());

		Entity addFeatureMesh(const std::string& mesh_path, Vec3 position = Vec3(0.0), Quat orientation = Quat());
		
		Entity importHeightmap(const std::string& heightmap_path, Vec3 position = Vec3(0.0), Quat orientation = Quat());

		void updateBrickFeatureCurves(uint index);

		void updateBricks(uint index);

		void updateAll();

		void updateCorrespondingLandscape(Entity modified_feature_curve);

		bool isReady(uint index);

		void setReady(uint index);

		std::vector<Entity>& getFeatureCurveList(uint index);

		std::vector<Entity>& getBrickList(uint index);

		Vec3 getDimension(Entity e);

		std::vector<Entity> getListofLandscapeEntities();

		std::vector<Entity> getListofBrickEntities();

		std::vector<Entity> getListofFeatureCurveEntities();

		std::vector<Entity> getListofFeatureMeshEntities();

		std::vector<Entity> getListofHeightmapEntities();

		void exportLandscapeMesh(Entity landscape_entity, std::string filepath);
	};

	/**
	 * Future work.
	 */
	class DynamicLandscapeComponentManager
	{
	public:
		DynamicLandscapeComponentManager();
		~DynamicLandscapeComponentManager();

	private:

	};

}

#endif