/*---------------------------------------------------------------------------------------------------
File: terrain_v.glsl
Author: Michael Becher
Date of (presumingly) last edit: 21.01.2014

Description: Terrain vertex shader. Uses vertex displacment to heightmap the terrain
					vertices according to a given heightmap texture.
---------------------------------------------------------------------------------------------------*/

#version 430

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

uniform sampler2D heightmap_tx2D;
uniform int size;

in vec3 v_position;

out vec3 position;
out vec2 uv_coord;
out vec3 viewer_direction;
out mat3 tangent_space_matrix;

void main()
{		
	/*	use gl_InstanceID to translate vertices to their respective position (still in object space) */
	float z_translation = floor( float(gl_InstanceID) / float(size) );
	float x_translation = gl_InstanceID - (size * z_translation);
	vec3 translation = vec3(x_translation, 0.0, z_translation);
	
	position = v_position;
	
	/* compute uv coordinate */
	uv_coord = (v_position+translation).xz / vec2(size);
	
	/*	use normalized position to access height-map and retrieve height-value (y-coordinate) */
	position.y = texture(heightmap_tx2D,uv_coord).x  * 50.0;
	
	/*	Transform vertex position to view space */
	position =  position+translation;
	
	/*	Compute tangent and bitangent from heightmap */
	vec2 x_neighbour_uv = uv_coord + vec2(1.0/float(size),0.0);
	vec2 z_neighbour_uv = uv_coord + vec2(0.0,1.0/float(size));
	float x_neighbour_height = texture(heightmap_tx2D,x_neighbour_uv).x * 50.0;
	float z_neighbour_height = texture(heightmap_tx2D,z_neighbour_uv).x * 50.0;
	
	vec3 tangent = normalize(vec3(0.0,z_neighbour_height-position.y,1.0));
	vec3 bitangent = normalize(vec3(1.0,x_neighbour_height-position.y,0.0));
	vec3 normal = -normalize(cross(bitangent, tangent));
	
	/*	Construct matrices that use the model matrix*/
	mat3 normal_matrix = transpose(inverse(mat3(model_view_matrix)));

	/*	Just to be on the safe side, normalize input vectors again */
	normal = normalize(normal);
	tangent = normalize(tangent);
	bitangent = normalize(bitangent);
	
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
	position = (model_view_matrix * vec4(position,1.0)).xyz;
	
	/*	Compute direction to the viewer/camer and light source into tangent space */
	viewer_direction = normalize(tangent_space_matrix * normalize( -position ));
	
	gl_Position =  projection_matrix  * vec4(position, 1.0);
}