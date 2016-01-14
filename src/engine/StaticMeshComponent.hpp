#ifndef StaticMeshComponent_hpp
#define StaticMeshComponent_hpp

#include <map>
#include <string>

#include "MTQueue.hpp"
#include "RenderJobs.hpp"
#include "ResourceManager.h"

class StaticMeshComponentManager
{	
private:
	/** Data a single component carries. */
	struct Data
	{
		Data(Entity e, std::string material_path, std::string mesh_path, bool cast_shadow)
		: entity(e), material_path(material_path), mesh_path(mesh_path), cast_shadow(cast_shadow) {}

		Entity entity;
		std::string material_path;
		std::string mesh_path;
		bool cast_shadow;
	};

	/** Dynamic storage of components data (Since components contains strings, dynamic allocation with a vector is used). */
	std::vector<Data> m_data;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint,uint> m_index_map;

	/** Thread safe queue containing indices of added components that haven't been registerd by the Rendering Pipeline yet. */
	MTQueue<uint> m_added_components_queue;

	std::vector<Data>& getData() { return m_data; }
	MTQueue<uint>& getComponentsQueue()  { return m_added_components_queue; }

	/* Grant Rendering Pipeline access to private members. */
	friend class DeferredRenderingPipeline;

public:
	StaticMeshComponentManager();
	~StaticMeshComponentManager();

	void addComponent(Entity e, std::string material_path, std::string mesh_path, bool cast_shadow);
};

#endif