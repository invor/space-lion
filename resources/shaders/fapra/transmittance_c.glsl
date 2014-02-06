#version 430

#define M_PI 3.1415926535897932384626433832795
#define TRANSMITTANCE_INTEGRAL_SAMPLES 200

layout(RGBA32F) uniform image2D transmittance_tx2D;
uniform float min_altitude;
uniform float max_altitude;
/*	extinction coefficient for Rayleight scattering */
uniform vec3 beta_r;
/*	extinction coefficient for Mie scattering */
uniform vec3 beta_m;
uniform float h_r;
uniform float h_m;

layout(local_size_x = 1, local_size_y = 1) in;


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


//	float computeDensity(in float scaleHeight, in float altitude, in float mu) 
//	{
//		// if ray below horizon return max density
//		float cosHorizon = -sqrt(1.0f - ((min_altitude*min_altitude)/(altitude*altitude)));
//		if(mu < cosHorizon)
//			return 1e9;
//		
//		float totalDensity = 0.0f;
//		float dx = intersectAtmosphere(altitude,mu) / float(TRANSMITTANCE_INTEGRAL_SAMPLES);
//		
//		float y_j = exp(-(altitude-min_altitude)/scaleHeight);
//		
//		for (int i = 1; i<=TRANSMITTANCE_INTEGRAL_SAMPLES; i++)
//		{
//			float x_i = float(i)*dx;
//			float altitude_i = sqrt(altitude*altitude + x_i*x_i + 2.0f*x_i*altitude*mu);
//			float y_i = exp(-(altitude_i-min_altitude)/scaleHeight);
//			totalDensity += (y_j+y_i)/2.0f*dx;
//			y_j = y_i;
//		}
//		
//		return totalDensity;
//	}


float computeDensity(in float scaleHeight, in float altitude, in float cosZenithAngle) 
{
	// if ray below horizon return max density
	//float cosHorizon = -sqrt(1.0f - ((min_altitude*min_altitude)/(altitude*altitude)));
	//if(cosZenithAngle < cosHorizon)
	//	return 1e9;
	
	/*	step-size of the discretized integration */
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


void main()
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	
	float s = float(storePos.x)/float(gl_NumWorkGroups.x);
	float t = float(storePos.y)/float(gl_NumWorkGroups.y);
	
	float angle = mix(0.0f,M_PI,s);
	float altitude = mix(max_altitude,min_altitude,t);
	
	vec3 ext_factor = beta_r * computeDensity(h_r,altitude, cos(angle) ) +
								beta_m * computeDensity(h_m,altitude, cos(angle) );
	
	imageStore(transmittance_tx2D,storePos,vec4( ext_factor ,1.0));
}