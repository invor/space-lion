/// <copyright file="MeshComponentManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef MeshComponentManager_hpp
#define MeshComponentManager_hpp

#include "EntityManager.hpp"
#include "BaseComponentManager.hpp"
#include "BaseResourceManager.hpp"

namespace EngineCore
{
	namespace Graphics
	{
		template<typename ResourceManagerType>
		class MeshComponentManager : public BaseComponentManager
		{
		public:
			MeshComponentManager(ResourceManagerType* resource_manager) : BaseComponentManager(), m_resource_mngr(resource_manager) {}
			~MeshComponentManager() = default;

			template<
				typename VertexContainer,
				typename IndexContainer,
				typename VertexDescriptor,
				typename IndexType,
				typename MeshType>
			ResourceID addComponent(
				Entity const&                                        entity,
				std::string const&                                   mesh_description,
				std::shared_ptr<std::vector<VertexContainer>> const& vertex_data,
				std::shared_ptr<IndexContainer> const&               index_data,
				std::shared_ptr<VertexDescriptor> const&             vertex_layout,
				IndexType const&                                     index_type,
				MeshType const&                                      mesh_type,
				bool                                                 store_seperate = false);

			std::tuple<unsigned int, unsigned int, unsigned int> getDrawIndexedParams(size_t component_index);

		private:

			struct ComponentData
			{
				ComponentData(
					Entity const       entity,
					std::string const& mesh_description,
					ResourceID const   mesh_rsrc,
					unsigned int const first_index,
					unsigned int const indices_cnt,
					unsigned int const base_vertex)
					: entity(entity),
					mesh_description(mesh_description),
					mesh_resource(mesh_rsrc),
					first_index(first_index),
					indices_cnt(indices_cnt),
					base_vertex(base_vertex)
				{}

				Entity       entity;
				std::string  mesh_description;
				ResourceID   mesh_resource;
				unsigned int first_index;
				unsigned int indices_cnt;
				unsigned int base_vertex;
			};

			struct MeshData
			{
				MeshData(
					ResourceID   mesh_rsrc,
					unsigned int vertices_allocated,
					unsigned int indices_allocated)
					: mesh_resource(mesh_rsrc),
					vertices_allocated(vertices_allocated),
					vertices_used(0),
					indices_allocated(indices_allocated),
					indices_used(0) {}

				ResourceID   mesh_resource;
				unsigned int vertices_allocated;
				unsigned int vertices_used;
				unsigned int indices_allocated;
				unsigned int indices_used;
			};

			std::vector<ComponentData> m_component_data;
			std::vector<MeshData>      m_mesh_data;
			mutable std::shared_mutex  m_data_mutex;

			ResourceManagerType*       m_resource_mngr;

		};

		template<typename ResourceManagerType>
		template<
			typename VertexContainer,
			typename IndexContainer,
			typename VertexDescriptor,
			typename IndexType,
			typename MeshType>
		inline ResourceID MeshComponentManager<ResourceManagerType>::addComponent(
			Entity const&                                        entity,
			std::string const&                                   mesh_description,
			std::shared_ptr<std::vector<VertexContainer>> const& vertex_data,
			std::shared_ptr<IndexContainer> const&               index_data,
			std::shared_ptr<VertexDescriptor> const&             vertex_layout,
			IndexType const&                                     index_type,
			MeshType const&                                      mesh_type,
			bool                                                 store_seperate)
		{
			// compute byte size of per vertex data in first vertex buffer
			std::vector<size_t> vb_attrib_byte_sizes;
			// single vertex buffer signals possible interleaved vertex layout, sum up all attribute byte sizes
			if (vertex_data->size() == 1) {
				vb_attrib_byte_sizes.push_back(0);
				for (auto& attr : vertex_layout->attributes) {
					vb_attrib_byte_sizes.back() += computeAttributeByteSize(attr);
				}
			}
			else {
				for (auto& attr : vertex_layout->attributes) {
					vb_attrib_byte_sizes.push_back(computeAttributeByteSize(attr));
				}
			}

			// get vertex buffer data pointers and byte sizes
			std::vector<size_t> vb_byte_sizes;
			for (auto& vb : (*vertex_data)) {
				vb_byte_sizes.push_back(sizeof(VertexContainer::value_type) * vb.size());
			}
			// compute overall byte size of index buffer
			size_t ib_byte_size = sizeof(IndexContainer::value_type) * index_data->size();

			// computer number of requested vertices and indices
			size_t req_vertex_cnt = vb_byte_sizes.front() / vb_attrib_byte_sizes.front();
			size_t req_index_cnt = ib_byte_size / computeByteSize(index_type);


			auto it = m_mesh_data.begin();
			if (!store_seperate)
			{
				// check for existing mesh batch with matching vertex layout and index type and enough available space
				for (; it != m_mesh_data.end(); ++it)
				{
					auto mesh = m_resource_mngr->getMeshResource(it->mesh_resource);

					bool layout_check = ( (*vertex_layout) == mesh.resource->getVertexLayout());
					//TODO check interleaved vs non-interleaved
					bool idx_type_check = (index_type == mesh.resource->getIndexFormat());

					if (layout_check && idx_type_check)
					{
						// check whether there is enough space left in batch
						size_t ava_vertex_cnt = (it->vertices_allocated - it->vertices_used);
						size_t ava_index_cnt = (it->indices_allocated - it->indices_used);

						if ((req_vertex_cnt < ava_vertex_cnt) && (req_index_cnt < ava_index_cnt))
						{
							break;
						}
					}
				}
			}
			else
			{
				it = m_mesh_data.end();
			}

			std::unique_lock<std::shared_mutex> lock(m_data_mutex);

			// Create a new gpu mesh object if no matching mesh was found
			if (it == m_mesh_data.end())
			{
				std::string mesh_name("MeshComponanteManager_BatchMesh_" + std::to_string(m_mesh_data.size()) );

				ResourceID new_mesh = m_resource_mngr->allocateMeshAsync(
					mesh_name,
					1000000,
					4000000,
					vertex_layout,
					index_type,
					mesh_type);
				
				m_mesh_data.emplace_back(MeshData(new_mesh, 1000000, 4000000));

				it = m_mesh_data.end();
				--it;
			}

			addIndex(entity.id(), m_component_data.size());
			m_component_data.push_back(ComponentData(
				entity,
				mesh_description,
				it->mesh_resource,
				it->indices_used,
				static_cast<unsigned int>(req_index_cnt),
				it->vertices_used));

			// update mesh async (in case a new mesh was created, the data update needs to wait async!)
			m_resource_mngr->updateMeshAsync(
				it->mesh_resource,
				it->vertices_used,
				it->indices_used,
				vertex_data,
				index_data);

			it->vertices_used += static_cast<unsigned int>(req_vertex_cnt);
			it->indices_used += static_cast<unsigned int>(req_index_cnt);

			return it->mesh_resource;
		}

		template<typename ResourceManagerType>
		inline std::tuple<unsigned int, unsigned int, unsigned int> MeshComponentManager<ResourceManagerType>::getDrawIndexedParams(size_t component_index)
		{
			ComponentData const& data = m_component_data[component_index];

			return { data.indices_cnt, data.first_index, data.base_vertex };
		}
}
}

#endif