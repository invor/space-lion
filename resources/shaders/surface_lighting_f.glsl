/*---------------------------------------------------------------------------------------------------
FFile: surface_lighting_f.glsl
Author: Michael Becher
Date of (presumingly) last edit: 26.08.2013

Description: Standard fragment shader for lighting calculations of opaque surfaces.
			 Based on the Cook-Torrance BRDF.
---------------------------------------------------------------------------------------------------*/
#version 330

#define PI 3.1415926535

struct LightProperties 
{
	vec3 position[5];
	vec3 intensity[5];
};

uniform sampler2D diffuse_tx2D;
uniform sampler2D specular_tx2D;
uniform sampler2D roughness_tx2D;
uniform sampler2D normal_tx2D;

uniform LightProperties lights;
uniform int num_lights;

uniform mat4 view_matrix;

in vec3 position;
in vec4 colour;
in vec2 uv_coord;
in vec3 viewer_direction;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in LightProperties lights_tangent_space;

out vec4 fragColour;

vec3 phongShading(in float specFactor ,in vec3 sColour , in vec3 sNormal, in vec3 lightDir, in vec4 light_colour)
{
	vec3 n = normalize(sNormal);
	vec3 reflection = reflect(-normalize(lightDir), n);

	return light_colour.w *
		( sColour*max( dot(normalize(lightDir), n), 0.0)) +
			(specFactor*light_colour.xyz*pow(max(dot(reflection,normalize(viewer_direction)),0.0),55.0) );
}

vec3 cookTorranceShading(in vec3 surface_albedo, in vec3 surface_specular_color, in float surface_roughness,
							in vec3 surface_normal, in vec3 light_direction, in vec3 viewer_direction, in vec3 light_colour)
{
	vec3 halfway = normalize(light_direction + viewer_direction);
	float n_dot_h = dot(surface_normal,halfway);
	float n_dot_l = dot(surface_normal,light_direction);
	float n_dot_v = dot(surface_normal,viewer_direction);
	float v_dot_h = dot(viewer_direction,halfway);
	float l_dot_h = v_dot_h;
	
	/*
	/	Compute Fresnel term using the Schlick approximation.
	/	To avoid artifact, a small epsilon is added to 1.0-l_dot_h
	*/
	vec3 fresnel_term = mix(surface_specular_color,vec3(1.0), pow(1.01-l_dot_h,5.0) );
	
	/*	
	/	Compute geometric attenuation, based on Smith shadowing term and following 
	/	"Crafting a Next-Gen Material Pipeline for The Order: 1886" Euqation 5.
	*/
	float g_1 = (2.0*n_dot_v) / (n_dot_v + sqrt(1.0 + pow(surface_roughness,2.0)*(1.0 - pow(n_dot_v,2.0))));
	float g_2 = (2.0*n_dot_l) / (n_dot_l + sqrt(1.0 + pow(surface_roughness,2.0)*(1.0 - pow(n_dot_l,2.0))));
	float geometry_term = g_1*g_2;
	//	float geometry_term = min( min(1.0, (2.0*n_dot_h*n_dot_v)/v_dot_h) , (2.0*n_dot_h*n_dot_l)/v_dot_h );
	
	/*	Compute microfacet normal distrubution term using GGX distribution by Walter et al (2007) */
	float distribution_term = pow(surface_roughness,2.0)/
								(PI*pow(pow(n_dot_h,2.0)*(pow(surface_roughness,2.0)-1.0)+1.0 , 2.0));
	//	/*	Beckman distribution */
	//	float distribution_term = exp( -(1.0-pow(n_dot_h,2.0))/(pow(surface_roughness,2.0)*pow(n_dot_h,2.0)) )/
	//								(PI*pow(surface_roughness,2.0)*pow(n_dot_h,4.0));
								
	/*	Compute Cook Torrance BRDF */
	vec3 specular_brdf = (fresnel_term*geometry_term*distribution_term)/(4.0*n_dot_l*n_dot_v);
	
	/*
	/	Compute diffuse lambertian BRDF.
	/	The specular reflection takes away some energy from the diffuse reflection.
	/	Only the fresnel term is considered, as to not include specular reflected light "blocked" 
	/	by the geometry or distribution term in the diffuse energy.
	/	To avoid tampering of the color, the mean of all three color channels is considered.
	*/
	float used_up_energy = (fresnel_term.x+fresnel_term.y+fresnel_term.z)/3.0;
	vec3 diffuse_brdf = (1.0-used_up_energy)*(surface_albedo/PI);

	vec3 surfance_reflectance = light_colour * (diffuse_brdf+specular_brdf) * max(0.0,n_dot_l);

	return surfance_reflectance;
}

void main()
{
	fragColour = vec4(1.0);
	vec3 tLightDirection;

	/*	Fetch colour from diffuse map and blend it with vertex colour */
	vec3 tColour = texture(diffuse_tx2D, uv_coord).xyz;
	//tColour = mix(tColour,colour.xyz,colour.w);

	/*	Fetch specular color from specular map */
	vec3 tSpecColour = texture(specular_tx2D, uv_coord).xyz;
	
	/*	Fetch roughness from roughness map */
	float tRoughness = texture(roughness_tx2D, uv_coord).x;

	/*	Fetch normal vector from normal map */
	vec3 tNormal = ((texture(normal_tx2D, uv_coord).xyz)*2.0)-1.0;
	//vec3 tNormal = texture2D(normal_tx2D, uv_coord).xyz;

	/*	Calculate Cook Torrance shading for each light source */
	vec3 rgb_linear = vec3(0.0);
	for(int i=0;i<num_lights;i++)
	{
		/*	Quick&Dirty light attenuation */
		vec3 light_intensity = 100.0 * lights_tangent_space.intensity[i] / pow(length(position-(view_matrix*vec4(lights.position[i],1.0)).xyz),2.0);
		
		rgb_linear += cookTorranceShading(tColour,tSpecColour,tRoughness,
											tNormal, lights_tangent_space.position[i], viewer_direction, light_intensity);
	}
	
	/*	Temporary gamma correction */
	fragColour = vec4( pow( rgb_linear, vec3(1.0/2.2) ), 1.0);
	
	/*	Let's do some debugging */
	//fragColour = vec4(vec3(tSpecFactor),1.0);
}