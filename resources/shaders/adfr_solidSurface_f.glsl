/*---------------------------------------------------------------------------------------------------
File: dfr_solidSurface_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 16.10.2015

Description: Fragment shader for solid surfaces in deferred rendering geometry pass.
---------------------------------------------------------------------------------------------------*/
#version 430

#extension GL_ARB_shader_storage_buffer_object : require

layout(early_fragment_tests) in;

struct headData
{
	uint fragmentCount;
	uint index;
};

struct gBufferFragment
{
	float depth;
	float normalX;
	float normalY;
	float albedo_color;
	float specular_color;
	
	uint prev_fragment;
};

layout (std430, binding=0) buffer gBuffer
{
	gBufferFragment fragments[];
};

layout (std430, binding=1) buffer head
{
	headData header[];
};

layout(binding=7, offset=0) uniform atomic_uint fragment_counter;

uniform sampler2D diffuse_tx2D;
uniform sampler2D specular_tx2D;
uniform sampler2D roughness_tx2D;
uniform sampler2D normal_tx2D;

uniform vec2 viewport_resolution;

in vec3 position;
in vec2 uvCoord;
in mat3 tangent_space_matrix;

float Vec4ToFloat(vec4 v)
{
	return v.r + v.g * 256.0 + v.b * 256.0 * 256.0 + v.a * 256.0 * 256.0 * 256.0;
}

void main()
{
	float depth = length(position);
	
	vec3 tNormal = ((texture(normal_tx2D, uvCoord).xyz)*2.0)-1.0;
	vec2 normal = tNormal.xy = normalize(transpose(tangent_space_matrix) * tNormal.xyz).xy;
	vec3 tAlbedoRGB = texture(diffuse_tx2D, uvCoord).rgb;
	vec4 specularRGB_roughness = vec4( texture(specular_tx2D, uvCoord).rgb,
									texture(roughness_tx2D, uvCoord).r );
									
	gBufferFragment out_values;
	
	out_values.depth = depth;
	out_values.normalX = normal.x;
	out_values.normalY = normal.y;
	out_values.albedo_color = Vec4ToFloat(vec4(tAlbedoRGB,0.0));
	out_values.specular_color = Vec4ToFloat(specularRGB_roughness);
	
	ivec2 pixel_coords = ivec2(floor(gl_FragCoord.x),floor(gl_FragCoord.y));
	uint header_idx = uint((viewport_resolution.x * pixel_coords.y) + pixel_coords.x);
	
	uint fragment_idx =  atomicCounterIncrement(fragment_counter);
	uint last_index = atomicExchange(header[header_idx].index,fragment_idx);
	header[header_idx].index = fragment_idx;
	atomicAdd(header[header_idx].fragmentCount,1);
	
	out_values.prev_fragment = last_index;
	
	//if(fragments[gBuffer_idx].depth > depth)
	fragments[fragment_idx] = out_values;
}