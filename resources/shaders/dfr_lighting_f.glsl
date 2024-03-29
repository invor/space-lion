/*---------------------------------------------------------------------------------------------------
File: dfr_lighting_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 30.10.2015

Description: Fragment shader for deferred rendering lighting pass.
---------------------------------------------------------------------------------------------------*/

#version 430

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

uniform sampler2D normal_depth_tx2D;
uniform sampler2D albedoRGB_tx2D;
uniform sampler2D specularRGB_roughness_tx2D;
uniform sampler2D atmosphereRGBA_tx2D;

uniform LightProperties lights[256];
uniform int num_lights;

uniform SunlightProperties suns[10];
uniform int num_suns;

uniform mat4 view_matrix;
uniform vec2 aspect_fovy;
uniform float exposure;

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
    
	return (light_colour*diffuse_brdf + light_colour*specular_brdf) * max(0.0,n_dot_l);
}

void main()
{
	/* Get values from gBuffer */
	vec4 normal_depth = texture(normal_depth_tx2D,uvCoord);
	vec3 albedo = texture(albedoRGB_tx2D,uvCoord).rgb;
	vec4 specular_roughness = texture(specularRGB_roughness_tx2D,uvCoord);
	
	float depth = (normal_depth.z + normal_depth.w)/1024.0;
	
	vec3 position = normalize( vec3( vec2(tan(aspect_fovy.y/2.0) * aspect_fovy.x,tan(aspect_fovy.y/2.0)) * ((uvCoord*2.0)-1.0),-1.0) ) * depth;
	
	vec3 normal = vec3( normal_depth.xy, sqrt(1.0 - dot(normal_depth.xy,normal_depth.xy)) );
	
	vec3 specular = specular_roughness.rgb;
	float roughness = specular_roughness.a;
	
	/*	Calculate Cook Torrance shading for each light source */
	vec3 rgb_linear = vec3(0.0);

	
	//vec3 light_intensity = lights_view_space.intensity;
	
	if(depth > 0.0)
	{
        vec3 viewer_direction = normalize(-position);
        LightProperties lights_view_space;
            
        for(int i=0; i<num_lights; i++)
        {
            lights_view_space.lumen = lights[i].lumen;
            // after this calculation, position contains a direction
            lights_view_space.position = normalize( (view_matrix * vec4(lights[i].position,1.0)).xyz - position);
            
            /* Calculate incident light intensity in Luminance (cd/m^2) for point lights.
             * Base light intensity is given in Lumen (lm).
             */
            float attenuation = pow(length(position-(view_matrix*vec4(lights[i].position,1.0)).xyz),2.0);
            vec3 light_intensity = lights_view_space.lumen / (4.0 * PI * attenuation);
            
            light_intensity = vec3(100000);
            
            
		    rgb_linear += cookTorranceShading(albedo,
		    									specular,
		    									roughness,
		    									normal,
		    									lights_view_space.position,
		    									viewer_direction,
		    									light_intensity);
        }
        
        for(int i=0; i<num_suns; i++)
        {
            vec3 sunlight_intensity = vec3(suns[i].illuminance);
            vec3 sun_view_direction = normalize( (view_matrix * vec4(suns[i].position,1.0)).xyz - position);
            
            rgb_linear += cookTorranceShading(albedo,
		  					specular,
		  					roughness,
		  					normal,
		  					sun_view_direction,
		  					viewer_direction,
		  					sunlight_intensity);
        }
	}
	
	rgb_linear += texture(atmosphereRGBA_tx2D,uvCoord).xyz; 
    
    /* Adjust exposure */
    rgb_linear *= exposure;
    //rgb_linear *= 0.18/4000.0; // default value for mapping avg luminance of ~4000cd/m^2 to 0.18 intensity
	
	/*	Temporary gamma correction */
	frag_colour = vec4( pow( rgb_linear, vec3(1.0/2.2) ), 1.0);
}