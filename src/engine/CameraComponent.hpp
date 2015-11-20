#ifndef CameraComponent_h
#define CameraComponent_h

#include <unordered_map>
#include <iostream>

#include "EntityManager.hpp"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

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
	CameraComponentManager(uint size);
	~CameraComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity,
					float near = 0.001f,
					float far = 1000.0f,
					float fovy = 0.5236f,
					float aspect_ratio = 16.0/9.0f);

	void deleteComponent(Entity entity);

	const uint getIndex(Entity entity);

	void setCameraAttributes(uint index, float near, float far, float fovy = 0.5236f, float aspect_ratio = 16.0/9.0f);

	void updateProjectionMatrix(uint index);

	const Mat4x4 getProjectionMatrix(uint index);

	const float getFovy(uint index);

	const float getAspectRatio(uint index);
};

#endif