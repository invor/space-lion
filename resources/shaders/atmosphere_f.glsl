#version 430

#define PI 3.141592653589

struct SunlightProperties
{
    vec3 sun_direction;
    float sun_luminance;    // luminance of the sun just before it hits the atmosphere
    float sun_angle;
};


uniform sampler2D depth_tx2D;
uniform vec3 camera_position;

uniform SunlightProperties suns[10];
uniform int sun_count;

uniform sampler3D rayleigh_inscatter_tx3D;
uniform sampler3D mie_inscatter_tx3D;
uniform sampler2D transmittance_tx2D;

uniform float max_altitude[32];
uniform float min_altitude[32];
uniform vec3 atmosphere_center[32];

in vec3 position;
flat in int instanceID;
in vec4 deviceCoords;

layout (location = 0) out vec4 frag_colour;

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
	vec3 pos = ray.origin - atmosphere_center[instanceID];
	vec3 dir = ray.direction;
	
	/* now solve linear equation for line circle intersection */
	float e = 0.001f; //epsilon value
	float a = dot(dir,dir);
	float b = dot(pos,dir);
	float c = dot(pos,pos) - square(max_altitude[instanceID]+e);
	
	
	float diskr = square(b) - a*c;
	
	if(diskr < 0.0) return false;
	
	d.x = (-b + sqrt(diskr))/a;
	d.y = (-b - sqrt(diskr))/a;

	if(d.x < 0.0 && d.y < 0.0)
		return false;
	
	return true;
}

bool intersectGround(in Ray ray,in out vec2 d)
{

	/*	view ray parameters */
	vec3 pos = ray.origin - atmosphere_center[instanceID];
	vec3 dir = ray.direction;
	
	/* now solve linear equation for line circle intersection */
	float e = 0.001f; //epsilon value
	float a = dot(dir,dir);
	float b = dot(pos,dir);
	float c = dot(pos,pos) - square(min_altitude[instanceID]+e);
	
	
	float diskr = square(b) - a*c;
	
	if(diskr < 0.0) return false;
	
	d.x = (-b + sqrt(diskr))/a;
	d.y = (-b - sqrt(diskr))/a;
	
	return true;
}



/*
*	Compute Rayleigh phase function
*/
float rayleighPhaseFunction(in float scattering_angle)
{
	//return (3.0 / (16.0 * PI)) * (1.0 + square(scattering_angle));
	//return 0.8*( (7.0/5.0) + 0.5*cos(scattering_angle));

    // As given in Elek and Kmoch
    return 0.75 * (1.0 + square(scattering_angle));
}

/*
*	Compute Mie phase function
*/
float miePhaseFunction(in float scattering_angle, in float mieG)
{
    //float mieG_squared = mieG*mieG;
    //return 1.5 * 1.0 / (4.0 * PI) * (1.0 - mieG_squared) * pow(1.0 + (mieG_squared) - 2.0*mieG*scattering_angle, -3.0/2.0) * (1.0 + square(scattering_angle)) / (2.0 + mieG_squared);
	//return 1.5 * 1.0 / (4.0 * PI) * (1.0 - mieG*mieG) * pow(1.0 + (mieG*mieG) - 2.0*mieG*scattering_angle, -3.0/2.0) * (1.0 + square(scattering_angle)) / (2.0 + mieG*mieG);
    
    // As given in Elek and Kmoch
    return ( (3.0*(1.0-square(mieG))) / (2.0*(2.0+square(mieG))) ) * ( (1.0 + square(scattering_angle)) / pow(1.0+square(mieG)-2.0*mieG*scattering_angle,1.5) );
}

struct Textuer4DCoords
{
	float x_l;
	float x_r;
	float alpha; //< interpolate between x_l and x_r
	float y;
	float z;
};

Textuer4DCoords compute4DTextureAccesCoordinates(float altitude, float viewZenith, float sunZenith, float viewSun)
{
	float H = sqrt(square(max_altitude[instanceID]) - square(min_altitude[instanceID]));
    float rho = sqrt(square(altitude) - square(min_altitude[instanceID]));
	
	float rmu = altitude * viewZenith;
    float delta = square(rmu) - square(altitude) + square(min_altitude[instanceID]);
	
	float res_mu = 128.0;
	float res_mu_s = 32.0;
	float res_r = 32.0;
	float res_nu = 8.0;
	
    vec4 cst = rmu < 0.0 && delta > 0.0 ? vec4(1.0, 0.0, 0.0, 0.5 - 0.5 / res_mu) : vec4(-1.0, H * H, H, 0.5 + 0.5 / res_mu);
	//float uR = 0.5 / res_r + rho / H * (1.0 - 1.0 / res_r);
	float uR = rho / H * (1.0 - 1.0 / res_r);

    //float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / res_mu);
	float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / res_mu);
	/*	for now, use a simpler mapping (as proposed by Elek) */
	//uMu = (1.0 + viewZenith)/2.0;
	 // paper formula
    //float uMuS = 0.5 / res_mu_s + max((1.0 - exp(-3.0 * sunZenith - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / res_mu_s);
    // better formula
    float uMuS = 0.5 / res_mu_s + (atan(max(sunZenith, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / res_mu_s);
	
    //float lerp = (viewSun + 1.0) / 2.0 * (res_nu - 1.0);
    float lerp = ((viewSun+1.0)/2.0) * (res_nu -1.0);
    float uNu = floor(lerp);
    lerp = lerp - uNu;
	
    /*
	//making my own calculation for the x access coordinate...the original just won't work
	float x_coord = max((1.0 - exp(-3.0 * sunZenith - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / res_mu_s);
	float lerp = mod(x_coord,1.0 / res_mu_s);
	x_coord += viewSun * (1.0/res_mu_s);
	float x_coord_l = x_coord - lerp;
	float x_coord_u = x_coord + ((1.0 / res_mu_s)-lerp);
	lerp /= (1.0/res_mu_s);
	return texture(inscatter_tx3D, vec3(x_coord_l, uMu, uR)) * (1.0 - lerp) +
					texture(inscatter_tx3D, vec3(x_coord_u, uMu, uR)) * lerp;
	*/

	Textuer4DCoords rtn;
	rtn.x_l = (uNu + uMuS)/ res_nu;
	rtn.x_r = (uNu + uMuS + 1.0)/ res_nu;
	rtn.alpha = lerp;
	rtn.y = uMu;
	rtn.z = uR;

	return rtn;
}

vec4 accessTexture4D(sampler3D tex, Textuer4DCoords coordinates)
{
	return texture(tex, vec3(coordinates.x_l,coordinates.y,coordinates.z)) * (1.0 - coordinates.alpha)
            + texture(tex, vec3(coordinates.x_r,coordinates.y,coordinates.z)) * coordinates.alpha;
}

/*
*	Access the precomputed inscatter table.
*	Parametrization is a mix between the one proposed by Bruneton and Neyret, by Elek and my own tweaks. 
*/
vec4 accessInscatterTexture(sampler3D inscatter_tx3D, float altitude, float viewZenith, float sunZenith, float viewSun)
{
    float H = sqrt(square(max_altitude[instanceID]) - square(min_altitude[instanceID]));
    float rho = sqrt(square(altitude) - square(min_altitude[instanceID]));
	
	float rmu = altitude * viewZenith;
    float delta = square(rmu) - square(altitude) + square(min_altitude[instanceID]);
	
	float res_mu = 128.0;
	float res_mu_s = 32.0;
	float res_r = 32.0;
	float res_nu = 8.0;
	
	//float uR = 0.5 / res_r + rho / H * (1.0 - 1.0 / res_r);
	float uR = rho / H * (1.0 - 1.0 / res_r);

	vec4 cst = rmu < 0.0 && delta > 0.0 ? vec4(1.0, 0.0, 0.0, 0.5 - 0.5 / res_mu) : vec4(-1.0, H * H, H, 0.5 + 0.5 / res_mu);
    //float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / res_mu);
	float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / res_mu);
	/*	for now, use a simpler mapping (as proposed by Elek) */
	//uMu = (1.0 + viewZenith)/2.0;
	 // paper formula
    //float uMuS = 0.5 / res_mu_s + max((1.0 - exp(-3.0 * sunZenith - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / res_mu_s);
    // better formula
    float uMuS = 0.5 / res_mu_s + (atan(max(sunZenith, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / res_mu_s);
	
    //float lerp = (viewSun + 1.0) / 2.0 * (res_nu - 1.0);
    float lerp = ((viewSun+1.0)/2.0) * (res_nu -1.0);
    float uNu = floor(lerp);
    lerp = lerp - uNu;
	
    /*
	//making my own calculation for the x access coordinate...the original just won't work
	float x_coord = max((1.0 - exp(-3.0 * sunZenith - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / res_mu_s);
	float lerp = mod(x_coord,1.0 / res_mu_s);
	x_coord += viewSun * (1.0/res_mu_s);
	float x_coord_l = x_coord - lerp;
	float x_coord_u = x_coord + ((1.0 / res_mu_s)-lerp);
	lerp /= (1.0/res_mu_s);
	return texture(inscatter_tx3D, vec3(x_coord_l, uMu, uR)) * (1.0 - lerp) +
					texture(inscatter_tx3D, vec3(x_coord_u, uMu, uR)) * lerp;
	*/

	//return texture(inscatter_tx3D, vec3((uNu + uMuS +1.0) / res_nu, uMu, uR));

	return texture(inscatter_tx3D, vec3((uNu + uMuS) / res_nu, uMu, uR)) * (1.0 - lerp)
            + texture(inscatter_tx3D, vec3((uNu + uMuS + 1.0) / res_nu, uMu, uR)) * lerp;
}

/*
*	Fetch values from the transmittance table.
*/
vec3 fetchTransmittance(in float altitude, in float cosAngle)
{
	float altitude_param = sqrt((altitude-min_altitude[instanceID])/(max_altitude[instanceID]-min_altitude[instanceID]));
	float angle_param = atan((cosAngle + 0.15) / (1.0 + 0.15) * tan(1.5)) / 1.5;
	//angle_param = 1.0 - ((cosAngle*0.5)+0.5);
	
	return texture(transmittance_tx2D,vec2(angle_param,altitude_param)).xyz;
	
	/*	direct evaluation - computationally expansive */
	//return exp(-(beta_r * computeDensity(h_r,altitude, cosAngle) +
	//							beta_m * computeDensity(h_m,altitude, cosAngle)));
}

/*
*	Compute the color of the sky using the precomputed tables and phase functions
*/
vec3 computeSkyColour(float viewSun, float altitude, float viewZenith, float sunZenith, float sun_luminance)
{
	vec3 rgb_sky = vec3(0.0);
	if(altitude > min_altitude[instanceID]+0.001)
	{
		Textuer4DCoords texCoords = compute4DTextureAccesCoordinates(altitude,viewZenith,sunZenith,viewSun);

		rgb_sky = rayleighPhaseFunction(viewSun) * accessTexture4D(rayleigh_inscatter_tx3D,texCoords).xyz;
		rgb_sky += miePhaseFunction(viewSun,0.8) * accessTexture4D(mie_inscatter_tx3D,texCoords).xyz;
		//rgb_sky = rayleighPhaseFunction(viewSun) * accessInscatterTexture(rayleigh_inscatter_tx3D,altitude,viewZenith,sunZenith,viewSun).xyz;
		//rgb_sky += miePhaseFunction(viewSun,0.8) * accessInscatterTexture(mie_inscatter_tx3D,altitude,viewZenith,sunZenith,viewSun).xyz;
		rgb_sky /= 4.0*PI;
        
        rgb_sky *= sun_luminance;

        // There seems to be an error somewhere in the atmosphere computation as the sky is too dark
        // when using physical units and realistic values for the sun luminance.
        // Therefore an additional factor is multiplied until the error is found
        rgb_sky *= 2.0;
	}
	
	return max(rgb_sky,vec3(0.0));
}

void main()
{
    /* get depth value from g-buffer */
    vec2 uvCoords = ((deviceCoords.xy / deviceCoords.w) + 1.0) * 0.5;
    float depth = texture(depth_tx2D,uvCoords).x;
    
    Ray view_ray;
    view_ray.origin = camera_position;
    view_ray.direction = normalize(position-camera_position);
    
    vec3 rgb_linear = vec3(0.0);
    
    vec2 intersections;
	intersectAtmosphere(view_ray,intersections);
    //if(!intersectAtmosphere(view_ray,intersections))
	//	discard;
    
    /*	check for two positive intersections -> camera outside atmosphere */
    if( intersections.x>0.0 && intersections.y>0.0)
    	view_ray.origin = view_ray.origin + view_ray.direction * min(intersections.x,intersections.y);
    	
    float altitude = length(view_ray.origin - atmosphere_center[instanceID]);
    vec3 zenith_direction = normalize(view_ray.origin - atmosphere_center[instanceID]);

    float viewZenith = dot( view_ray.direction, zenith_direction );
    
    for(int i=0; i<sun_count; i++)
    {
        float sunZenith = dot( suns[i].sun_direction, zenith_direction );
        float viewSun = dot( view_ray.direction, suns[i].sun_direction );
        
        rgb_linear += computeSkyColour(viewSun,altitude,viewZenith,sunZenith,suns[i].sun_luminance);
        
        if( depth > 0.0)
        {
            vec3 geometry_position = view_ray.origin + depth * view_ray.direction;

            altitude = length(geometry_position - atmosphere_center[instanceID]);
            zenith_direction = normalize(geometry_position - atmosphere_center[instanceID]);

            sunZenith = dot( suns[i].sun_direction, zenith_direction );
            viewZenith = dot( view_ray.direction, zenith_direction );
        
            rgb_linear -= computeSkyColour(viewSun,altitude,viewZenith,sunZenith,suns[i].sun_luminance);
        }
        else
        {
            /* sun disc */
            if( acos(viewSun) <= suns[i].sun_angle )
            {
                float intensity = suns[i].sun_luminance;
                rgb_linear += vec3(intensity) * fetchTransmittance(altitude,viewZenith);
            }

            //  vec2 ground_intersections;
            //  if(intersectGround(view_ray,ground_intersections))
            //      if( ground_intersections.x > 0 && ground_intersections.y > 0 )
            //          rgb_linear = vec3(5000.0);
        }
    }

    frag_colour = vec4(rgb_linear, length(position-camera_position));

	//frag_colour = vec4( vec3(max(intersections.x,intersections.y)) * 100000.0 ,length(position-camera_position));

	if(!intersectAtmosphere(view_ray,intersections))
		frag_colour = vec4(vec3(100000.0,0.0,0.0), length(position-camera_position));
}