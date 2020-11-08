#ifndef AtmosphereComponent_hpp
#define AtmosphereComponent_hpp

#include <unordered_map>

struct Entity;
class Material;
class Mesh;
class GLSLProgram;

#include "types.hpp"

/**
 * Manages atmosphere components and functions as a decentralized module of the rendering pipeline
 */
class AtmosphereComponentManager
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
        float* max_altitude;			///< maximum altitude of the atmosphere
        Material** material;
    };

    Data m_data;

    std::unordered_map<uint, uint> m_index_map;

    /** Pointer to sphere mesh used as atmoshpere proxy geometry. */
    Mesh* m_atmosphere_proxySphere;
    /** Pointer to GLSL program used to draw atmospheres. */
    GLSLProgram* m_atmosphere_prgm;

    /** Access raw data. */
    Data const* const getData() const;


    /*****************************************************************
     * Buffer, compute and draw methods (only call from GPU thread!)
     ****************************************************************/
    void createGpuResources();

    void createMaterial(uint index);
    void computeTransmittance(uint index);
    void computeInscatterSingle(uint index);
    void computeIrradianceSingle(uint index);
    void draw();
    void drawDebugInterface();

public:
    AtmosphereComponentManager(uint size);
    ~AtmosphereComponentManager();

    void registerRenderingPipelineTasks();

    void reallocate(uint size);

    void addComponent(Entity entity,
        Vec3 beta_r,
        Vec3 beta_m,
        float h_r,
        float h_m,
        float min_altitude,
        float max_altitude);

    uint getIndex(Entity entity) const;

    Material const* const getMaterial(Entity entity) const;

    Material const* const getMaterial(uint index) const;
};

#endif