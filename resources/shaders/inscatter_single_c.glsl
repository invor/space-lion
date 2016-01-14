#version 430

#define M_PI 3.1415926535897932384626433832795
#define INSCATTER_INTEGRAL_SAMPLES 50
#define TRANSMITTANCE_INTEGRAL_SAMPLES 20

uniform sampler2D transmittance_tx2D;
layout(RGBA32F) uniform image3D rayleigh_inscatter_tx3D;
layout(RGBA32F) uniform image3D mie_inscatter_tx3D;

uniform float min_altitude;
uniform float max_altitude;
/*	extinction coefficient for Rayleigh scattering */
uniform vec3 beta_r;
/*	extinction coefficient for Mie scattering */
uniform vec3 beta_m;
uniform float h_r;
uniform float h_m;

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

/*
*	Helper function...GLSL's pow() has some nasty behaviour with negative numbers
*/
float square(in float base)
{
	return base*base;
}

/*
*	We are only interested in the distance value here, so assuming any position that
*	satisfies the given altitude condition will result in a feasible value.
*	Also, considering the geometric setup, we can compute the value in 2D
*/
float intersectAtmosphere(in float altitude, in float cosZenithAngle)
{
	/*	view ray parameters */
	vec2 pos = vec2(0.0f,altitude);
	vec2 dir = normalize(vec2( sqrt(1.0-square(cosZenithAngle)) , cosZenithAngle ));
	
	/* now solve linear equation for line circle intersection */
	float e = 1.0f; //epsilon value
	float a = dot(dir,dir);
	float b = dot(dir,pos);
	float c = dot(pos,pos) - pow(max_altitude+e,2.0f);
	
	/*
	*	usually you would have to check the discriminant of the sqrt for > 0 
	*	but in this special case, an intersection is guaranteed because altitude<max_altitude+e
	*/
	float d_1 = (-b + sqrt(square(b) - a*c))/a;
	float d_2 = (-b - sqrt(square(b) - a*c))/a;
	float d = max(d_1,d_2);
	
	return d;
}

/*
*	Compute transmittance - in case the texture lookup doesn't work. Has been used for debugging.
*/
float computeDensity(in float scaleHeight, in float altitude, in float cosZenithAngle) 
{
	/* if ray below horizon return max density */
	float cosHorizon = -sqrt(1.0f - ((min_altitude*min_altitude)/(altitude*altitude)));
	if(cosZenithAngle < cosHorizon)
		return 1e9;
	
	/*	step-size of the discrete integration */
	float dx = intersectAtmosphere(altitude,cosZenithAngle) / float(TRANSMITTANCE_INTEGRAL_SAMPLES);
	
	/*	rho value at beginning of interval */
	float rho_j = exp(-(altitude-min_altitude)/scaleHeight);
	
	float totalDensity = 0.0;
	
	for (int i = 1; i<=TRANSMITTANCE_INTEGRAL_SAMPLES; i++)
	{
		float d_i = float(i)*dx;
		
		vec2 p_i = vec2(0.0,altitude) + normalize(vec2( sqrt(1.0-square(cosZenithAngle)) , cosZenithAngle )) * d_i;
		
		float altitude_i = length(p_i);
		
		/*	rho value at end of interval */
		float rho_i = exp(-(altitude_i-min_altitude)/scaleHeight);
		
		/*	average and divide by step-size */
		totalDensity += (rho_i+rho_j)/2.0*dx;
		
		rho_j=rho_i;
	}
	
	return totalDensity;
}

/*
*	Fetch values from the transmittance table.
*/
vec3 fetchTransmittance(in float altitude, in float cosAngle)
{
	float altitude_param = sqrt((altitude-min_altitude)/(max_altitude-min_altitude));
	float angle_param = atan((cosAngle + 0.15) / (1.0 + 0.15) * tan(1.5)) / 1.5;
	//angle_param = 1.0 - ((cosAngle*0.5)+0.5);
	
	return texture(transmittance_tx2D,vec2(angle_param,altitude_param)).xyz;
	
	/*	direct evaluation - computationally expansive */
	//return exp(-(beta_r * computeDensity(h_r,altitude, cosAngle) +
	//							beta_m * computeDensity(h_m,altitude, cosAngle)));
}

/*
*	Compute the inscatter integral for a single scattering event.
*/
void computeSingleInscatter(in float altitude, in float viewZenith, in float sunZenith, in float viewSun, out vec3 rayleigh, out vec3 mie)
{
	vec3 rayleigh_integral = vec3(0.0);
	vec3 mie_integral = vec3(0.0);
	vec3 rayleigh_integrand_l = vec3(0.0);
	vec3 rayleigh_integrand_u  = vec3(0.0);
	vec3 mie_integrand_l = vec3(0.0);
	vec3 mie_integrand_u = vec3(0.0);
	
	if(sunZenith >= -sqrt(1.0 - square(min_altitude)/square(altitude)))
	{
		//vec3 transmittance =  exp(-fetchTransmittance(altitude,sunZenith));
		vec3 transmittance =  fetchTransmittance(altitude,sunZenith);
		rayleigh_integrand_l = exp(-((altitude-min_altitude)/h_r)) * transmittance;
		mie_integrand_l = exp(-((altitude-min_altitude)/h_m)) * transmittance;
	}
	
	/*	step-size of the discrete integration */
	float dx = intersectAtmosphere(altitude,viewZenith) / float(INSCATTER_INTEGRAL_SAMPLES);
	
	/*	compute light direction */
	//vec2 light_dir = normalize(vec2( sqrt(1.0-square(sunZenith)) , sunZenith ));
	/*	flip to the other side if necessary */
	//if(acos(viewSun) > acos(viewZenith) && acos(sunZenith) < acos(viewZenith))
	//	light_dir.x *= -1.0;

	for (int i = 1; i<=INSCATTER_INTEGRAL_SAMPLES; i++)
	{
		float d_i = float(i)*dx;
		
		vec2 p_i = vec2(0.0,altitude) + normalize(vec2( sqrt(1.0-square(viewZenith)) , viewZenith )) * d_i;
		
		//float sunZenith_i = dot(light_dir,normalize(p_i));
		float altitude_i = length(p_i);
		altitude_i = sqrt(square(altitude) + square(d_i) + 2.0 * altitude * viewZenith * d_i);
		float sunZenith_i = (viewSun * d_i + sunZenith * altitude) / altitude_i;
		altitude_i = max(min_altitude,altitude_i);
		
		if(sunZenith_i >= -sqrt(1.0 - square(min_altitude)/square(altitude_i)))
		{
			vec3 t_sampleToSun = fetchTransmittance(altitude_i,sunZenith_i);
			
			float l = length(vec2(0.0,p_i.y));
			float viewZenith_i = (altitude * viewZenith + d_i) / altitude_i;
			
			vec3 t_camToSample = fetchTransmittance(altitude,viewZenith) / fetchTransmittance(altitude_i,viewZenith_i);
			//rayleigh_integrand_u = exp(-((altitude_i-min_altitude)/h_r)) * exp(-t_sampleToSun - t_camToSample);
			//mie_integrand_u = exp(-((altitude_i-min_altitude)/h_m)) * exp(-t_sampleToSun - t_camToSample);
			rayleigh_integrand_u = exp(-(altitude_i-min_altitude)/h_r) * t_camToSample * t_sampleToSun;
			mie_integrand_u = exp(-(altitude_i-min_altitude)/h_m) * t_camToSample * t_sampleToSun;
		}
		else
		{
			rayleigh_integrand_u = vec3(0.0);
			mie_integrand_u = vec3(0.0);
		}
		
		rayleigh_integral += (rayleigh_integrand_l+rayleigh_integrand_u)/2.0*dx;
		mie_integral += (mie_integrand_l+mie_integrand_u)/2.0*dx;
		
		rayleigh_integrand_l = rayleigh_integrand_u;
		mie_integrand_l = mie_integrand_u;
	}
	
	rayleigh = rayleigh_integral * beta_r;
	mie = mie_integral * beta_m;
}

/*
*	Non-linear parametrization for the inscatter table as proposed by Bruneton and Neyret (2008).
*	Uses quite a lot of ad-hoc functions/values....
*	Some tweaks were made to get things working.
*/
void computeNonLinearParams(out float altitude, out float cosViewZenith, out float cosSunZenith, out float cosViewSun)
{
	float t_res = gl_NumWorkGroups.y;
	float s_res = gl_NumWorkGroups.x;

	float s = float(gl_WorkGroupID.x)/float(gl_NumWorkGroups.x-1);;
	float s_2 = float(gl_LocalInvocationID.x)/float(gl_WorkGroupSize.x-1);
	float t = float(gl_GlobalInvocationID.y);
	float r = float(gl_GlobalInvocationID.z)/float(gl_NumWorkGroups.z-1);

	altitude = r * r;
	altitude = sqrt( square(min_altitude) + altitude * (square(max_altitude)-square(min_altitude)))
					+ (gl_GlobalInvocationID.z == 0 ? 0.01 : (gl_GlobalInvocationID.z == gl_NumWorkGroups.z-1 ? -0.001 : 0.0));
	
	float dmin = max_altitude - altitude;
    float dmax = sqrt(square(altitude) - square(min_altitude)) + sqrt(square(max_altitude) - square(min_altitude));
    float dminp = altitude - min_altitude;
    float dmaxp = sqrt(square(altitude) - square(min_altitude));
	
	if (t < (t_res / 2.0))
	{
        float d = 1.0 - t / (t_res / 2.0 - 1.0);
        d = min(max(dminp, d * dmaxp), dmaxp * 0.999);
        cosViewZenith = (square(min_altitude) - square(altitude) - square(d)) / (2.0 * altitude * d);
		cosViewZenith = min(cosViewZenith, -sqrt(1.0 - square(min_altitude / altitude)) - 0.001);
    }
	else
	{
        float d = (t - t_res / 2.0) / (t_res / 2.0 - 1.0);
        d = min(max(dmin, d * dmax), dmax * 0.999);
        cosViewZenith = (square(max_altitude) - square(altitude) - d * d) / (2.0 * altitude * d);
    }
	/*	for now, use a simpler mapping (as proposed by Elek) */
	cosViewZenith = 2.0*(t/(t_res-1.0))-1.0;
	
	// paper formula
    cosSunZenith = -(0.6 + log(1.0 - s * (1.0 -  exp(-3.6)))) / 3.0;
    // better formula
    //cosSunZenith = tan((2.0 * s - 1.0 + 0.26) * 1.1) / tan(1.26 * 1.1);
	
    cosViewSun = -1.0 + s_2 * 2.0;
}

void computeLinearParams(out float altitude, out float cosViewZenith, out float cosSunZenith, out float cosViewSun)
{
	float s = float(gl_WorkGroupID.x)/float(gl_NumWorkGroups.x-1);
	float s_2 = float(gl_LocalInvocationID.x)/float(gl_WorkGroupSize.x-1);
	float t = float(gl_GlobalInvocationID.y)/float(gl_NumWorkGroups.y-1);
	float r = float(gl_GlobalInvocationID.z)/float(gl_NumWorkGroups.z-1);
	
	altitude = mix(min_altitude,max_altitude,r);
	cosViewZenith = mix(-1.0,1.0,t);
	cosSunZenith = mix(-1.0,1.0,s);
	cosViewSun = mix(-1.0,1.0,s_2);
}

void main()
{
	ivec3 global_pos = ivec3(gl_GlobalInvocationID.xyz);
	ivec3 local_pos = ivec3(gl_LocalInvocationID.xyz);
	ivec3 store_pos = ivec3( global_pos.x,
											global_pos.y, global_pos.z );
	
	float s = float(gl_WorkGroupID.x)/float(gl_NumWorkGroups.x-1);
	float s_2 = float(local_pos.x)/float(gl_WorkGroupSize.x-1);
	float t = float(global_pos.y)/float(gl_NumWorkGroups.y-1);
	float r = float(global_pos.z)/float(gl_NumWorkGroups.z-1);
	
	//	float altitude = mix(min_altitude,max_altitude,r);
	//	float viewZenithAngle = mix(0.0,M_PI,s);
	//	float sunZenithAngle = mix(0.0,M_PI,t);
	//	float viewSunAngle = mix(0.0,M_PI,0.0);
	
	float viewSun = 0.0;
	float viewZenith = 0.0;
	float sunZenith = 0.0;
	float altitude = 0.0;
	//computeLinearParams(altitude,viewZenith,sunZenith,viewSun);
	computeNonLinearParams(altitude,viewZenith,sunZenith,viewSun);

	vec3 rgb_rayleigh = vec3(0.0);
	vec3 rgb_mie = vec3(0.0);
	computeSingleInscatter(altitude,viewZenith,sunZenith,viewSun,rgb_rayleigh,rgb_mie);
	
	/*	debugging */
	//rgb_rayleigh = abs(fetchTransmittance(altitude,viewZenith));
	
	imageStore(rayleigh_inscatter_tx3D,store_pos,vec4(rgb_rayleigh,1.0));
	imageStore(mie_inscatter_tx3D,store_pos,vec4(rgb_mie,1.0));
	
	//imageStore(mie_inscatter_tx3D,store_pos,vec4(0.0,abs(viewZenith),0.0,1.0));
	//imageStore(mie_inscatter_tx3D,store_pos,vec4(viewSun,0.0,0.0,1.0));
}