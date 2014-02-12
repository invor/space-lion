#version 430

#define M_PI 3.1415926535897932384626433832795

uniform mat4 view_mx;

uniform sampler2D scene_depth_tx2D;
uniform sampler3D rayleigh_inscatter_tx3D;
uniform sampler3D mie_inscatter_tx3D;

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

float rayleightPhaseFunction(in float scattering_angle)
{
	return 0.8*( (7.0/5.0) + 0.5*cos(scattering_angle));
}

float miePhaseFunction(in float scattering_angle, in float g)
{
	return ( (3.0*(1.0-square(g))) / (2.0*(2.0+square(g))) ) * ( (1.0 + square(cos(scattering_angle))) / pow(1.0+square(g)-2.0*g*cos(scattering_angle),1.5) );
}

void main()
{
	vec2 fragment_view_coords = (uvCoord * 2.0) - 1.0;
	fragment_view_coords.y = fragment_view_coords.y * tan(fov_y*M_PI/360.0);
	fragment_view_coords.x = fragment_view_coords.x * tan(fov_y*M_PI/360.0) * aspect_ratio;
	vec3 camera_direction = normalize((inverse(view_mx) * vec4(fragment_view_coords,-1.0,1.0)).xyz - camera_position);
	Ray view_ray;
	view_ray.origin = camera_position;
	view_ray.direction = camera_direction;
	
	vec2 intersections = vec2(0.0);
	//if(!intersectAtmosphere(view_ray, intersections)) discard;
	
	/*	check for two positive intersections -> camera outside atmosphere */
	if( (intersections.x*intersections.y) > 0.0)
	view_ray.origin = min(view_ray.origin + view_ray.direction * intersections.x,
							view_ray.origin + view_ray.direction * intersections.y);
	
	float altitude = length(view_ray.origin - planet_center);
	float cosViewZenithAngle = dot( normalize(view_ray.direction), normalize(view_ray.origin - planet_center) );
	float cosSunZenithAngle = dot( normalize(sun_direction), normalize(view_ray.origin - planet_center) );
	float cosViewSunAngle = dot( normalize(view_ray.direction), normalize(sun_direction) );
	
	vec3 rayleigh = texture(rayleigh_inscatter_tx3D,vec3(uvCoord,0.0)).xyz;
	vec3 mie = texture(mie_inscatter_tx3D,vec3(uvCoord,0.0)).xyz;

	//float scattering_angle = 
	
	frag_colour = vec4(rayleigh+mie,1.0);
}