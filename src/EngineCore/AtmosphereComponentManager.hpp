#ifndef AtmosphereComponent_hpp
#define AtmosphereComponent_hpp

#include <unordered_map>

#include "BaseSingleInstanceComponentManager.hpp"
#include "BaseResourceManager.hpp"
#include "EntityManager.hpp"
#include "GeometryBakery.hpp"
#include "types.hpp"

namespace EngineCore{
namespace Graphics {

    /**
     * Manages atmosphere components and functions as a decentralized module of the rendering pipeline
     */
    template<typename ResourceManagerType>
    class AtmosphereComponentManager : public BaseSingleInstanceComponentManager
    {
    private:
        struct Data
        {
            uint used;						///< number of components currently in use
            uint allocated;					///< number of components that the allocated memery can hold
            uint8_t* buffer;				///< raw data pointer

            Entity* entity;					///< entity owning the component

            Vec3* beta_r;					///< extinction coefficient for Rayleigh scattering
            Vec3* beta_m;					///< extinction coefficient for Mie scattering
            float* h_r;						///< ?
            float* h_m;						///< ?
            float* min_altitude;			///< minimum altitude of the atmosphere
            float* max_altitude;			///< maximum altitude of the atmosphere<
            ResourceID* transmittance_lut;
            ResourceID* mie_inscatter_lut;
            ResourceID* rayleigh_inscatter_lut;
            ResourceID* irradiance_lut;
        };

        Data m_data;
        mutable std::shared_mutex m_data_access_mutex;

        ResourceManagerType& m_rsrc_mngr;

        ResourceID m_proxy_mesh;

        /*****************************************************************
         * Buffer, compute and draw methods (only call from GPU thread!)
         ****************************************************************/
        void createGpuResources();

        void computeTransmittance(uint index);
        void computeInscatterSingle(uint index);
        void computeIrradianceSingle(uint index);
        void draw();
        void drawDebugInterface();

    public:
        AtmosphereComponentManager(uint size, ResourceManagerType& rsrc_mngr);
        ~AtmosphereComponentManager();

        void reallocate(uint size);

        void setProxyMesh(ResourceID proxy_mesh);

        ResourceID getProxyMesh();

        void addComponent(Entity entity,
            Vec3 beta_r,
            Vec3 beta_m,
            float h_r,
            float h_m,
            float min_altitude,
            float max_altitude);

        uint getComponentCount() { return m_data.used; }

        void setBetaR(uint index, Vec3 const& beta_r);

        void setBetaM(uint index, Vec3 const& beta_m);

        void setHR(uint index, float h_r);

        void setHM(uint index, float h_m);

        void setMinAltitude(uint index, float min_altitude);

        void setMaxAltitude(uint index, float max_altitude);

        void setTransmittanceLUT(uint index, ResourceID transmittance_lut);

        void setMieInscatterLUT(uint index, ResourceID mie_inscatter_lut);

        void setRayleighInscatterLUT(uint index, ResourceID rayleigh_inscatter_lut);

        Entity getEntity(uint index) const;

        Vec3 getBetaR(uint index) const;

        Vec3 getBetaM(uint index) const;

        float getHR(uint index) const;

        float getHM(uint index) const;

        float getMinAltitude(uint index) const;

        float getMaxAltitude(uint index) const;

        ResourceID getTransmittanceLUT(uint index);

        ResourceID getMieInscatterLUT(uint index);

        ResourceID getRayleighInscatterLUT(uint index);
    };

    template<typename ResourceManagerType>
    inline AtmosphereComponentManager<ResourceManagerType>::AtmosphereComponentManager(uint size, ResourceManagerType& rsrc_mngr)
        : m_rsrc_mngr(rsrc_mngr), m_proxy_mesh(ResourceManagerType::invalidResourceID())
    {
        const uint bytes = size * (sizeof(Entity) +
            2 * sizeof(Vec3) +
            4 * sizeof(float) +
            4 * sizeof(ResourceID));

        m_data.buffer = new uint8_t[bytes];

        m_data.used = 0;
        m_data.allocated = size;

        m_data.entity = (Entity*)(m_data.buffer);
        m_data.beta_r = (Vec3*)(m_data.entity + size);
        m_data.beta_m = (Vec3*)(m_data.beta_r + size);
        m_data.h_r = (float*)(m_data.beta_m + size);
        m_data.h_m = (float*)(m_data.h_r + size);
        m_data.min_altitude = (float*)(m_data.h_m + size);
        m_data.max_altitude = (float*)(m_data.min_altitude + size);
        m_data.transmittance_lut = (ResourceID*)(m_data.max_altitude + size);
        m_data.mie_inscatter_lut = (ResourceID*)(m_data.transmittance_lut + size);
        m_data.rayleigh_inscatter_lut = (ResourceID*)(m_data.mie_inscatter_lut + size);
        m_data.irradiance_lut = (ResourceID*)(m_data.rayleigh_inscatter_lut + size);
    }

    template<typename ResourceManagerType>
    inline AtmosphereComponentManager<typename ResourceManagerType>::~AtmosphereComponentManager()
    {
        delete[] m_data.buffer;

        //TODO flag gpu resources for deletion
    }

    template<typename ResourceManagerType>
    void AtmosphereComponentManager<typename ResourceManagerType>::reallocate(uint size)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

        Data new_data;
        const uint bytes = size * (sizeof(Entity) +
            2 * sizeof(Vec3) +
            4 * sizeof(float));

        new_data.buffer = new uint8_t[bytes];

        new_data.used = m_data.used;
        new_data.allocated = size;

        new_data.entity = (Entity*)(new_data.buffer);
        new_data.beta_r = (Vec3*)(new_data.entity + size);
        new_data.beta_m = (Vec3*)(new_data.beta_r + size);
        new_data.h_r = (float*)(new_data.beta_m + size);
        new_data.h_m = (float*)(new_data.h_r + size);
        new_data.min_altitude = (float*)(new_data.h_m + size);
        new_data.max_altitude = (float*)(new_data.min_altitude + size);
        new_data.transmittance_lut = (ResourceID*)(m_data.max_altitude + size);
        new_data.mie_inscatter_lut = (ResourceID*)(m_data.transmittance_lut + size);
        new_data.rayleigh_inscatter_lut = (ResourceID*)(m_data.mie_inscatter_lut + size);
        new_data.irradiance_lut = (ResourceID*)(m_data.rayleigh_inscatter_lut + size);

        std::memcpy(new_data.entity, m_data.entity, m_data.used * sizeof(Entity));
        std::memcpy(new_data.beta_r, m_data.beta_r, m_data.used * sizeof(Vec3));
        std::memcpy(new_data.beta_m, m_data.beta_m, m_data.used * sizeof(Vec3));
        std::memcpy(new_data.h_r, m_data.h_r, m_data.used * sizeof(float));
        std::memcpy(new_data.h_m, m_data.h_m, m_data.used * sizeof(float));
        std::memcpy(new_data.min_altitude, m_data.min_altitude, m_data.used * sizeof(float));
        std::memcpy(new_data.max_altitude, m_data.max_altitude, m_data.used * sizeof(float));
        std::memcpy(new_data.transmittance_lut, m_data.transmittance_lut, m_data.used * sizeof(ResourceID*));
        std::memcpy(new_data.mie_inscatter_lut, m_data.mie_inscatter_lut, m_data.used * sizeof(ResourceID*));
        std::memcpy(new_data.rayleigh_inscatter_lut, m_data.rayleigh_inscatter_lut, m_data.used * sizeof(ResourceID*));
        std::memcpy(new_data.irradiance_lut, m_data.irradiance_lut, m_data.used * sizeof(ResourceID*));

        delete m_data.buffer;

        m_data = new_data;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setProxyMesh(ResourceID proxy_mesh)
    {
        m_proxy_mesh = proxy_mesh;
    }

    template<typename ResourceManagerType>
    inline ResourceID AtmosphereComponentManager<ResourceManagerType>::getProxyMesh()
    {
        return m_proxy_mesh;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<typename ResourceManagerType>::addComponent(
        Entity entity,
        Vec3 beta_r,
        Vec3 beta_m,
        float h_r,
        float h_m,
        float min_altitude,
        float max_altitude)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

        assert(m_data.used < m_data.allocated);

        uint index = m_data.used;
        addIndex(entity.id(), index);
        ++m_data.used;

        m_data.entity[index] = entity;
        m_data.beta_r[index] = beta_r;
        m_data.beta_m[index] = beta_m;
        m_data.h_r[index] = h_r;
        m_data.h_m[index] = h_m;
        m_data.min_altitude[index] = min_altitude;
        m_data.max_altitude[index] = max_altitude;
        m_data.transmittance_lut[index] = ResourceManagerType::invalidResourceID();
        m_data.mie_inscatter_lut[index] = ResourceManagerType::invalidResourceID();
        m_data.rayleigh_inscatter_lut[index] = ResourceManagerType::invalidResourceID();
        m_data.irradiance_lut[index] = ResourceManagerType::invalidResourceID();

        // using GL enum values as default, need to be translated to DX values by DX resource manager
        //uint32_t rgba32f_type = 0x8814;
        //uint32_t rgba_type = 0x1908;
        //uint32_t float_type = 0x1406;
        //uint32_t texture_wrap_s = 0x2802;
        //uint32_t texture_wrap_t = 0x2803;
        //uint32_t texture_wrap_r = 0x8072;
        //uint32_t clamp_to_edge = 0x812F;
        //uint32_t texture_min_filter = 0x2801;
        //uint32_t texture_mag_filter = 0x2800;
        //uint32_t linear = 0x2601;
        //
        //GenericTextureLayout transmittance_layout(rgba32f_type, 256, 64, 1, rgba_type, float_type, 1,
        //    { std::pair<GLenum,GLenum>(texture_wrap_s,clamp_to_edge),
        //    std::pair<GLenum,GLenum>(texture_wrap_t,clamp_to_edge) }, {});
        //m_data.transmittance_lut[index] = m_rsrc_mngr.createTexture2DAsync("transmittance_table_" + m_data.entity[index].id(), transmittance_layout, nullptr);
        m_data.transmittance_lut[index] = m_rsrc_mngr.invalidResourceID();
        
        //GenericTextureLayout inscatter_layout(rgba32f_type, 256, 128, 32, rgba_type, float_type, 1,
        //    { std::pair<GLenum,GLenum>(texture_wrap_s,clamp_to_edge),
        //        std::pair<GLenum,GLenum>(texture_wrap_t,clamp_to_edge),
        //        std::pair<GLenum,GLenum>(texture_wrap_r,clamp_to_edge),
        //        std::pair<GLenum,GLenum>(texture_min_filter,linear),
        //        std::pair<GLenum, GLenum>(texture_mag_filter,linear) }, {});
        //m_data.mie_inscatter_lut[index] = m_rsrc_mngr.createTexture3DAsync("mie_inscatter_table_" + m_data.entity[index].id(), inscatter_layout, nullptr);
        //m_data.rayleigh_inscatter_lut[index] = m_rsrc_mngr.createTexture3DAsync("rayleigh_inscatter_table_" + m_data.entity[index].id(), inscatter_layout, nullptr);
        m_data.rayleigh_inscatter_lut[index] = m_rsrc_mngr.invalidResourceID();

        //GenericTextureLayout irradiance_layout(rgba32f_type, 256, 64, 1, rgba_type, float_type, 1,
        //    { std::pair<GLenum,GLenum>(texture_wrap_s,clamp_to_edge),
        //        std::pair<GLenum,GLenum>(texture_wrap_t,clamp_to_edge) }, {});
        //m_data.irradiance_lut[index] = m_rsrc_mngr.createTexture2DAsync("irradience_table_" + m_data.entity[index].id(), irradiance_layout, nullptr);
        m_data.irradiance_lut[index] = m_rsrc_mngr.invalidResourceID();

        // Enqueue GPU tasks for atmosphere computation
        //  GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeTransmittance(index); });
        //  GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeInscatterSingle(index); });
        //  GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index] { this->computeIrradianceSingle(index); });
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setBetaR(uint index, Vec3 const& beta_r)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.beta_r[index] = beta_r;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setBetaM(uint index, Vec3 const& beta_m)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.beta_m[index] = beta_m;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setHR(uint index, float h_r)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.h_r[index] = h_r;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setHM(uint index, float h_m)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.m_r[index] = h_m;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setMinAltitude(uint index, float min_altitude)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.min_altitude[index] = min_altitude;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setMaxAltitude(uint index, float max_altitude)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.max_altitude[index] = max_altitude;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setTransmittanceLUT(uint index, ResourceID transmittance_lut)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.transmittance_lut[index] = transmittance_lut;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setMieInscatterLUT(uint index, ResourceID mie_inscatter_lut)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.mie_inscatter_lut[index] = mie_inscatter_lut;
    }

    template<typename ResourceManagerType>
    inline void AtmosphereComponentManager<ResourceManagerType>::setRayleighInscatterLUT(uint index, ResourceID rayleigh_inscatter_lut)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data.rayleigh_inscatter_lut[index] = rayleigh_inscatter_lut;
    }

    template<typename ResourceManagerType>
    inline Entity AtmosphereComponentManager<ResourceManagerType>::getEntity(uint index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.entity[index];
    }

    template<typename ResourceManagerType>
    inline Vec3 AtmosphereComponentManager<ResourceManagerType>::getBetaR(uint index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.beta_r[index];
    }

    template<typename ResourceManagerType>
    inline Vec3 AtmosphereComponentManager<ResourceManagerType>::getBetaM(uint index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.beta_m[index];
    }

    template<typename ResourceManagerType>
    inline float AtmosphereComponentManager<ResourceManagerType>::getHR(uint index) const
    {
        return m_data.h_r[index];
    }

    template<typename ResourceManagerType>
    inline float AtmosphereComponentManager<ResourceManagerType>::getHM(uint index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.h_m[index];
    }

    template<typename ResourceManagerType>
    inline float AtmosphereComponentManager<ResourceManagerType>::getMinAltitude(uint index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.min_altitude[index];
    }

    template<typename ResourceManagerType>
    inline float AtmosphereComponentManager<ResourceManagerType>::getMaxAltitude(uint index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.max_altitude[index];
    }

    template<typename ResourceManagerType>
    inline ResourceID AtmosphereComponentManager<ResourceManagerType>::getTransmittanceLUT(uint index)
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.transmittance_lut[index];
    }

    template<typename ResourceManagerType>
    inline ResourceID AtmosphereComponentManager<ResourceManagerType>::getMieInscatterLUT(uint index)
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.mie_inscatter_lut[index];
    }

    template<typename ResourceManagerType>
    inline ResourceID AtmosphereComponentManager<ResourceManagerType>::getRayleighInscatterLUT(uint index)
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.rayleigh_inscatter_lut[index];
    }

}
}

#endif
