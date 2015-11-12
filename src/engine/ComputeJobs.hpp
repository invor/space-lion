#ifndef ComputeJobs
#define ComputeJobs

#include <list>
#include <memory>
#include <queue>

#include "EntityManager.hpp"
#include "glowl.h"
#include "types.hpp"

/**
 * Compact representation of a compute job. Contains an entity and pointers
 * to the graphic resources (that are already created and managed in the ResourceManager),
 * as well as addition uniform input data.
 */
struct ComputeJob
{
	union UniformValueType
	{
		const Vec2* const v;
		const Mat4x4* const m;
	};

	struct Uniform
	{
		std::string name;
		UniformValueType value;
	};

	struct Image2D
	{
		std::string name;
		std::shared_ptr<Texture2D> texture;
	};

	struct Image3D
	{
		std::string name;
		std::shared_ptr<Texture3D> texture;
	};

	struct SSBO
	{
		GLuint index;

		std::shared_ptr<ShaderStorageBufferObject> ssbo;
	};

	Entity entity;

	std::shared_ptr<GLSLProgram> compute_prgm;

	GLuint num_groups_x, num_groups_y, num_groups_z;

	std::vector<Uniform> uniforms;
	std::vector<Image2D> textures;
	std::vector<Image3D> volumes;
	std::vector<SSBO> ssbos;
};

struct TextureRequest
{
	std::string name;

	uint width;
	uint height;

	GLenum format;
	GLenum internal_format;
};

struct ComputeJobRequest
{
	ComputeJobRequest(Entity entity, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
		: entity(entity), num_groups_x(num_groups_x), num_groups_y(num_groups_y), num_groups_z(num_groups_z) {}

	Entity entity;

	std::string compute_prgm;

	GLuint num_groups_x, num_groups_y, num_groups_z;

	std::vector<ComputeJob::Uniform> uniforms;

	std::vector<std::string> textures_ids;
	std::vector<std::string> volume_ids;
	std::vector<std::string> ssbo_ids;

	bool oneshot;
};

class ComputeJobManager
{
public:
	ComputeJobManager();
	~ComputeJobManager();

	void addOneshotComputeJob(ComputeJob new_job);

	void addRepeatingComputeJob(ComputeJob new_job);

	void executeOneshotComputerJobs();

	void executeRepeatingComputeJobs();

private:
	/** Collection of compute jobs that are executed only once */
	std::queue<ComputeJob> m_oneshot_jobs;
	/** Collection of compute jobs that are executed regularily */
	std::list<ComputeJob> m_repeating_jobs;
};

#endif