/*---------------------------------------------------------------------------------------------------
File: dfr_solidSurface_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 30.10.2015

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

layout (location = 0) out vec4 normal_depth;
layout (location = 1) out vec3 albedoRGB;
layout (location = 2) out vec4 specularRGB_roughness;

void main()
{
	float depth = length(position);
	
	vec3 tNormal = ((texture(normal_tx2D, uvCoord).xyz)*2.0)-1.0;
	normal_depth.xy = (normalize(transpose(tangent_space_matrix) * tNormal)).xy;
	normal_depth.zw = vec2(floor(depth*1024),fract(depth*1024)); 
	
	albedoRGB = texture(diffuse_tx2D, uvCoord).rgb;
	
	specularRGB_roughness = vec4( texture(specular_tx2D, uvCoord).rgb,
									texture(roughness_tx2D, uvCoord).r );							
}