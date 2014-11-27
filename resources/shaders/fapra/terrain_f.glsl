/*---------------------------------------------------------------------------------------------------
File: terrain_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 24.01.2014

Description: 
---------------------------------------------------------------------------------------------------*/

#version 430

#define PI 3.1415926535
#define INV_PI 0.318309886183

uniform sampler2D diffuse_tx2D;
uniform sampler2D specular_tx2D;
uniform sampler2D roughness_tx2D;
uniform sampler2D normal_tx2D;

uniform mat4 view_matrix;

in vec3 position;
in vec2 uv_coord;
in vec3 viewer_direction;
in mat3 tangent_space_matrix;

layout (location = 0) out vec3 frag_colour;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec4 tangent_bitangent;
layout (location = 3) out vec4 spec_colour_roughness;
layout (location = 4) out float depth;

void main()
{
	/*	Fetch colour from diffuse map (and blend it with vertex colour) */
	frag_colour = texture(diffuse_tx2D, uv_coord).xyz;
	
	/*	Fetch specular color from specular map */
	spec_colour_roughness.xyz = texture(specular_tx2D, uv_coord).xyz;
	
	/*	Fetch roughness from roughness map */
	spec_colour_roughness.w = texture(roughness_tx2D, uv_coord).x;

	/*	Fetch normal vector from normal map */
	normal = ((texture(normal_tx2D, uv_coord).xyz)*2.0)-1.0;
	
	tangent_bitangent.xy = vec2(tangent_space_matrix[0][0],tangent_space_matrix[1][0]);
	tangent_bitangent.zw = vec2(tangent_space_matrix[0][1],tangent_space_matrix[1][1]);
	
	depth = length(position);
}