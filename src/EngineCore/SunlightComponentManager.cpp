#include "SunlightComponent.hpp"

#include "EntityManager.hpp"
#include "DeferredRenderingPipeline.hpp"

#include "GlobalCoreComponents.hpp"
#include "TransformComponent.hpp"

#include "EditorUI.hpp"
#include "GlobalTools.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

SunlightComponentManager::SunlightComponentManager(uint size)
{
	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Vec3)
								+ (2 * sizeof(float)) );
	m_data.buffer = new uint8_t[bytes];

	m_data.used = 0;
	m_data.allocated = size;

	m_data.entity = (Entity*)(m_data.buffer);
	m_data.light_colour = (Vec3*)(m_data.entity + size);
	m_data.lumen = (float*)(m_data.light_colour + size);
	m_data.star_radius = (float*)(m_data.lumen + size);
	

	//GEngineCore::renderingPipeline().addPerFrameInterfaceGpuTask(std::bind(&SunlightComponentManager::drawDebugInterface, this));
}

SunlightComponentManager::~SunlightComponentManager()
{
	delete[] m_data.buffer;
}

void SunlightComponentManager::reallocate(uint size)
{
	Data new_data;

	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Vec3)
								+ (2 * sizeof(float)) );
	new_data.buffer = new uint8_t[bytes];

	new_data.used = m_data.used;
	new_data.allocated = size;

	new_data.entity = (Entity*)(new_data.buffer);
	new_data.light_colour = (Vec3*)(new_data.entity + size);
	new_data.lumen = (float*)(new_data.light_colour + size);
	new_data.star_radius = (float*)(new_data.lumen + size);

	std::memcpy(m_data.entity, new_data.entity, m_data.used * sizeof(Entity));
	std::memcpy(m_data.light_colour, new_data.light_colour, m_data.used * sizeof(Vec3));
	std::memcpy(m_data.lumen, new_data.lumen, m_data.used * sizeof(float));
	std::memcpy(m_data.star_radius, new_data.star_radius, m_data.used * sizeof(float));

	delete m_data.buffer;

	m_data = new_data;
}

void SunlightComponentManager::addComponent(Entity entity, Vec3 light_colour, float lumen, float star_radius)
{
	assert(m_data.used < m_data.allocated);

	uint index = m_data.used;

	m_index_map.insert({entity.id(),index});

	m_data.entity[index] = entity;
	m_data.light_colour[index] = light_colour;
	m_data.lumen[index] = lumen;
	m_data.star_radius[index] = star_radius;

	m_data.used++;
}

void SunlightComponentManager::deleteComonent(Entity entity)
{

}

uint SunlightComponentManager::getIndex(Entity entity)
{
	auto search = m_index_map.find(entity.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}

std::pair<bool, uint> SunlightComponentManager::getIndex(uint entity_id)
{
	auto search = m_index_map.find(entity_id);

	std::pair<bool, uint> rtn;

	if (search != m_index_map.end())
	{
		rtn.first = true;
		rtn.second = search->second;
	}
	else
	{
		rtn.first = false;
		rtn.second = 0;
	}

	return rtn;
}

void SunlightComponentManager::setColour(uint index, Vec3 colour)
{
	m_data.light_colour[index] = colour;
}

void SunlightComponentManager::setLumen(uint index, float lumen)
{
	m_data.lumen[index] = lumen;
}

void SunlightComponentManager::setStarRadius(uint index, float radius)
{
	m_data.star_radius[index] = radius;
}

Entity SunlightComponentManager::getEntity(uint index) const
{
	return m_data.entity[index];
}

Vec3 SunlightComponentManager::getColour(uint index) const
{
	return m_data.light_colour[index];
}

float SunlightComponentManager::getLumen(uint index) const
{
	return m_data.lumen[index];
}

float SunlightComponentManager::getStarRadius(uint index) const
{
	return m_data.star_radius[index];
}