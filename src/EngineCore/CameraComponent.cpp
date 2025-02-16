#include "CameraComponent.hpp"

#include "EntityManager.hpp"
namespace EngineCore
{
    namespace Graphics
    {
        CameraComponentManager::CameraComponentManager()
            : m_active_camera(EntityManager::invalidEntity())
        {
        }

        size_t CameraComponentManager::addComponent(
            Entity entity,
            float near_cp,
            float far_cp,
            float fovy,
            float aspect_ratio,
            float exposure)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    near_cp,
                    far_cp,
                    fovy,
                    aspect_ratio,
                    exposure
                }
            );

            addIndex(entity.id(), index);

            updateProjectionMatrix(index);

            return index;
        }

        void CameraComponentManager::setActiveCamera(Entity entity)
        {
            auto query = getIndex(entity);

            if (!query.empty())
                m_active_camera = entity;
        }

        Entity CameraComponentManager::getActiveCamera() const
        {
            return m_active_camera;
        }

        Entity CameraComponentManager::getEntity(size_t index) const
        {
            auto indices = data_.getIndices(index);

            return data_(indices.first, indices.second).entity;
        }

        void CameraComponentManager::setCameraAttributes(size_t index, float near_cp, float far_cp, float fovy, float aspect_ratio, float exposure)
        {
            auto indices = data_.getIndices(index);
            
            data_(indices.first, indices.second).near_cp = near_cp;
            data_(indices.first, indices.second).far_cp = far_cp;
            data_(indices.first, indices.second).fovy = fovy;
            data_(indices.first, indices.second).aspect_ratio = aspect_ratio;
            data_(indices.first, indices.second).exposure = exposure;
            
            updateProjectionMatrix(index);
        }

        void CameraComponentManager::updateProjectionMatrix(size_t index)
        {
            auto indices = data_.getIndices(index);

            float near_cp = data_(indices.first, indices.second).near_cp;
            float far_cp = data_(indices.first, indices.second).far_cp;
            float fovy = data_(indices.first, indices.second).fovy;
            float aspect_ratio = data_(indices.first, indices.second).aspect_ratio;
            data_(indices.first, indices.second).projection_matrix = glm::perspective(fovy, aspect_ratio, near_cp, far_cp);

            //    Mat4x4 projection_matrix;// = m_data.projection_matrix[index];
            //    float f = 1.0f / std::tan(fovy / 2.0f);
            //    float nf = 1.0f / (near_cp - far_cp);
            //    projection_matrix[0][0] = f / aspect_ratio;
            //    projection_matrix[0][1] = 0.0f;
            //    projection_matrix[0][2] = 0.0f;
            //    projection_matrix[0][3] = 0.0f;
            //    projection_matrix[1][0] = 0.0f;
            //    projection_matrix[1][1] = f;
            //    projection_matrix[1][2] = 0.0f;
            //    projection_matrix[1][3] = 0.0f;
            //    projection_matrix[2][0] = 0.0f;
            //    projection_matrix[2][1] = 0.0f;
            //    projection_matrix[2][2] = (far_cp + near_cp) * nf;
            //    projection_matrix[2][3] = -1.0f;
            //    projection_matrix[3][0] = 0.0f;
            //    projection_matrix[3][1] = 0.0f;
            //    projection_matrix[3][2] = (2.0f * far_cp * near_cp) * nf;
            //    projection_matrix[3][3] = 0.0f;
            //    
            //    m_data.projection_matrix[index] = projection_matrix;
        }

        void CameraComponentManager::setNear(size_t index, float near_cp)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).near_cp = near_cp;
        }

        void CameraComponentManager::setFar(size_t index, float far_cp)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).far_cp = far_cp;
        }

        Mat4x4 CameraComponentManager::getProjectionMatrix(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).projection_matrix;
        }

        float CameraComponentManager::getFovy(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).fovy;
        }

        void CameraComponentManager::setFovy(size_t index, float fovy)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).fovy = fovy;
        }

        float CameraComponentManager::getAspectRatio(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).aspect_ratio;
        }

        void CameraComponentManager::setAspectRatio(size_t index, float aspect_ratio)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).aspect_ratio = aspect_ratio;
        }

        float CameraComponentManager::getExposure(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).exposure;
        }

        void CameraComponentManager::setExposure(size_t index, float exposure)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).exposure = exposure;
        }

    }
}