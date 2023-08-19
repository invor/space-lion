#ifndef OceanComponent_hpp
#define OceanComponent_hpp

class Texture2D;
class Mesh;
class GLSLProgram;

struct Frame;

#include "BaseSingleInstanceComponentManager.hpp"
#include "BaseResourceManager.hpp"
#include "EntityManager.hpp"

// std includes
#include <unordered_map>
#include <random>

class OceanComponentManager : public EngineCore::BaseSingleInstanceComponentManager
{
public:

    enum class TextureSemantic {
        GAUSSIAN_NOISE,
        TILDE_H0_K,
        TILDE_H0_MINUS_K,
        SPECTRUM_X_DISPLACEMENT,
        SPECTRUM_Y_DISPLACEMENT,
        SPECTRUM_Z_DISPLACEMENT,
        TWIDDLE,
        IFFT_X_A,
        IFFT_X_B,
        IFFT_Y_A,
        IFFT_Y_B,
        IFFT_Z_A,
        IFFT_Z_B,
        DISPLACEMENT,
        NORMAL
    };

private:

    struct Data
    {
        Data(Entity e, float ocean_wave_height, float ocean_patch_size, uint grid_size)
            : entity(e),
            ocean_wave_height(ocean_wave_height),
            ocean_patch_size(ocean_patch_size),
            grid_size(grid_size),
            simulation_time(0.0f),
            gaussian_noise(),
            tilde_h0_of_k(),
            tilde_h0_of_minus_k(),
            spectrum_x_displacement(),
            spectrum_y_displacement(),
            spectrum_z_displacement(),
            twiddle(),
            ifft_x_a(),
            ifft_x_b(),
            ifft_y_a(),
            ifft_y_b(),
            ifft_z_a(),
            ifft_z_b(),
            displacement(),
            normal()
        {}

        //    uint used;                        ///< number of components currently in use
        //    uint allocated;                    ///< number of components that the allocated memery can hold
        //    void* buffer;                    ///< raw data pointer

        Entity entity;                    ///< entity owning that owns the component
        float ocean_wave_height;
        float ocean_patch_size;
        uint grid_size;
        float simulation_time;

        EngineCore::Graphics::ResourceID gaussian_noise;
        EngineCore::Graphics::ResourceID tilde_h0_of_k;
        EngineCore::Graphics::ResourceID tilde_h0_of_minus_k;
        EngineCore::Graphics::ResourceID spectrum_x_displacement;
        EngineCore::Graphics::ResourceID spectrum_y_displacement;
        EngineCore::Graphics::ResourceID spectrum_z_displacement;

        EngineCore::Graphics::ResourceID twiddle;
        EngineCore::Graphics::ResourceID ifft_x_a;
        EngineCore::Graphics::ResourceID ifft_x_b;
        EngineCore::Graphics::ResourceID ifft_y_a;
        EngineCore::Graphics::ResourceID ifft_y_b;
        EngineCore::Graphics::ResourceID ifft_z_a;
        EngineCore::Graphics::ResourceID ifft_z_b;

        EngineCore::Graphics::ResourceID displacement;
        EngineCore::Graphics::ResourceID normal;    
    };

    std::vector<Data> m_data;
    mutable std::shared_mutex m_data_access_mutex;
    mutable std::shared_mutex m_ocean_mutex;

    EngineCore::Graphics::ResourceID m_ocean_surface_mesh;
    EngineCore::Graphics::ResourceID m_ocean_fresnel_lut;


public:
    OceanComponentManager() = default;
    ~OceanComponentManager() = default;

    void addComponent(Entity entity, float wave_height, float patch_size, uint grid_size)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

        uint idx = static_cast<int>(m_data.size());

        m_data.push_back(Data(entity, wave_height, patch_size, grid_size));
    }

    size_t getComponentCount() const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data.size();
    }

    void setSurfaceMesh(EngineCore::Graphics::ResourceID rsrc_id)
    {
        std::unique_lock<std::shared_mutex> lock(m_ocean_mutex);
        m_ocean_surface_mesh = rsrc_id;
    }

    EngineCore::Graphics::ResourceID getSurfaceMesh() const
    {
        std::shared_lock<std::shared_mutex> lock(m_ocean_mutex);
        return m_ocean_surface_mesh;
    }

    void setFresnelLUT(EngineCore::Graphics::ResourceID rsrc_id)
    {
        std::unique_lock<std::shared_mutex> lock(m_ocean_mutex);
        m_ocean_fresnel_lut = rsrc_id;
    }

    EngineCore::Graphics::ResourceID getFresnelLUT() const
    {
        std::shared_lock<std::shared_mutex> lock(m_ocean_mutex);
        return m_ocean_fresnel_lut;
    }

    unsigned int getGridSize(size_t index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data[index].grid_size;
    }

    float getPatchSize(size_t index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data[index].ocean_patch_size;
    }

    float getWaveHeight(size_t index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data[index].ocean_wave_height;
    }

    float getSimulationTime(size_t index) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
        return m_data[index].simulation_time;
    }

    void setSimulationTime(size_t index, float time) 
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        m_data[index].simulation_time = time;
    }

    EngineCore::Graphics::ResourceID getTextureResource(size_t index, TextureSemantic semantic) const
    {
        std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);

        switch (semantic)
        {
        case OceanComponentManager::TextureSemantic::GAUSSIAN_NOISE:
            return m_data[index].gaussian_noise;
            break;
        case OceanComponentManager::TextureSemantic::TILDE_H0_K:
            return m_data[index].tilde_h0_of_k;
            break;
        case OceanComponentManager::TextureSemantic::TILDE_H0_MINUS_K:
            return m_data[index].tilde_h0_of_minus_k;
            break;
        case OceanComponentManager::TextureSemantic::SPECTRUM_X_DISPLACEMENT:
            return m_data[index].spectrum_x_displacement;
            break;
        case OceanComponentManager::TextureSemantic::SPECTRUM_Y_DISPLACEMENT:
            return m_data[index].spectrum_y_displacement;
            break;
        case OceanComponentManager::TextureSemantic::SPECTRUM_Z_DISPLACEMENT:
            return m_data[index].spectrum_z_displacement;
            break;
        case OceanComponentManager::TextureSemantic::TWIDDLE:
            return m_data[index].twiddle;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_X_A:
            return m_data[index].ifft_x_a;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_X_B:
            return m_data[index].ifft_x_b;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_Y_A:
            return m_data[index].ifft_y_a;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_Y_B:
            return m_data[index].ifft_y_b;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_Z_A:
            return m_data[index].ifft_z_a;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_Z_B:
            return m_data[index].ifft_z_b;
            break;
        case OceanComponentManager::TextureSemantic::DISPLACEMENT:
            return m_data[index].displacement;
            break;
        case OceanComponentManager::TextureSemantic::NORMAL:
            return m_data[index].normal;
            break;
        default:
            break;
        }
    }

    void setTextureResource(size_t index, TextureSemantic semantic, EngineCore::Graphics::ResourceID rsrc_id)
    {
        std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

        switch (semantic)
        {
        case OceanComponentManager::TextureSemantic::GAUSSIAN_NOISE:
            m_data[index].gaussian_noise = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::TILDE_H0_K:
            m_data[index].tilde_h0_of_k = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::TILDE_H0_MINUS_K:
            m_data[index].tilde_h0_of_minus_k = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::SPECTRUM_X_DISPLACEMENT:
            m_data[index].spectrum_x_displacement = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::SPECTRUM_Y_DISPLACEMENT:
            m_data[index].spectrum_y_displacement = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::SPECTRUM_Z_DISPLACEMENT:
            m_data[index].spectrum_z_displacement = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::TWIDDLE:
            m_data[index].twiddle = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_X_A:
            m_data[index].ifft_x_a = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_X_B:
            m_data[index].ifft_x_b = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_Y_A:
            m_data[index].ifft_y_a = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_Y_B:
            m_data[index].ifft_y_b = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_Z_A:
            m_data[index].ifft_z_a = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::IFFT_Z_B:
            m_data[index].ifft_z_b = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::DISPLACEMENT:
            m_data[index].displacement = rsrc_id;
            break;
        case OceanComponentManager::TextureSemantic::NORMAL:
            m_data[index].normal = rsrc_id;
            break;
        default:
            break;
        }
    }
};

#endif // !OceanComponent_hpp