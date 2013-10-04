/*---------------------------------------------------------------------------------------------------
File: surface_lighting_v.glsl
Author: Michael Becher
Date of (presumingly) last edit: 11.09.2013

Description: Standard vertex shader for lighting calculations of opaque surfaces.
---------------------------------------------------------------------------------------------------*/
#version 330

/*	Support handling for up to 256 instances, and reserve a fixed amount of uniforms for it */
uniform mat4 model_view_matrix[256];
uniform mat4 projection_matrix;

in vec3 v_position;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec4 v_colour;
in vec2 v_uv_coord;

out vec3 position;
//out vec4 colour;
out vec2 uv_coord;
out vec3 viewer_direction;
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
	
	/*	Compute direction to the viewer/camer and light source into tangent space */
	viewer_direction = normalize(tangent_space_matrix * normalize( -position ));
	
	//colour = v_colour;
	uv_coord = v_uv_coord;
	
	gl_Position =  projection_matrix  * vec4(position, 1.0);
}