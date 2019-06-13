#include "gltfSceneLoading.hpp"

#include <iostream>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <tiny_gltf.h>

#include "../EntityManager.hpp"
#include "../GenericVertexLayout.hpp"
#include "ResourceManager.hpp"
#include "../WorldState.hpp"

void EngineCore::Graphics::OpenGL::addGLTFNode(
    int node_idx,
    Entity parent_node,
    std::shared_ptr<tinygltf::Model> const model,
    WorldState & world,
    ResourceManager& resource_mngr)
{
	// add entity
	auto entity = world.accessEntityManager().create();

	// add name
	world.accessNameManager().addComponent(entity, model->nodes[node_idx].name);

	// add transform
	size_t transform_idx = world.accessTransformManager().addComponent(entity);

	if (parent_node != world.accessEntityManager().invalidEntity())
	{
		world.accessTransformManager().setParent(transform_idx, parent_node);
	}

	if (model->nodes[node_idx].matrix.size() != 0) // has matrix transform
	{
		// TODO
	}
	else
	{
		auto& translation = model->nodes[node_idx].translation;
		auto& scale = model->nodes[node_idx].scale;
		auto& rotation = model->nodes[node_idx].rotation;

		if (translation.size() != 0) {
			world.accessTransformManager().setPosition(
				transform_idx,
				Vec3(static_cast<float>(translation[0]), static_cast<float>(translation[1]), static_cast<float>(translation[2]))
			);
		}
		if (scale.size() != 0) {
			world.accessTransformManager().scale(
				transform_idx,
				Vec3(static_cast<float>(scale[0]), static_cast<float>(scale[1]), static_cast<float>(scale[2]))
			);
		}
		if (rotation.size() != 0) {
			world.accessTransformManager().setOrientation(
				transform_idx,
				Quat(static_cast<float>(rotation[0]),
					static_cast<float>(rotation[1]),
					static_cast<float>(rotation[2]),
					static_cast<float>(rotation[3]))
			);
		}
	}

	if (model->nodes[node_idx].mesh != -1)
	{
		auto primitive_cnt = model->meshes[model->nodes[node_idx].mesh].primitives.size();

		for (size_t primitive_idx = 0; primitive_idx < primitive_cnt; ++primitive_idx)
		{
			// add bbox component
			auto max_data = model->accessors[model->meshes[model->nodes[node_idx].mesh].primitives[primitive_idx].attributes.find("POSITION")->second].maxValues;
			auto min_data = model->accessors[model->meshes[model->nodes[node_idx].mesh].primitives[primitive_idx].attributes.find("POSITION")->second].minValues;
			Vec3 max(static_cast<float>(max_data[0]), static_cast<float>(max_data[1]), static_cast<float>(max_data[2]));
			Vec3 min(static_cast<float>(min_data[0]), static_cast<float>(min_data[1]), static_cast<float>(min_data[2]));

			//app_content.accessBoundingBoxManager().addComponent(entity, DirectX::BoundingBox(
			//	(max + min) * 0.5f,
			//	(max - min) * 0.5f));

			auto mesh_data = loadSingleNodeGLTFMeshPrimitiveData(model, node_idx, primitive_idx);

			auto material_idx = model->meshes[model->nodes[node_idx].mesh].primitives[primitive_idx].material;
			std::string material_name = "";
			std::array<float, 4> base_colour = { 1.0f,0.0f,1.0f,1.0f };
			float metalness = 0.0f;
			float roughness = 0.8f;
			std::array<float, 4> specular_colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			if (material_idx != -1)
			{
				material_name = model->materials[material_idx].name;

				auto baseColour_query = model->materials[material_idx].values.find("baseColorFactor");
				auto metallic_query = model->materials[material_idx].values.find("metallicFactor");
				auto roughness_query = model->materials[material_idx].values.find("roughnessQuery");

				if (metallic_query != model->materials[material_idx].values.end()) {
					metalness = static_cast<float>(metallic_query->second.Factor());
				}

				if (baseColour_query != model->materials[material_idx].values.end()){
					auto c = baseColour_query->second.ColorFactor();
					base_colour = {
						static_cast<float>(c[0]) * (1.0f - metalness),
						static_cast<float>(c[1]) * (1.0f - metalness),
						static_cast<float>(c[2]) * (1.0f - metalness),
						static_cast<float>(c[3])
					};
					// assume a specular color value of 0.04 (around plastic) as default value for dielectrics
					specular_colour = {
						(static_cast<float>(c[0]) * metalness) + 0.04f * (1.0f - metalness),
						(static_cast<float>(c[1]) * metalness) + 0.04f * (1.0f - metalness),
						(static_cast<float>(c[2]) * metalness) + 0.04f * (1.0f - metalness),
						static_cast<float>(c[3])
					};

					//debugging
					if (material_name == "orange") {

						std::cout << "orange" << std::endl;

					}
				}

				if (roughness_query != model->materials[material_idx].values.end()) {
					roughness = static_cast<float>(roughness_query->second.Factor());
				}
			}

			EngineCore::Graphics::ResourceID mesh_rsrc = world.accessMeshComponentManager().addComponent(
				entity,
				"gltf_debug_mesh",
				std::get<1>(mesh_data),
				std::get<2>(mesh_data),
				std::get<0>(mesh_data),
				std::get<3>(mesh_data),
                0x0004/*GL_TRIANGLES*/);

			typedef std::pair < std::string, GLSLProgram::ShaderType > ShaderFilename;

			auto shader_names = std::make_shared<std::vector<ShaderFilename>>();
			shader_names->push_back({ "../resources/shaders/simple_forward_vert.glsl",GLSLProgram::VertexShader });
            shader_names->push_back({ "../resources/shaders/simple_forward_frag.glsl", GLSLProgram::FragmentShader });

			EngineCore::Graphics::ResourceID shader_rsrc = resource_mngr.createShaderProgramAsync(
				"gltf_debug_shader",
				shader_names);

			world.accessMaterialComponentManager().addComponent(entity, material_name, shader_rsrc,base_colour,specular_colour,roughness);

			size_t mesh_subidx = world.accessMeshComponentManager().getIndex(entity).size() - 1;
			size_t mtl_subidx = world.accessMaterialComponentManager().getIndex(entity).size() - 1;;

			//for (int subidx = 0; subidx < component_idxs.size(); ++subidx)
			world.accessRenderTaskComponentManager().addComponent(entity, mesh_rsrc, mesh_subidx, shader_rsrc, mtl_subidx);
		}
	}

	// traverse children and add gltf nodes recursivly
	for (auto child : model->nodes[node_idx].children)
	{
		addGLTFNode(child, entity, model, world, resource_mngr);
	}

	// if current node has no mesh to take the bounding box from, create bounding box based on children
	//if ((model->nodes[node_idx].mesh == -1) && (model->nodes[node_idx].children.size() > 0))
	//{
	//	Vec3 max(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
	//	Vec3 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	//
	//	auto children = world.accessTransformManager().getAllChildren(entity);
	//
	//	for (auto& ce : children)
	//	{
	//		auto bb_query = world.accessBoundingBoxManager().getIndex(ce);
	//		auto tf_query = world.accessTransformManager().getIndex(ce);
	//
	//		if (bb_query.first && tf_query.first)
	//		{
	//			auto ce_bbox = app_content.accessBoundingBoxManager().getBoundingBox(bb_query.second);
	//			auto ce_pos = app_content.accessTransformManager().getPosition(tf_query.second);
	//			// TODO fix that this assumes axis-aligned bounding boxes!
	//			max = Vec3::Max(max, ce_pos + ce_bbox.Center + ce_bbox.Extents);
	//			min = Vec3::Min(min, ce_pos + ce_bbox.Center - ce_bbox.Extents);
	//		}
	//	}
	//
	//	world.accessBoundingBoxManager().addComponent(entity, DirectX::BoundingBox(
	//		(max + min) * 0.5f,
	//		(max.x > min.x) ? (max - min) * 0.5f : Vec3(0.0f, 0.0f, 0.0f))
	//	);
	//}
}

std::shared_ptr<tinygltf::Model> EngineCore::Graphics::OpenGL::loadGLTFModel(std::string gltf_filepath)
{
	std::shared_ptr<tinygltf::Model> model = std::make_shared<tinygltf::Model>();
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	auto ret = loader.LoadASCIIFromFile(
		model.get(),
		&err,
		&warn,
        gltf_filepath
	);

	if (!warn.empty()) {
		std::cout << "Warn: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cout << "Err: " << err << std::endl;
	}

	if (!ret) {
		std::cout << "Failed to parse glTF\n" << std::endl;
		return nullptr;
	}

	return model;
}

void EngineCore::Graphics::OpenGL::loadGLTFScene(std::string gltf_filepath, WorldState& world, ResourceManager& resource_mngr)
{
	auto gltf_model = loadGLTFModel(gltf_filepath);
	
	// iterate over nodes of model and add entities+components to world
	for (auto& scene : gltf_model->scenes)
	{
		for (auto node : scene.nodes)
		{
			addGLTFNode(node, world.accessEntityManager().invalidEntity(), gltf_model, world, resource_mngr);
		}
	}
}

std::tuple<
    EngineCore::Graphics::OpenGL::VertexLayoutPtr, 
    EngineCore::Graphics::OpenGL::VertexDataPtr,
    EngineCore::Graphics::OpenGL::IndexDataPtr,
    EngineCore::Graphics::OpenGL::IndexDataType> EngineCore::Graphics::OpenGL::loadSingleNodeGLTFMeshPrimitiveData(
        std::shared_ptr<tinygltf::Model> model,
        size_t node_index,
        size_t primitive_idx)
{
	if (model == nullptr)
		return { nullptr,nullptr,nullptr,0 };

	if (node_index < model->nodes.size() && model->nodes[node_index].mesh != -1)
	{
		auto& indices_accessor = model->accessors[model->meshes[model->nodes[node_index].mesh].primitives[primitive_idx].indices];
		auto& indices_bufferView = model->bufferViews[indices_accessor.bufferView];
		auto& indices_buffer = model->buffers[indices_bufferView.buffer];
		auto indices = std::make_shared<std::vector<unsigned char>>(
			indices_buffer.data.begin() + indices_bufferView.byteOffset + indices_accessor.byteOffset,
			indices_buffer.data.begin() + indices_bufferView.byteOffset + indices_accessor.byteOffset
			+ (indices_accessor.count * indices_accessor.ByteStride(indices_bufferView)));

		auto vertices = std::make_shared<std::vector<std::vector<unsigned char>>>();
		auto vertex_descriptor = std::make_shared<GenericVertexLayout>();
        vertex_descriptor->byte_size = 0; //TODO rename byte_size

		UINT input_slot = 0;

		auto& vertex_attributes = model->meshes[model->nodes[node_index].mesh].primitives[primitive_idx].attributes;
		for (auto attrib : vertex_attributes)
		{
			auto& vertexAttrib_accessor = model->accessors[attrib.second];
			auto& vertexAttrib_bufferView = model->bufferViews[vertexAttrib_accessor.bufferView];
			auto& vertexAttrib_buffer = model->buffers[vertexAttrib_bufferView.buffer];

            vertex_descriptor->attributes.push_back(
                GenericVertexLayout::Attribute(vertexAttrib_accessor.type, vertexAttrib_accessor.componentType,
                    vertexAttrib_accessor.normalized, vertexAttrib_accessor.byteOffset));

			vertices->push_back(std::vector<unsigned char>(
				vertexAttrib_buffer.data.begin() + vertexAttrib_bufferView.byteOffset + vertexAttrib_accessor.byteOffset,
				vertexAttrib_buffer.data.begin() + vertexAttrib_bufferView.byteOffset + vertexAttrib_accessor.byteOffset
				+ (vertexAttrib_accessor.count * vertexAttrib_accessor.ByteStride(vertexAttrib_bufferView)))
			);

		}

		return { vertex_descriptor, vertices, indices, indices_accessor.componentType };
	}
	else
	{
		return { nullptr,nullptr,nullptr,0 };
	}
}
