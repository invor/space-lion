#include "DynamicPtexMeshComponent.hpp"

//	size_t EngineCore::Graphics::PtexMeshComponentManager::addComponent(
//		Entity entity,
//		uint32_t texture_tiles_cnt,
//		uint32_t patch_cnt,
//		ResourceID mesh,
//		ResourceID material,
//		ResourceID ptex_parameters,
//		ResourceID ptex_bindless_texture_handles)
//	{
//		auto index = data_.addComponent(
//			{ entity, texture_tiles_cnt, patch_cnt, mesh, material, ptex_parameters, ptex_bindless_texture_handles }
//		);
//	
//		addIndex(entity.id(), index);
//	
//		return index;
//	}
//	
//	void EngineCore::Graphics::PtexMeshComponentManager::setPtexParameters(Entity entity, ResourceID ptex_params)
//	{
//		auto indices = data_.getIndices( getIndex(entity) );
//	
//		data_(indices.first, indices.second).ptex_parameters = ptex_params;
//	}
//	
//	EngineCore::Graphics::ResourceID EngineCore::Graphics::PtexMeshComponentManager::getMesh(uint index)
//	{
//		auto indices = data_.getIndices(index);
//		return data_(indices.first, indices.second).mesh;
//	}
//	
//	EngineCore::Graphics::ResourceID EngineCore::Graphics::PtexMeshComponentManager::getMaterial(uint index)
//	{
//		auto indices = data_.getIndices(index);
//		return data_(indices.first, indices.second).material;
//	}
//	
//	EngineCore::Graphics::ResourceID EngineCore::Graphics::PtexMeshComponentManager::getPtexParameters(uint index)
//	{
//		auto indices = data_.getIndices(index);
//		return data_(indices.first, indices.second).ptex_parameters;
//	}
//	
//	EngineCore::Graphics::ResourceID EngineCore::Graphics::PtexMeshComponentManager::getPtexBindTextureHandles(uint index)
//	{
//		auto indices = data_.getIndices(index);
//		return data_(indices.first, indices.second).ptex_bindless_texture_handles;
//	}