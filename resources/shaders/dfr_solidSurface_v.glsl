/*---------------------------------------------------------------------------------------------------
File: dfr_solidSurface_v.glsl
Author: Michael Becher
Date of (presumingly) last edit: 16.10.2015

Description: Vertex shader for solid surfaces in deferred rendering geometry pass.
---------------------------------------------------------------------------------------------------*/
#version 430

/*	Support handling for up to 128 instances, and reserve a fixed amount of uniforms for it */
uniform mat4 model_view_matrix[128];
uniform mat4 projection_matrix;

in vec3 v_position;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec4 v_colour;
in vec2 v_uvCoord;

out vec3 position;
out vec2 uvCoord;
out mat3 tangent_space_matrix;

void main()
{
	/*	Construct matrices that use the model matrix*/
	mat3 normal_matrix = transpose(inverse(mat3(model_view_matrix[gl_InstanceID])));

	/*	Just to be on the safe side, normalize input vectors again */
	vec3 normal = normalize(v_normal);
	vec3 tangent = normalize(v_tangent);
	vec3 bitangent = normalize(v_bitangent);
	
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
	position = (model_view_matrix[gl_InstanceID] * vec4(v_position,1.0)).xyz;
	
	uvCoord = v_uvCoord;
	
	gl_Position =  projection_matrix  * vec4(position, 1.0);
}