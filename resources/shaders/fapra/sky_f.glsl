#version 430

#define M_PI 3.1415926535897932384626433832795
#define INV_PI 0.318309886183

uniform mat4 view_mx;

uniform sampler3D rayleigh_inscatter_tx3D;
uniform sampler3D mie_inscatter_tx3D;
uniform sampler2D scene_diffuse_albedo_tx2D;
uniform sampler2D scene_normal_tx2D;
uniform sampler2D scene_tangent_bitangent_tx2D;
uniform sampler2D scene_specular_roughness_tx2D;
uniform sampler2D scene_linear_depth_tx2D;

uniform vec3 camera_position;
uniform float fov_y;
uniform float aspect_ratio;

uniform float min_altitude;
uniform float max_altitude;
uniform vec3 planet_center;
uniform vec3 sun_direction;

in vec2 uvCoord;

out vec4 frag_colour;

struct Ray
{
	vec3 origin;
	vec3 direction;
};

float square(in float base)
{
	return base*base;
}

bool intersectAtmosphere(in Ray ray,in out vec2 d)
{

	/*	view ray parameters */
	vec3 pos = ray.origin - planet_center;
	vec3 dir = ray.direction;
	
	/* now solve linear equation for line circle intersection */
	float e = 1.0f; //epsilon value
	float a = dot(dir,dir);
	float b = dot(pos,dir);
	float c = dot(pos,pos) - square(max_altitude+e);
	
	
	float diskr = square(b) - a*c;
	
	if(diskr < 0.0) return false;
	
	d.x = (-b + sqrt(diskr))/a;
	d.y = (-b - sqrt(diskr))/a;
	
	return true;
}

float rayleighPhaseFunction(in float scattering_angle)
{
	return (3.0 / (16.0 * M_PI)) * (1.0 + square(scattering_angle));
	//return 0.8*( (7.0/5.0) + 0.5*cos(scattering_angle));
}

float miePhaseFunction(in float scattering_angle, in float mieG)
{
	return 1.5 * 1.0 / (4.0 * M_PI) * (1.0 - mieG*mieG) * pow(1.0 + (mieG*mieG) - 2.0*mieG*scattering_angle, -3.0/2.0) * (1.0 + square(scattering_angle)) / (2.0 + mieG*mieG);
	//return ( (3.0*(1.0-square(g))) / (2.0*(2.0+square(g))) ) * ( (1.0 + square(cos(scattering_angle))) / pow(1.0+square(g)-2.0*g*cos(scattering_angle),1.5) );
}

vec4 accessInscatterTexture(sampler3D inscatter_tx3D, float altitude, float viewZenith, float sunZenith, float viewSun)
{
    float H = sqrt(square(max_altitude) - square(min_altitude));
    float rho = sqrt(square(altitude) - square(min_altitude));
	
	float rmu = altitude * viewZenith;
    float delta = square(rmu) - square(altitude) + square(min_altitude);
	
	float res_mu = 128.0;
	float res_mu_s = 32.0;
	float res_r = 32.0;
	float res_nu = 8.0;
	
    vec4 cst = rmu < 0.0 && delta > 0.0 ? vec4(1.0, 0.0, 0.0, 0.5 - 0.5 / res_mu) : vec4(-1.0, H * H, H, 0.5 + 0.5 / res_mu);
	float uR = 0.5 / res_r + rho / H * (1.0 - 1.0 / res_r);
    //float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / res_mu);
	float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / res_mu);
	/*	for now, use a simpler mapping (as proposed by Elek) */
	uMu = (1.0 + viewZenith)/2.0;
	 // paper formula
    float uMuS = 0.5 / res_mu_s + max((1.0 - exp(-3.0 * sunZenith - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / res_mu_s);
    // better formula
    //float uMuS = 0.5 / res_mu_s + (atan(max(sunZenith, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / res_mu_s);
	
    //float lerp = (viewSun + 1.0) / 2.0 * (res_nu - 1.0);
    //float uNu = floor(lerp);
    //lerp = lerp - uNu;
	
	//making my own calculation for the x access coordinate...I don't trust the orig
	float x_coord = max((1.0 - exp(-3.0 * sunZenith - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / res_mu_s);
	float lerp = mod(x_coord,1.0 / res_mu_s);
	x_coord += viewSun * (1.0/res_mu_s);
	float x_coord_l = x_coord - lerp;
	float x_coord_u = x_coord + ((1.0 / res_mu_s)-lerp);
	lerp /= (1.0/res_mu_s);
	return texture3D(inscatter_tx3D, vec3(x_coord_l, uMu, uR)) * (1.0 - lerp) +
					texture3D(inscatter_tx3D, vec3(x_coord_u, uMu, uR)) * lerp;
	
	//return texture3D(inscatter_tx3D, vec3((uNu + uMuS) / res_nu, uMu, uR)) * (1.0 - lerp) +
    //       texture3D(inscatter_tx3D, vec3((uNu + uMuS + 1.0) / res_nu, uMu, uR)) * lerp;
}

vec3 computeSkyColour(float viewSun, float altitude, float viewZenith, float sunZenith)
{
	vec3 rgb_sky = vec3(0.0);
	if(altitude > min_altitude+0.001)
	{
		rgb_sky = rayleighPhaseFunction(viewSun) * accessInscatterTexture(rayleigh_inscatter_tx3D,altitude,viewZenith,sunZenith,viewSun).xyz;
		rgb_sky += miePhaseFunction(viewSun,0.8) * accessInscatterTexture(mie_inscatter_tx3D,altitude,viewZenith,sunZenith,viewSun).xyz;
		rgb_sky /= 4.0*M_PI;
		rgb_sky *= 100.0;
	}
	
	return max(rgb_sky,vec3(0.0));
}

vec3 cookTorranceShading(in vec3 surface_albedo, in vec3 surface_specular_color, in float surface_roughness,
							in vec3 surface_normal, in vec3 light_direction, in vec3 viewer_direction, in vec3 light_colour)
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
								(M_PI*pow(pow(n_dot_h,2.0)*roughness_squared-pow(n_dot_h,2.0)+1.0 , 2.0));
								
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
	/*	Generate view ray */
	vec2 fragment_view_coords = (uvCoord * 2.0) - 1.0;
	fragment_view_coords.y = fragment_view_coords.y * tan(fov_y*M_PI/360.0);
	fragment_view_coords.x = fragment_view_coords.x * tan(fov_y*M_PI/360.0) * aspect_ratio;
	vec3 camera_direction = normalize((inverse(view_mx) * vec4(fragment_view_coords,-1.0,1.0)).xyz - camera_position);
	Ray view_ray;
	view_ray.origin = camera_position;
	view_ray.direction = camera_direction;
	
	vec2 intersections = vec2(0.0);
	if(!intersectAtmosphere(view_ray, intersections)) discard;
	if(intersections.x<0.0 && intersections.y<0.0) discard;
	
	/*	check for two positive intersections -> camera outside atmosphere */
	if( intersections.x>0.0 && intersections.y>0.0)
	view_ray.origin = view_ray.origin + view_ray.direction * min(intersections.x,intersections.y);
	
	float altitude = length(view_ray.origin - planet_center);
	float viewZenith = dot( normalize(view_ray.direction), normalize(view_ray.origin - planet_center) );
	float sunZenith = dot( normalize(sun_direction), normalize(view_ray.origin - planet_center) );
	float viewSun = dot( normalize(view_ray.direction), normalize(sun_direction) );

	vec3 rgb_out = computeSkyColour(viewSun,altitude,viewZenith,sunZenith);
	
	float t_depth = texture(scene_linear_depth_tx2D,uvCoord).x;
	if(t_depth > 0.0)
	{
		vec3 t_colour = texture(scene_diffuse_albedo_tx2D,uvCoord).xyz;
		vec3 t_normal = (texture(scene_normal_tx2D,uvCoord).xyz*2.0)-1.0;
		vec4 t_specular_roughness = texture(scene_specular_roughness_tx2D,uvCoord);
		vec4 t_tangent_bitangent = texture(scene_tangent_bitangent_tx2D,uvCoord);
		
		mat3 normal_matrix =  transpose(inverse(mat3(view_mx)));
		
		vec3 tangent = normalize(vec3(t_tangent_bitangent.xy,1.0-length(t_tangent_bitangent.xy)));
		vec3 bitangent = normalize(vec3(t_tangent_bitangent.zw,1.0-length(t_tangent_bitangent.zw)));
		vec3 normal = normalize(cross(tangent,bitangent));
		
		mat3 tangent_space_matrix = mat3(
			tangent.x, bitangent.x, normal.x,
			tangent.y, bitangent.y, normal.y,
			tangent.z, bitangent.z, normal.z);
			
		/*	compute light direction */
		vec3 light_dir = normalize(sun_direction);
		vec3 viewer_dir = -normalize(camera_direction);
		
		rgb_out = cookTorranceShading(t_colour,t_specular_roughness.xyz,t_specular_roughness.w,
										normal,light_dir, viewer_dir, vec3(1.0));
	}
	
	frag_colour = vec4(rgb_out,1.0);
}