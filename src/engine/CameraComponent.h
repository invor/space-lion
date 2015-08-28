#ifndef CameraComponent_h
#define CameraComponent_h

#include <unordered_map>

#include "EntityManager.h"
#include <glm\glm.hpp>

typedef glm::mat4 Mat4x4;

class CameraComponentManager
{
private:
	struct Data
	{
		uint used;		///< number of components currently in use
		uint allocated;	///< number of components that the allocated memery can hold
		void* buffer;	///< raw data pointer

		Entity* entity;				///< entity owning the component
		float* near;				///< near clipping plane
		float* far;					///< far clipping plane
		float* fovy;				///< camera vertical field of view in radian
		float* aspect_ratio;		///< camera aspect ratio
		Mat4x4* view_matrix;		///< camera view matrix
		Mat4x4* projection_matrix;	///< camera projection matrix

	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

public:
	CameraComponentManager();
	CameraComponentManager(uint size);
	~CameraComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity);

	void addComponent(Entity entity, float near, float far, float fovy, float aspect_ratio);

	void deleteComponent(Entity entity);

	void getIndex(Entity entity);

	void setCameraAttributes(uint index, float near, float far, float fovy, float aspect_ratio);

	void updateViewMatrix(uint index);

	void updateProjectionMatrix(uint index);
};

#endif