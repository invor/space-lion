#ifndef SelectComponent_hpp
#define SelectComponent_hpp

#include "EntityManager.hpp"

class SelectComponentManager
{
private:
	struct Data
	{
		Entity entity;
		bool selected;

		std::string material_path;
		std::string mesh_path;
		std::vector<uint8_t> vertex_data;
		std::vector<uint32_t> index_data;
		VertexDescriptor vertex_description;
		GLenum mesh_type;
	};

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
	SelectComponentManager(uint size);
	~SelectComponentManager();

	void reallocate(uint size);

};

#endif