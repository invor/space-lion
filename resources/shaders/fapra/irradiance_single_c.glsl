#version 430

#define M_PI 3.1415926535897932384626433832795

layout(RGBA32F) uniform image2D irradiance_tx2D;
uniform vec3 sun_direction;
uniform float min_altitude;
uniform float max_altitude;
/*	extinction coefficient for Rayleight scattering */
uniform vec3 beta_r;
/*	extinction coefficient for Mie scattering */
uniform vec3 beta_m;
uniform float h_r;
uniform float h_m;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;


/*
*	We are only interested in the distance value here, so assuming any position that
*	satisfies the given altitude condition will result in a feasible value.
*	Also, considering the geometric setup, we can compute the value in 2D
*/
float intersectAtmosphere(in float altitude, in float angle)
{
	/*	view ray parameters */
	vec2 pos = vec2(0.0f,altitude);
	vec2 dir = normalize(mix(vec2(1.0f,0.0f),vec2(0.0f,1.0f),angle));
	
	/* now solve linear equation for line circle intersection */
	float e = 1.0f; //epsilon value
	float a = dot(dir,dir);
	float b = dot(dir,pos);
	float c = dot(pos,pos) - pow(max_altitude+e,2.0f);
	
	/*
	*	usually you would have to check the discriminant of the sqrt for > 0 
	*	but in this special case, an intersection is guaranteed because altitude<max_altitude+e
	*/
	float d_1 = (-b + sqrt(pow(b,2.0f) - a*c))/a;
	float d_2 = (-b - sqrt(pow(b,2.0f) - a*c))/a;
	float d = max(d_1,d_2);
	
	return d;
}

vec3 computeSingleInscatter()
{
	return vec3(0.0);
}

void main()
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	
	float s = float(storePos.x)/float(gl_NumWorkGroups.x);
	float t = float(storePos.y)/float(gl_NumWorkGroups.y);
	
	imageStore(irradiance_tx2D,storePos,vec4(1.0));
}