/*---------------------------------------------------------------------------------------------------
File: dfr_lighting_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 16.10.2015

Description: Fragment shader for deferred rendering lighting pass.
---------------------------------------------------------------------------------------------------*/
#version 430

#extension GL_ARB_shader_storage_buffer_object : require

#define PI 3.1415926535
#define INV_PI 0.318309886183

struct LightProperties 
{
	vec3 position;
	vec3 intensity;
};

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

uniform LightProperties lights;
uniform int num_lights;

uniform vec2 viewport_resolution; 
uniform mat4 view_matrix;
uniform vec2 fov;

in vec2 uvCoord;

layout (location = 0) out vec4 frag_colour;


vec3 cookTorranceShading(in vec3 surface_albedo,
						in vec3 surface_specular_color,
						in float surface_roughness,
						in vec3 surface_normal,
						in vec3 light_direction,
						in vec3 viewer_direction,
						in vec3 light_colour)
{
	vec3 halfway = normalize(light_direction + viewer_direction);
	float n_dot_h = dot(surface_normal,halfway);
	float n_dot_l = dot(surface_normal,light_direction);
	float n_dot_v = dot(surface_normal,viewer_direction);
	/* prevent black artefacts */
	n_dot_v = (n_dot_v < 0.0) ? 0.0 : n_dot_v;
	float l_dot_h = dot(light_direction,halfway);
	float roughness_squared = pow(surface_roughness,2.0);
	
	/*
	/	Compute Fresnel term using the Schlick approximation.
	/	To avoid artefacts, a small epsilon is added to 1.0-l_dot_h
	*/
	vec3 fresnel_term = mix(surface_specular_color,vec3(1.0), pow(1.01-l_dot_h,5.0) );
	
	/*	
	/	Compute geometric attenuation / visbility term, based on Smith shadowing term and following 
	/	"Crafting a Next-Gen Material Pipeline for The Order: 1886" Equation 8/9
	/	from SIGGRAPH 2013 Course Notes.
	*/
	float v_1 = n_dot_v + sqrt( roughness_squared+pow(n_dot_v,2.0)-roughness_squared*pow(n_dot_v,2.0) );
	float v_2 = n_dot_l + sqrt( roughness_squared+pow(n_dot_l,2.0)-roughness_squared*pow(n_dot_l,2.0) ) ; 
	float visibility_term =  1.0/(v_1 * v_2);
	
	/*	Compute micro-facet normal distribution term using GGX distribution by Walter et al (2007) */
	float distribution_term = roughness_squared/
								(PI*pow(pow(n_dot_h,2.0)*roughness_squared-pow(n_dot_h,2.0)+1.0 , 2.0));
								
	/*	Compute Cook Torrance BRDF */
	vec3 specular_brdf = fresnel_term*(visibility_term*distribution_term);
	
	/*
	/	Compute diffuse lambertian BRDF.
	/	The specular reflection takes away some energy from the diffuse reflection.
	/	Only the Fresnel term is considered, as to not include specular reflected light "blocked" 
	/	by the geometry or distribution term in the diffuse energy.
	/	To avoid tampering of the colour, the mean of all three colour channels is considered.
	/
	/	dot product with vec(1/3)  implements (r+g+b)/3
	*/
	const vec3 third = vec3(0.333);
	vec3 diffuse_brdf = surface_albedo * (INV_PI-dot(fresnel_term,third)*INV_PI);
    
	return (light_colour*diffuse_brdf + light_colour*specular_brdf) * max(0.0,n_dot_l);
}

vec4 floatToVec4(float f)
{
	vec4 v;
	v.a = floor(f / 256.0 / 256.0 / 256.0);
    v.b = floor((f - v.a * 256.0 * 256.0 * 256.0) / 256.0 / 256.0);
    v.g = floor((f - v.a * 256.0 * 256.0 * 256.0 - v.b * 256.0 * 256.0) / 256.0);
    v.r = floor(f - v.a * 256.0 * 256.0 * 256.0 - v.b * 256.0 * 256.0 - v.g * 256.0);
    // now we have a vec3 with the 3 components in range [0..256]. Let's normalize it!
    return v / 256.0;
}

void main()
{
	ivec2 pixel_coords = ivec2(floor(gl_FragCoord.x),floor(gl_FragCoord.y));
	uint header_idx = uint((viewport_resolution.x * pixel_coords.y) + pixel_coords.x);
	
	uint fragment_idx = header[header_idx].index;
	
	gBufferFragment in_values = fragments[fragment_idx];
	
	headData header_reset;
	gBufferFragment fragment_reset;
	fragment_reset.depth = 999999.9;
	
	for(uint i=0; i < header[header_idx].fragmentCount; i++)
	{
		if(fragments[fragment_idx].depth < in_values.depth)
		{
			in_values = fragments[fragment_idx];
		}
		
		fragments[fragment_idx] =  fragment_reset;
		
		fragment_idx = in_values.prev_fragment;
	}
	
	header[header_idx] = header_reset;
	
	
	float depth = in_values.depth;
	
	vec3 position = normalize( vec3( vec2(tan(fov.x/2.0),tan(fov.y/2.0)) * ((uvCoord*2.0)-1.0),-1.0) ) * depth;
	
	vec3 normal = vec3(in_values.normalX,in_values.normalY,0.0);
	normal.z = 1.0 - length(normal.xy);
	vec3 albedoRGB = floatToVec4(in_values.albedo_color).rgb;
	
	vec4 specularRGB_roughness = floatToVec4(in_values.specular_color);
	vec3 specularRGB = specularRGB_roughness.rgb;
	float roughness = specularRGB_roughness.a;
	
	
	/*	Calculate Cook Torrance shading for each light source */
	vec3 rgb_linear = vec3(0.0);
	/*	CAUTION: arbitrary values in use */
	vec3 viewer_direction = normalize(-position);
	LightProperties lights_view_space;
	lights_view_space.intensity = lights.intensity;
	lights_view_space.position = normalize( (view_matrix * vec4(lights.position,1.0)).xyz - position);
	
	/*	Quick&Dirty light attenuation */
	vec3 light_intensity = 100.0 * lights_view_space.intensity / pow(length(position-(view_matrix*vec4(lights.position,1.0)).xyz),2.0);
	
	rgb_linear += cookTorranceShading(albedoRGB,
										specularRGB,
										roughness,
										normal,
										lights_view_space.position,
										viewer_direction,
										light_intensity);
	
	/*	Temporary gamma correction */
	frag_colour = vec4( pow( rgb_linear, vec3(1.0/2.2) ), 1.0);
	
	frag_colour = vec4(normal,1.0);
}