#include "CameraComponent.hpp"

#include "EntityManager.hpp"

CameraComponentManager::CameraComponentManager(uint size)
	: m_active_camera_idx(0)
{
	const uint bytes = size * (sizeof(Entity)
								+ 5*sizeof(float)
								+ 1*sizeof(Mat4x4));
	m_data.buffer = new uint8_t[bytes];

	m_data.used = 0;
	m_data.allocated = size;

	m_data.entity = (Entity*)(m_data.buffer);
	m_data.near = (float*)(m_data.entity + size);
	m_data.far = (float*)(m_data.near + size);
	m_data.fovy = (float*)(m_data.far + size);
	m_data.aspect_ratio = (float*)(m_data.fovy + size);
	m_data.exposure = (float*)(m_data.aspect_ratio + size);
	m_data.projection_matrix = (Mat4x4*)(m_data.exposure + size);

	//GEngineCore::renderingPipeline().addPerFrameInterfaceGpuTask(std::bind(&CameraComponentManager::drawDebugInterface, this));
}

CameraComponentManager::~CameraComponentManager()
{
	delete[] m_data.buffer;
}

void CameraComponentManager::reallocate(uint size)
{
	Data new_data;
	const uint bytes = size * (sizeof(Entity)
								+ 5*sizeof(float)
								+ 1*sizeof(Mat4x4));
	new_data.buffer = new uint8_t[bytes];

	new_data.used = m_data.used;
	new_data.allocated = size;

	new_data.entity = (Entity*)(new_data.buffer);
	new_data.near = (float*)(new_data.entity + size);
	new_data.far = (float*)(new_data.near + size);
	new_data.fovy = (float*)(new_data.far + size);
	new_data.aspect_ratio = (float*)(new_data.fovy + size);
	new_data.exposure = (float*)(new_data.aspect_ratio + size);
	new_data.projection_matrix = (Mat4x4*)(new_data.exposure + size);

	std::memcpy(new_data.entity, m_data.entity, m_data.used * sizeof(Entity));
	std::memcpy(new_data.near, m_data.near, m_data.used * sizeof(float));
	std::memcpy(new_data.far, m_data.far, m_data.used * sizeof(float));
	std::memcpy(new_data.fovy, m_data.fovy, m_data.used * sizeof(float));
	std::memcpy(new_data.aspect_ratio, m_data.aspect_ratio, m_data.used * sizeof(float));
	std::memcpy(new_data.exposure, m_data.exposure, m_data.used * sizeof(Mat4x4));
	std::memcpy(new_data.projection_matrix, m_data.projection_matrix, m_data.used * sizeof(Mat4x4));

	delete m_data.buffer;

	m_data = new_data;
}

void CameraComponentManager::addComponent(Entity entity, float near, float far, float fovy, float aspect_ratio, float exposure)
{	
	assert(m_data.used < m_data.allocated);

	uint index = m_data.used;

	m_index_map.insert({entity.id(),index});

	m_data.entity[index] = entity;
	m_data.near[index] = near;
	m_data.far[index] = far;
	m_data.fovy[index] = fovy;
	m_data.aspect_ratio[index] = aspect_ratio;
	m_data.exposure[index] = exposure;

	m_data.used++;

	updateProjectionMatrix(index);
}

void CameraComponentManager::deleteComponent(Entity entity)
{
}

bool CameraComponentManager::checkComponent(uint entity_id) const
{
	auto search = m_index_map.find(entity_id);

	if (search == m_index_map.end())
		return false;
	else
		return true;
}

uint CameraComponentManager::getIndex(Entity entity) const
{
	auto search = m_index_map.find(entity.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}

uint CameraComponentManager::getIndex(uint entity_id) const
{
	auto search = m_index_map.find(entity_id);

	assert((search != m_index_map.end()));

	return search->second;
}

void CameraComponentManager::setActiveCamera(Entity entity)
{
	auto search = m_index_map.find(entity.id());

	if(search != m_index_map.end())
		m_active_camera_idx = search->second;
}

uint CameraComponentManager::getActiveCameraIndex() const
{
	return m_active_camera_idx;
}

Entity CameraComponentManager::getEntity(uint index) const
{
	return m_data.entity[index];
}

void CameraComponentManager::setCameraAttributes(uint index, float near, float far, float fovy, float aspect_ratio, float exposure)
{
	assert(index < m_data.used);

	m_data.near[index] = near;
	m_data.far[index] = far;
	m_data.fovy[index] = fovy;
	m_data.aspect_ratio[index] = aspect_ratio;
	m_data.exposure[index] = exposure;

	updateProjectionMatrix(index);
}

void CameraComponentManager::updateProjectionMatrix(uint index)
{
	float near = m_data.near[index];
	float far = m_data.far[index];
	float fovy = m_data.fovy[index];
	float aspect_ratio = m_data.aspect_ratio[index];

	//Mat4x4 projection_matrix;// = m_data.projection_matrix[index];

	m_data.projection_matrix[index] = glm::perspective(fovy,aspect_ratio,near,far);

	//	float f = 1.0f / std::tan(fovy / 2.0f);
	//	float nf = 1.0f / (near - far);
	//	projection_matrix[0][0] = f / aspect_ratio;
	//	projection_matrix[0][1] = 0.0f;
	//	projection_matrix[0][2] = 0.0f;
	//	projection_matrix[0][3] = 0.0f;
	//	projection_matrix[1][0] = 0.0f;
	//	projection_matrix[1][1] = f;
	//	projection_matrix[1][2] = 0.0f;
	//	projection_matrix[1][3] = 0.0f;
	//	projection_matrix[2][0] = 0.0f;
	//	projection_matrix[2][1] = 0.0f;
	//	projection_matrix[2][2] = (far + near) * nf;
	//	projection_matrix[2][3] = -1.0f;
	//	projection_matrix[3][0] = 0.0f;
	//	projection_matrix[3][1] = 0.0f;
	//	projection_matrix[3][2] = (2.0f * far * near) * nf;
	//	projection_matrix[3][3] = 0.0f;
	//	
	//	m_data.projection_matrix[index] = projection_matrix;
}

void CameraComponentManager::setNear(uint index, float near)
{
	m_data.near[index] = near;
}

void CameraComponentManager::setFar(uint index, float far)
{
	m_data.far[index] = far;
}

Mat4x4 CameraComponentManager::getProjectionMatrix(uint index) const
{
	return m_data.projection_matrix[index];
}

float CameraComponentManager::getFovy(uint index) const
{
	return m_data.fovy[index];
}

void CameraComponentManager::setFovy(uint index, float fovy)
{
	m_data.fovy[index] = fovy;
}

float CameraComponentManager::getAspectRatio(uint index) const
{
	return m_data.aspect_ratio[index];
}

void CameraComponentManager::setAspectRatio(uint index, float aspect_ratio)
{
	m_data.aspect_ratio[index] = aspect_ratio;
}

float CameraComponentManager::getExposure(uint index) const
{
	return m_data.exposure[index];
}

void CameraComponentManager::setExposure(uint index, float exposure)
{
	m_data.exposure[index] = exposure;
}