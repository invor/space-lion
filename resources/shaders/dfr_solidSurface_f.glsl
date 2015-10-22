/*---------------------------------------------------------------------------------------------------
File: dfr_solidSurface_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 16.10.2015

Description: Fragment shader for solid surfaces in deferred rendering geometry pass.
---------------------------------------------------------------------------------------------------*/
#version 430

uniform sampler2D diffuse_tx2D;
uniform sampler2D specular_tx2D;
uniform sampler2D roughness_tx2D;
uniform sampler2D normal_tx2D;

in vec3 position;
in vec2 uvCoord;
in mat3 tangent_space_matrix;

layout (location = 0) out vec4 position_depth;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 albedoRGB;
layout (location = 3) out vec4 specularRGB_roughness;

void main()
{
	position_depth = vec4(position,length(position));
	
	vec3 tNormal = ((texture(normal_tx2D, uvCoord).xyz)*2.0)-1.0;
	normal = transpose(tangent_space_matrix) * tNormal.xyz;
	
	albedoRGB = texture(diffuse_tx2D, uvCoord).rgb;
	
	specularRGB_roughness = vec4( texture(specular_tx2D, uvCoord).rgb,
									texture(roughness_tx2D, uvCoord).r );
}