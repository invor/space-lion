#version 430

layout (location = 0) in vec3 w_position;
layout (location = 1) in vec3 w_normal;

layout (location = 0) out vec4 normal;
layout (location = 1) out float depth;
layout (location = 2) out vec3 albedoRGB;
layout (location = 3) out vec4 specularRGB_roughness;

void main()
{
	depth = length(w_position);
	
	normal.xyz = w_normal * 0.5 + 0.5;
	
	albedoRGB = vec3(0.8);
	
	specularRGB_roughness = vec4( vec3(0.8), 0.7);							
}