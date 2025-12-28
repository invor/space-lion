#ifndef CameraComponent_hpp
#define CameraComponent_hpp

// forward declartion of Entity
struct Entity;
// space-lion includes
#include "BaseMultiInstanceComponentManager2.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        struct CameraComponentData {
            Entity entity;            ///< entity owning the component
            float  near_cp;           ///< near clipping plane
            float  far_cp;            ///< far clipping plane
            float  fovy;              ///< camera vertical field of view in radian
            float  aspect_ratio;      ///< camera aspect ratio
            float  exposure;          ///< camera exposure, a simplification of iso-value, aperature number and exposure time
            Mat4x4 projection_matrix; ///< camera projection matrix
        };

        class CameraComponentManager : public BaseMultiInstanceComponentManager2<CameraComponentData, 1000, 1000>
        {
        private:
            // TODO move this elsewhere?
            Entity m_active_camera;

        public:
            CameraComponentManager();
            ~CameraComponentManager() = default;

            size_t addComponent(Entity entity,
                float near_cp = 0.001f,
                float far_cp = 1000.0f,
                float fovy = 0.5236f,
                float aspect_ratio = 16.0 / 9.0f,
                float exposure = 0.000036f); // default value for mapping avg luminance of ~5000cd/m^2 to 0.18 intensity

            void setActiveCamera(Entity entity);

            Entity getActiveCamera() const;

            Entity getEntity(size_t index) const;

            void setCameraAttributes(
                size_t index,
                float near_cp,
                float far_cp,
                float fovy = 0.5236f,
                float aspect_ratio = 16.0 / 9.0f,
                float exposure = 0.000045f);

            void updateProjectionMatrix(size_t index);

            Mat4x4 getProjectionMatrix(size_t index) const;

            void setNear(size_t index, float near_cp);

            void setFar(size_t index, float far_cp);

            float getFovy(size_t index) const;

            void setFovy(size_t index, float fovy);

            float getAspectRatio(size_t index) const;

            void setAspectRatio(size_t index, float aspect_ratio);

            float getExposure(size_t index) const;

            void setExposure(size_t index, float exposure);
        };

    }
}

#endif