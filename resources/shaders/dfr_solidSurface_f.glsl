/*---------------------------------------------------------------------------------------------------
File: dfr_solidSurface_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 30.10.2015

Description: Fragment shader for solid surfaces in deferred rendering geometry pass.
---------------------------------------------------------------------------------------------------*/
#version 430

uniform sampler2D tx2D_0; // diffuse
uniform sampler2D tx2D_1; // specular
uniform sampler2D tx2D_2; // roughness
uniform sampler2D tx2D_3; // normal

in vec3 position;
in vec2 uvCoord;
in mat3 tangent_space_matrix;

layout (location = 0) out vec4 normal;
layout (location = 1) out float depth;
layout (location = 2) out vec3 albedoRGB;
layout (location = 3) out vec4 specularRGB_roughness;

void main()
{
	depth = length(position);
	
	vec3 tNormal = ((texture(tx2D_3, uvCoord).xyz)*2.0)-1.0;
	normal.xyz = (normalize(transpose(tangent_space_matrix) * tNormal)) * 0.5 + 0.5;
	
	albedoRGB = texture(tx2D_0, uvCoord).rgb;
	
	specularRGB_roughness = vec4( texture(tx2D_1, uvCoord).rgb,
									texture(tx2D_2, uvCoord).r );							
}