/*---------------------------------------------------------------------------------------------------
File: dfr_lighting_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 09.02.2017

Description: Compute shader for deferred rendering lighting pass.
---------------------------------------------------------------------------------------------------*/

#version 450

#define PI 3.1415926535
#define INV_PI 0.318309886183

struct LightProperties 
{
	vec3 position;
	vec3 lumen;
};

struct SunlightProperties
{
    vec3 position;
    float illuminance; //(lux)
};

layout(rgba32f, binding = 0) writeonly uniform image2D lighting_tx2D;

layout(location = 0) uniform sampler2D normal_tx2D;
layout(location = 1) uniform sampler2D depth_tx2D;
layout(location = 2) uniform sampler2D albedoRGB_tx2D;
layout(location = 3) uniform sampler2D metalness_roughness_tx2D;
layout(location = 4) uniform samplerCubeArray pointlight_shadowmaps;

layout ( std430, binding = 0 ) buffer PointlightBuffer { LightProperties pointlights[]; };
layout ( std430, binding = 1 ) buffer SunlightBuffer { SunlightProperties sunlights[]; };

uniform int num_pointlights;
uniform int num_suns;

uniform mat4 view_matrix;
uniform vec2 aspect_fovy;
uniform vec2 screen_resolution;
uniform float exposure;

// Taken from learnopengl.com
vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
); 

float shadowCalculation(in vec3 frag_pos, int idx)
{
	vec3 light_pos = pointlights[idx].position;
	vec3 direction = frag_pos - light_pos;
	float frag_light_dist = length(direction);
	
	float shadow = 0.0;
	float bias = 0.05;
	float far_plane = 2500.0;//TODO uniform for far plane
	int samples = 20;
	float diskRadius = 0.001;

	for(int i = 0; i < samples; ++i)
	{
	    float closest_depth = texture(pointlight_shadowmaps, vec4( (normalize(direction) + sampleOffsetDirections[i] * diskRadius), idx)).r;
	    closest_depth *= far_plane;   // Undo mapping [0;1]
	    if(frag_light_dist - bias > closest_depth)
	        shadow += 1.0;
	}
	shadow /= float(samples);

	return shadow;
}

//layout (location = 0) out vec4 frag_colour;

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
	vec3 fresnel_term = mix( surface_specular_color,vec3(1.0), pow(1.01-l_dot_h,5.0) );
	
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

	//return 10000.0 * (1.0-fresnel_term);
	//return light_colour*specular_brdf * max(0.0,n_dot_l);
    return (light_colour*diffuse_brdf + light_colour*specular_brdf) * max(0.0,n_dot_l);
}

void lightPixel(in ivec2 pixel_coords)
{
	float depth = texelFetch(depth_tx2D,pixel_coords,0).x;

	//early exit if empty pixel
	if( !(depth > 0.0) )
	{
		imageStore(lighting_tx2D,pixel_coords,vec4(0.0));
		return;
	}

	vec3 normal = (texelFetch(normal_tx2D,pixel_coords,0).xyz) * 2.0 - 1.0;
	vec3 albedo = texelFetch(albedoRGB_tx2D,pixel_coords,0).rgb;
	vec2 metallicRoughness = texelFetch(metalness_roughness_tx2D,pixel_coords,0).bg;

	vec2 normalized_pixel_coords = pixel_coords / screen_resolution;

	vec3 position = normalize( vec3( vec2(tan(aspect_fovy.y/2.0) * aspect_fovy.x,tan(aspect_fovy.y/2.0)) * ((normalized_pixel_coords*2.0)-1.0),-1.0) ) * depth;
	position = (inverse(view_matrix) * vec4(position,1.0)).xyz;

	vec3 specular_color = (albedo * metallicRoughness.r) + ((1.0 - metallicRoughness.r) * vec3(0.04));
	vec3 albedo_color = albedo * (1.0 - metallicRoughness.r);
	float roughness = metallicRoughness.g;

	// Calculate and sum up Cook Torrance shading for each light source
	vec3 rgb_linear = vec3(0.0);

	vec3 camera_world = (inverse(view_matrix) * vec4(0.0,0.0,0.0,1.0)).xyz;
    //vec3 viewer_direction = normalize(-position);
	vec3 viewer_direction = normalize(camera_world-position);
	
    for(int i=0; i<num_pointlights; i++)
    {
        // after this calculation, position contains a direction
        //lights_view_space.position = normalize( (view_matrix * vec4(lights[i].position,1.0)).xyz - position);
		vec3 light_direction = normalize(pointlights[i].position - position);
        
        // Calculate incident light intensity in Luminance (cd/m^2) for point lights.
        // Base light intensity is given in Lumen (lm).
        float attenuation = pow(length(pointlights[i].position - position),2.0);
        vec3 light_intensity = pointlights[i].lumen / (4.0 * PI * attenuation);

		//float shadow = shadowCalculation(position,i);

		//rgb_linear += shadow * 1000.0;

		// view dependent roughness experiment
		//float vd_roughness = roughness * (1.0 - pow(1.0 - dot(normal,viewer_direction),2.0));

	    //rgb_linear += (1.0-shadow) * cookTorranceShading(
		//	albedo_color,
	    //	specular_color,
	    //	roughness,
	    //	normal,
	    //	light_direction,
	    //	viewer_direction,
	    //	light_intensity
		//);

		rgb_linear += cookTorranceShading(
			albedo_color,
	    	specular_color,
	    	roughness,
	    	normal,
	    	light_direction,
	    	viewer_direction,
	    	light_intensity
		);
											
    }
    
	
    for(int i=0; i<num_suns; i++)
    {
        vec3 sunlight_intensity = vec3(sunlights[i].illuminance);
        
		//TODO for now, treat as point light...
		float attenuation = pow(length(sunlights[i].position - position),2.0);
        sunlight_intensity = vec3(sunlights[i].illuminance) / (4.0 * PI * attenuation);

		vec3 sun_view_direction = normalize(sunlights[i].position - position);
		
        rgb_linear += cookTorranceShading(
			albedo_color,
	  		specular_color,
	  		roughness,
	  		normal,
	  		sun_view_direction,
	  		viewer_direction,
	  		sunlight_intensity
		);

    }

	imageStore(lighting_tx2D,pixel_coords,vec4(rgb_linear,depth));

	//imageStore(lighting_tx2D,pixel_coords,vec4(normalize(sunlights[0].position - position),depth));
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

void main()
{
	vec3 gID = gl_GlobalInvocationID.xyz;

	if(gID.x >= screen_resolution.x || gID.y >= screen_resolution.y)
		return;

	ivec2 base_pixel_coords = ivec2(gID.xy);

	lightPixel(base_pixel_coords);
}