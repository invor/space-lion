#version 430

#define PI 3.14159265359

uniform sampler2D gaussian_noise_tx2D;
layout(rg32f, binding = 0) writeonly uniform image2D tilde_h0_of_k_tx2D;
layout(rg32f, binding = 1) writeonly uniform image2D tilde_h0_of_minus_k_tx2D;

uniform vec2 wind;
uniform vec2 size;
uniform float A; //controls wave height
uniform uint grid_size;

/**
* Compute Phillips spectrum
*
* k = wave vector
* w = wind vector
* a = wave height parameter
*/
float Phillips(in vec2 k, in vec2 w, in float a)
{
	float v = length(w);
	float l = (v*v)/9.81;
	float k_sqr = max(0.00001,dot(k,k));

	//if( length(k) > 0.0)
	//	k = normalize(k);

	if( v > 0.0 )
		w = normalize(w);

	float KdotV = dot(k,w);

	float ph = a * ( exp( -1.0/(k_sqr * l * l) ) / (k_sqr*k_sqr) ) *  KdotV*KdotV;

	if (KdotV < 0.0)
		ph *= 0.07;

	// damp out waves with very small length w << l
	float w_damp = l / 40000.0;
	ph *= exp(-k_sqr * w_damp * w_damp);

	return ph;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main()
{   
    ivec3 gID = ivec3(gl_GlobalInvocationID.xyz);

	vec4 noise =  texelFetch(gaussian_noise_tx2D,gID.xy,0);

	float n = float(gID.x) - (float(grid_size)/2.0);
	float m = float(gID.y) - (float(grid_size)/2.0);

	//n = float(uv.x);
	//m = float(uv.y);

	vec2 k = vec2( (2.0*PI*n)/size.x, (2.0*PI*m)/size.y );
	//k = (length(k) > 0.0) ? normalize(k) : vec2(0.0);

	float Ph = Phillips(k, wind, A);
	vec2 h0 =  noise.rg * sqrt(Ph/2.0);
	imageStore(tilde_h0_of_k_tx2D, gID.xy, vec4(h0,0.0,0.0));


	Ph = Phillips(-k, wind, A);
	h0 = noise.ba * sqrt( Ph/2.0 );
	imageStore(tilde_h0_of_minus_k_tx2D, gID.xy ,vec4(h0,0.0,0.0));
}
