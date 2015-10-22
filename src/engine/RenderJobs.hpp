#ifndef RenderJobs_hpp
#define RenderJobs_hpp

#include <memory>
#include <vector>
#include <string>

#include "EntityManager.hpp"
#include "GLSLProgram.h"
#include "Material.h"
#include "mesh.h"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include "LightComponent.hpp"
#include "types.hpp"
#include "ResourceManager.h"

/**
 * A request for a render job is compromised of an entity and the paths to
 * the graphic resources. Typically used in the RenderingPipeline class
 * to create necessary resources and pass these on to a RenderJobManager using
 * a RenderJob.
 */
struct RenderJobRequest
{
	RenderJobRequest(Entity e, std::string material_path, std::string mesh_path)
		: entity(e), material_path(material_path), mesh_path(mesh_path) {}

	Entity entity;
	std::string material_path;
	std::string mesh_path;
};

/**
 * Compact representation of a render job. Contains an entity and pointers
 * to the graphic resources (that are already created and managed in the ResourceManager).
 */
struct RenderJob
{
	RenderJob(Entity e, std::shared_ptr<Material> material, std::shared_ptr<Mesh> mesh)
		: entity(e), material(material), mesh(mesh) {}

	Entity entity;
	std::shared_ptr<Material> material;
	std::shared_ptr<Mesh> mesh;
};

/**
 * Manages a set of render jobs within a tree datastructure optimized for rendering.
 * Theoretically a render job is a per-frame task issued to the renderer by each
 * entity that has a visual representation. It primarily contains information 
 * about the graphics resources that are used during rendering.
 * In practise, each path from the root node of the tree datastructure to a
 * leave node represents a single render job.
 * Render jobs are processed by traversing the tree in what is basically
 * a depth-first-search, and are handled in batches (using instancing) if possible.
 */
class RenderJobManager
{
private:
	struct MeshNode
	{
		std::shared_ptr<Mesh> mesh;
	
		uint instance_count;
		std::vector<Entity> enities;	///< List of entites that use this mesh. The leaves of the tree datastructure.
	};

	struct MaterialNode
	{
		std::shared_ptr<Material> material;
	
		std::vector<MeshNode> meshes;
	};

	struct ShaderNode
	{
		std::shared_ptr<GLSLProgram> shader_prgm;
	
		std::vector<MaterialNode> materials;
	};

	// Idea: Camera or Framebuffer node? Each camera owns a framebuffer, and choice of camera/fbo is part of a draw job

	struct RootNode
	{
		uint num_render_jobs;

		std::vector<ShaderNode> shaders;
	};

	RootNode m_root;

	friend class RenderingPipeline;
	friend class DeferredRenderingPipeline;

public:
	RenderJobManager();
	~RenderJobManager();

	void addRenderJob(RenderJob new_job);

	RootNode getRoot();

	void clearRenderJobs();
};

#endif