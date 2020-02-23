#version 450
#extension GL_ARB_bindless_texture : require

#define PI 3.1415926535
#define INV_PI 0.318309886183

struct PerDrawData
{
	mat4 model_matrix;

	uvec2 base_color_tx_hndl;
	uvec2 roughness_tx_hndl;
    uvec2 normal_tx_hndl;
};

layout(std430, binding = 0) readonly buffer PerDrawDataBuffer { PerDrawData per_draw_data[]; };

in vec3 position;
in vec2 uvCoord;
in mat3 tangent_space_matrix;

flat in int draw_id;

out vec4 frag_colour;

vec3 cookTorranceShading(
	in vec3 surface_albedo,
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

void main()
{
	sampler2D base_tx_hndl = sampler2D(per_draw_data[draw_id].base_color_tx_hndl);
	sampler2D roughness_tx_hndl = sampler2D(per_draw_data[draw_id].roughness_tx_hndl);
    sampler2D normal_tx_hndl = sampler2D(per_draw_data[draw_id].normal_tx_hndl);

	vec3 base_color = texture(base_tx_hndl, uvCoord).rgb;
    vec3 normal = texture(normal_tx_hndl, uvCoord).rgb;
	normal = normalize( transpose(tangent_space_matrix) * ((normal*2.0)-1.0) );
	vec2 metallicRoughness = texture(roughness_tx_hndl, uvCoord).bg;

	vec3 specular_color = base_color * metallicRoughness.r;
	vec3 albedo_color = base_color * (1.0 - metallicRoughness.r);

	vec3 lighting = cookTorranceShading(
		albedo_color,
		specular_color,
		metallicRoughness.g,
		normal,
		normalize(vec3(1.0,0.0,1.0)),
		vec3(0.0,0.0,1.0),
		vec3(1.0,1.0,1.0)
	);

    //frag_colour = vec4(uvCoord,0.0,1.0);
    frag_colour = vec4(lighting,1.0);
}