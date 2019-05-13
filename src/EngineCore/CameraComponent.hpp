#ifndef CameraComponent_hpp
#define CameraComponent_hpp

// forward declartion of Entity
struct Entity;

// space-lion includes
#include "types.hpp"

// std includes
#include <unordered_map>

class CameraComponentManager
{
private:
	struct Data
	{
		uint used;		///< number of components currently in use
		uint allocated;	///< number of components that the allocated memery can hold
		uint8_t* buffer;	///< raw data pointer

		Entity* entity;				///< entity owning the component
		float* near;				///< near clipping plane
		float* far;					///< far clipping plane
		float* fovy;				///< camera vertical field of view in radian
		float* aspect_ratio;		///< camera aspect ratio
		float* exposure;			///< camera exposure, a simplification of iso-value, aperature number and exposure time
		Mat4x4* projection_matrix;	///< camera projection matrix
	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

	// TODO move this elsewhere?
	uint m_active_camera_idx;

public:
	CameraComponentManager(uint size);
	~CameraComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity,
					float near = 0.001f,
					float far = 1000.0f,
					float fovy = 0.5236f,
					float aspect_ratio = 16.0/9.0f,
					float exposure = 0.000036f); // default value for mapping avg luminance of ~5000cd/m^2 to 0.18 intensity

	void deleteComponent(Entity entity);

	bool checkComponent(uint entity_id) const;

	uint getIndex(Entity entity) const;

	uint getIndex(uint entity_id) const;

	void setActiveCamera(Entity entity);

	uint getActiveCameraIndex() const;

	Entity getEntity(uint index) const;

	void setCameraAttributes(uint index, float near, float far, float fovy = 0.5236f, float aspect_ratio = 16.0/9.0f, float exposure = 0.000045f);

	void updateProjectionMatrix(uint index);

	Mat4x4 getProjectionMatrix(uint index) const;

	void setNear(uint index, float near);

	void setFar(uint index, float far);

	float getFovy(uint index) const;

	void setFovy(uint index, float fovy);

	float getAspectRatio(uint index) const;

	void setAspectRatio(uint index, float aspect_ratio);
	
	float getExposure(uint index) const;

	void setExposure(uint index, float exposure);
};

#endif