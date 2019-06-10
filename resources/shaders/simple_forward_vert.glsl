#version 450
#extension GL_ARB_shader_draw_parameters : require

struct PerDrawData
{
	mat4 model_matrix;
};

layout(std430, binding = 0) readonly buffer PerDrawDataBuffer { PerDrawData[] per_draw_data; };

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

in vec3 v_normal;
in vec3 v_position;
in vec4 v_tangent;
in vec2 v_uvCoord;

out vec3 position;
out vec2 uvCoord;
out mat3 tangent_space_matrix;

const vec2 quadVertices[4] = { vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0) };
const int quadIndices[6] = {3,2,0,0,1,3};

void main()
{   
	/*	Construct matrices that use the model matrix*/
	mat3 normal_matrix = transpose(inverse(mat3(per_draw_data[gl_DrawIDARB].model_matrix)));

	/*	Just to be on the safe side, normalize input vectors again */
	vec3 normal = normalize(v_normal);
	vec3 tangent = normalize(v_tangent.xyz);
	vec3 bitangent = normalize( cross(normal, tangent) * v_tangent.w );
	
	/*	Transform input vectors into view space */
	normal = normalize(normal_matrix * normal);
	tangent = normalize(normal_matrix * tangent);
	bitangent = normalize(normal_matrix * bitangent);

	/*	Compute transformation matrix for tangent space transformation */
	tangent_space_matrix = mat3(
		tangent.x, bitangent.x, normal.x,
		tangent.y, bitangent.y, normal.y,
		tangent.z, bitangent.z, normal.z);
	
	/*	Transform vertex position to view space */
	position = (per_draw_data[gl_DrawIDARB].model_matrix * vec4(v_position,1.0)).xyz;
	
	uvCoord = v_uvCoord;
	
	gl_Position =  projection_matrix * view_matrix * vec4(position, 1.0);

	//gl_Position = vec4(quadVertices[gl_VertexID], 0.0,1.0);
}