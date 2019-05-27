#ifndef EntityManager_h
#define EntityManager_h

#include <deque>
#include <limits>
#include <shared_mutex>
#include <vector>

#include "types.hpp"

const size_t MAX_ENTITY_ID = std::numeric_limits<uint>::max() - 1;

/**
 * Most basic entity representation using only a unique id.
 * The id of an entity can't be changed after construction
 * and only the EntityManager is allowed to construct new enties.
 */
struct Entity
{
    inline uint id() const { return m_id; }

    inline bool operator==(const Entity& rhs) { return m_id == rhs.id(); }
    inline bool operator!=(const Entity& rhs) { return m_id != rhs.id(); }

    friend class EntityManager;
private:
    constexpr Entity() : m_id(std::numeric_limits<uint>::max()) {}

    uint m_id;
};

class EntityManager
{
    /**
     * Keeps track of created entities.
     */
    std::vector<bool> m_is_alive;

    /**
     * Store ids (indices) of deleted entities in order to reuse
     * reuse them once the number of created entities exceeds
     * the value range of a uint.
     * Ids are reused in order of deletion, i.e. the time between
     * deletion and reuse is maximized.
     */
    std::deque<uint> m_free_indices;

    /** Mutex to protect queue operations. */
    mutable std::shared_mutex m_mutex;

public:
    Entity create();

    void destroy(Entity entity);

    size_t getEntityCount() const;

    std::pair<bool, Entity> getEntity(uint index) const;

    bool alive(Entity entity) const;

    /**
     * Get the invalid entity, i.e. the entity with id = max_uint.
     * Useful for functions that return an entity if something goes wrong.
     */
    Entity invalidEntity() const { return Entity(); }
};

#endif