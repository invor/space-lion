#ifndef FeatureCurveComponent_hpp
#define FeatureCurveComponent_hpp

#include <unordered_map>
#include <vector>

#include "BaseSingleInstanceComponentManager.hpp"
#include "BaseResourceManager.hpp"
#include "EntityManager.hpp"
#include "GeometryBakery.hpp"

namespace EngineCore {

    class WorldState;

    namespace Graphics {
        namespace Landscape
        {
            template<typename ResourceManagerType>
            class FeatureCurveComponentManager : public BaseSingleInstanceComponentManager
            {
            private:

                struct ConstraintPoint
                {
                    ConstraintPoint(Entity cp_entity,
                        Vec3 lh_bitangent,
                        Vec3 rh_bitangent,
                        float curve_position,
                        Entity lh_bitangent_entity,
                        Entity rh_bitangent_entity,
                        float noise_amplitude = 1.0f,
                        float noise_roughness = 1.0f,
                        int lh_material = 0,
                        int rh_material = 0)
                        : m_entity(cp_entity),
                        m_lefthand_bitangent(lh_bitangent),
                        m_righthand_bitangent(rh_bitangent),
                        m_noise_amplitude(1.0f),
                        m_noise_roughness(1.0f),
                        m_lefthand_material(0),
                        m_righthand_material(0),
                        m_curve_position(curve_position),
                        m_lh_bitangent_entity(lh_bitangent_entity),
                        m_rh_bitangent_entity(rh_bitangent_entity) {}

                    Entity	m_entity;				///< entity that constraint point is associated with

                    Vec3	m_tangent;				///< curve tangent at constraint point location (note: not guaranteed to be up-to-date)

                    Vec3	m_lefthand_bitangent;	///< lefthand bitangent vector constraint 
                    Vec3	m_righthand_bitangent;	///< righthand bitangent vector constraint

                    float	m_noise_amplitude;		///< noise amplitude constraint
                    float	m_noise_roughness;		///< noise roughness constraint

                    int		m_lefthand_material;	///< lefthand material index
                    int		m_righthand_material;	///< righthand material index

                    float	m_curve_position;		///< constraint point position on the curve (in normalized curve parameter space)

                    Entity	m_lh_bitangent_entity;	///< entity that lefthand bitangent vector is associated with
                    Entity	m_rh_bitangent_entity;	///< entity that righthand bitangent vector is associated with
                };

                struct Data
                {
                    Data(Entity e, bool is_surface_seed, ResourceID proxy_mesh)
                        : m_entity(e),
                        m_is_surface_seed(is_surface_seed),
                        m_ribbon_width(1.0f),
                        m_proxy_mesh(proxy_mesh) {}

                    Entity							m_entity;
                    bool							m_is_surface_seed;
                    float							m_ribbon_width;
                    std::vector<ConstraintPoint>	m_constraint_points;
                    ResourceID						m_proxy_mesh;
                };

                std::vector<Data> m_data;

                std::shared_mutex m_dataAccess_mutex;

                /** Mapping from curve entity to curve index */
                std::unordered_map<uint, uint> m_curve_eID_to_idx;
                /** Mapping from constrain point entity to index of curve that the CP belongs to */
                std::unordered_map<uint, uint> m_cp_eID_to_curve_idx;
                /** Mapping from bitangent entity id to constraint point entity id */
                std::unordered_map<uint, uint> m_bitangent_eID_to_cp_eID;

                //TODO: decouple from world state
                WorldState& m_world;

                //TODO: loosly couple only for functions needing access to resource manager?
                ResourceManagerType& m_resource_mngr;

                std::pair<Vec3, Vec3> interpolateCurveBitangents(uint curve_idx, float u);

                std::tuple<float, float, int, int> interpolateCurveSurfaceProperties(uint curve_idx, float u);

                std::tuple<VertexData, IndexData, GenericVertexLayout> computeCurveProxyMesh(uint curve_idx);

                std::tuple<VertexData, IndexData, GenericVertexLayout> computeBitangentProxyMesh(Entity bitangent);

            public:

                FeatureCurveComponentManager(WorldState& world_state, ResourceManagerType& resource_manager);
                ~FeatureCurveComponentManager() = default;

                ///////////////////
                // Feature Curve functions
                ///////////////////

                void addComponent(Entity entity, bool is_surface_seed);

                void deleteComponent(uint idx);

                std::pair<bool, uint> getIndex(Entity entity) const;

                /** Set the ribbon width of a single Feature Curve to a new value */
                void setRibbonWidth(uint curve_idx, float ribbon_width);

                /** Set the ribbon width of all feature curves to a new value */
                void setRibbonWidth(float ribbon_width);

                /** Recomputes constraint point positions and updates mesh data */
                void updateCurve(uint curve_idx);

                bool isSurfaceSeed(uint curve_idx) const;

                ResourceID getProxyMesh(uint curve_idx) const;

                ///////////////////
                // Access functions
                ///////////////////

                Entity getCurveFromConstraintPoint(Entity cp_entity) const;

                Entity getConstraintPointFromBitangent(Entity bitangent_entity) const;

                //////////////////
                // Constraint Point functions
                //////////////////

                Entity addConstraintPoint(uint curve_idx, float curve_position);

                Entity addConstraintPoint(Entity curve_entity, float curve_position);

                Entity addConstraintPoint(uint curve_idx, float curve_position, Vec3 lefthand_bitangent, Vec3 righthand_bitangent);

                void setConstraintPointCurvePosition(Entity cp_entity, float curve_position);

                void setConstraintPointBitangent(Entity bitangent_entity, Vec3 new_bitangent);

                void setConstraintPointNoiseAmplitude(Entity cp_entity, float amplitude);

                void setConstraintPointNoiseRoughness(Entity cp_entity, float roughness);

                void setConstraintPointMaterials(Entity cp_entity, int lefthand_material, int righthand_material);

                std::pair<float, float> getConstraintPointNoise(Entity cp_entity);

                std::pair<int, int> getConstraintPointMaterialIDs(Entity cp_entity);

                float getConstraintPointCurvePosition(Entity cp_entity);

                Vec3 getConstraintPointTangent(Entity cp_entity);
            };
        }
    }
}

#include "LandscapeFeatureCurveComponent.inl"

#endif // !FeatureCurveComponent_hpp
