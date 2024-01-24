#include "TransformComponentManager.hpp"

namespace EngineCore
{
    namespace Common
    {
        TransformComponentManager::TransformComponentManager()
            : BaseSingleInstanceComponentManager()
        {}

        TransformComponentManager::~TransformComponentManager()
        {}

        size_t TransformComponentManager::addComponent(Entity entity, Vec3 position, Quat orientation, Vec3 scale)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    Mat4x4(0.0f),
                    position,
                    orientation,
                    scale,
                    0,
                    0,
                    0
                }
            );

            addIndex(entity.id(), index);

            auto [page_idx, idx_in_page] = data_.getIndices(index);

            {
                auto lock = data_.accquirePageLock(page_idx);

                data_(page_idx, idx_in_page).parent = index;
                data_(page_idx, idx_in_page).first_child = index;
                data_(page_idx, idx_in_page).next_sibling = index;
            }

            transform(index);

            return index;
        }

        void TransformComponentManager::deleteComonent(Entity entity)
        {
            auto query = getIndex(entity);

            data_.deleteComponent(query);
        }

        size_t TransformComponentManager::getComponentCount() const
        {
            return data_.getComponentCount();
        }

        void TransformComponentManager::translate(Entity entity, Vec3 translation)
        {
            auto query = getIndex(entity);

            translate(query, translation);
        }

        void TransformComponentManager::translate(size_t index, Vec3 translation)
        {
            {
                auto [page_idx, idx_in_page] = data_.getIndices(index);

                auto lock = data_.accquirePageLock(page_idx);

                data_(page_idx, idx_in_page).position += translation;
            }

            transform(index);
        }

        void TransformComponentManager::rotate(size_t index, Quat rotation)
        {
            {
                auto [page_idx, idx_in_page] = data_.getIndices(index);

                auto lock = data_.accquirePageLock(page_idx);

                data_(page_idx, idx_in_page).orientation = glm::normalize(rotation * data_(page_idx, idx_in_page).orientation);
            }

            transform(index);
        }

        void TransformComponentManager::rotateLocal(size_t index, Quat rotation)
        {
            {
                auto [page_idx, idx_in_page] = data_.getIndices(index);

                auto lock = data_.accquirePageLock(page_idx);

                data_(page_idx, idx_in_page).orientation = glm::normalize(data_(page_idx, idx_in_page).orientation * rotation);
            }

            transform(index);
        }

        void TransformComponentManager::scale(size_t index, Vec3 scale_factors)
        {
            {
                auto [page_idx, idx_in_page] = data_.getIndices(index);

                auto lock = data_.accquirePageLock(page_idx);

                data_(page_idx, idx_in_page).scale *= scale_factors;
            }

            transform(index);
        }

        void TransformComponentManager::transform(size_t index)
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);

            auto lock = data_.accquirePageLock(page_idx);

            Mat4x4 xform = glm::toMat4(data_(page_idx, idx_in_page).orientation);
            xform[3] = Vec4(data_(page_idx, idx_in_page).position, 1.0);
            xform[0] *= data_(page_idx, idx_in_page).scale.x;
            xform[1] *= data_(page_idx, idx_in_page).scale.y;
            xform[2] *= data_(page_idx, idx_in_page).scale.z;

            if (data_(page_idx, idx_in_page).parent != index) {
                auto [parent_page_idx, parent_idx_in_page] = data_.getIndices(data_(page_idx, idx_in_page).parent);
                data_(page_idx, idx_in_page).world_transform = data_(parent_page_idx, parent_idx_in_page).world_transform * xform;
            }
            else {
                data_(page_idx, idx_in_page).world_transform = xform;
            }

            // update transforms of all children
            size_t child_idx = data_(page_idx, idx_in_page).first_child;
            if (child_idx != index)
            {
                lock.unlock();
                transform(child_idx);
                lock.lock();

                auto [child_page_idx, child_idx_in_page] = data_.getIndices(child_idx);
                size_t sibling_idx = data_(child_page_idx, child_idx_in_page).next_sibling;

                while (sibling_idx != child_idx)
                {
                    auto [sibling_page_idx, sibing_idx_in_page] = data_.getIndices(sibling_idx);

                    child_idx = sibling_idx;
                    child_page_idx = sibling_page_idx;
                    child_idx_in_page = sibing_idx_in_page;

                    lock.unlock();
                    transform(child_idx);
                    lock.lock();

                    sibling_idx = data_(child_page_idx, child_idx_in_page).next_sibling;
                }
            }
        }

        void TransformComponentManager::setPosition(Entity entity, Vec3 position)
        {
            auto query = getIndex(entity);

            setPosition(query, position);
        }

        void TransformComponentManager::setPosition(size_t index, Vec3 position)
        {
            {
                auto [page_idx, idx_in_page] = data_.getIndices(index);

                auto lock = data_.accquirePageLock(page_idx);

                data_(page_idx, idx_in_page).position = position;
            }

            transform(index);
        }

        void TransformComponentManager::setOrientation(size_t index, Quat orientation)
        {
            {
                auto [page_idx, idx_in_page] = data_.getIndices(index);

                auto lock = data_.accquirePageLock(page_idx);

                data_(page_idx, idx_in_page).orientation = orientation;
            }

            transform(index);
        }

        void TransformComponentManager::setScale(size_t index, Vec3 scale)
        {
            {
                auto [page_idx, idx_in_page] = data_.getIndices(index);

                auto lock = data_.accquirePageLock(page_idx);

                data_(page_idx, idx_in_page).scale = scale;
            }

            transform(index);
        }

        void TransformComponentManager::setParent(size_t index, Entity parent)
        {
            {
                auto [page_idx, idx_in_page] = data_.getIndices(index);

                auto lock = data_.accquirePageLock(page_idx);

                auto query = getIndex(parent);

                size_t parent_idx = query;
                data_(page_idx, idx_in_page).parent = parent_idx;

                auto [parent_page_idx, parent_idx_in_page] = data_.getIndices(parent_idx);

                if (data_(parent_page_idx, parent_idx_in_page).first_child == parent_idx)
                {
                    data_(parent_page_idx, parent_idx_in_page).first_child = index;
                }
                else
                {
                    size_t child_idx = data_(parent_page_idx, parent_idx_in_page).first_child;
                    auto [child_page_idx, child_idx_in_page] = data_.getIndices(child_idx);

                    while (data_(child_page_idx, child_idx_in_page).next_sibling != child_idx)
                    {
                        child_idx = data_(child_page_idx, child_idx_in_page).next_sibling;
                        std::tie(child_page_idx, child_idx_in_page) = data_.getIndices(child_idx);
                    }

                    data_(child_page_idx, child_idx_in_page).next_sibling = index;
                }
            }

            transform(index);

        }

        Vec3 const& TransformComponentManager::getPosition(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);

            auto lock = data_.accquirePageLock(page_idx);

            return data_(page_idx, idx_in_page).position;
        }

        Vec3 TransformComponentManager::getWorldPosition(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);

            auto lock = data_.accquirePageLock(page_idx);

            return Vec3(data_(page_idx, idx_in_page).world_transform * Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }

        Vec3 TransformComponentManager::getWorldPosition(Entity e) const
        {
            Vec3 retval;

            auto query = getIndex(e);

            retval = getWorldPosition(query);

            return retval;
        }

        Quat const& TransformComponentManager::getOrientation(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);

            auto lock = data_.accquirePageLock(page_idx);

            return data_(page_idx, idx_in_page).orientation;
        }

        Mat4x4 const& TransformComponentManager::getWorldTransformation(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);

            auto lock = data_.accquirePageLock(page_idx);

            return data_(page_idx, idx_in_page).world_transform;
        }

        std::vector<Entity> TransformComponentManager::getChildren(Entity entity) const
        {
            std::vector<Entity> retval;

            auto query = getIndex(entity);

            retval = getChildren(query);

            return retval;
        }

        std::vector<Entity> TransformComponentManager::getChildren(size_t index) const
        {
            std::vector<Entity> retval;

            auto [page_idx, idx_in_page] = data_.getIndices(index);

            auto lock = data_.accquirePageLock(page_idx);

            size_t child_idx = data_(page_idx, idx_in_page).first_child;
            if (child_idx != index)
            {
                auto [child_page_idx, child_idx_in_page] = data_.getIndices(child_idx);

                retval.push_back(data_(child_page_idx, child_idx_in_page).entity);

                size_t sibling_idx = data_(child_page_idx, child_idx_in_page).next_sibling;

                while (sibling_idx != child_idx)
                {
                    auto [sibling_page_idx, sibing_idx_in_page] = data_.getIndices(sibling_idx);

                    child_idx = sibling_idx;
                    child_page_idx = sibling_page_idx;
                    child_idx_in_page = sibing_idx_in_page;

                    retval.push_back(data_(child_page_idx, child_idx_in_page).entity);

                    sibling_idx = data_(child_page_idx, child_idx_in_page).next_sibling;
                }
            }

            return retval;
        }

        Entity TransformComponentManager::getParent(size_t index) const
        {
            Entity retval;

            auto [page_idx, idx_in_page] = data_.getIndices(index);

            auto lock = data_.accquirePageLock(page_idx);

            size_t parent_idx = data_(page_idx, idx_in_page).parent;

            if (parent_idx != index)
            {
                auto [parent_page_idx, parent_idx_in_page] = data_.getIndices(parent_idx);
                retval = data_(parent_page_idx, parent_idx_in_page).entity;
            }

            return retval;
        }
    }
}