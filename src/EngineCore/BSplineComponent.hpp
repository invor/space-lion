#ifndef BSplineComponent_hpp
#define BSPlineComponent_hpp

#include <unordered_map>

#include "BaseMultiInstanceComponentManager.hpp"
#include "WorldState.hpp"

namespace EngineCore {
namespace Common {

    class BSplineComponentManager : public BaseMultiInstanceComponentManager
    {
    private:
        typedef Entity ControlVertex;
    
        struct ComponentData
        {
            ComponentData(Entity e) : m_entity(e), m_degree(0) {}
    
            Entity m_entity;
    
            uint m_degree;
    
            std::vector<float> m_knotvector;
            std::vector<ControlVertex> m_control_vertices;
        };
    
        std::vector<ComponentData> m_data;
        std::shared_mutex          m_data_mutex;

        WorldState& m_world;
    
    
        /** Recursive De Boor algorithm as seen on Wikipedia */
        Vec3 recursiveDeBoor(size_t index, uint k, uint i, float x) const;
    
    public:

        BSplineComponentManager(WorldState& world);
        ~BSplineComponentManager() = default;
    
        void addComponent(Entity entity);
    
        void addComponent(Entity entity, std::vector<ControlVertex> const& control_vertices);
    
        /**
        * Insert a new control vertex to a specified location in the control vertex array of the specified spline curve
        * @param spline The Bspline curve that the control vertex is inserted in
        * @param insert_location Insert location given by a control vertex of the spline
        * @param control_vertex The control vertex that is added to the spline
        * @param insert_behind Specifies whether the new control vertex will be added before or after the given insert location
        */
        void insertControlVertex(size_t component_idx, ControlVertex insert_location, ControlVertex control_vertex, bool insert_after = true);

        /**
         * Convenience function for inserting a control vertex to a spline specified by entity
         * Note: Will insert the cv only to the first component of the entity
         */
        void insertControlVertex(Entity entity, ControlVertex insert_location, ControlVertex control_vertex, bool insert_after = true);
    
        Vec3 computeCurvePoint(size_t component_idx, float u) const;
    
        Vec3 computeCurvePoint(Entity entity, float u) const;
    
        Vec3 computeCurveTangent(size_t component_idx, float u) const;
    
        Vec3 computeCurveTangent(Entity entity, float u) const;
    
        Entity getLastControlVertex(Entity entity) const;
    };

}
}

#endif // !BSplineComponent_hpp
