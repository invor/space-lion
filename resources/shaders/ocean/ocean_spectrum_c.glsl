#version 430

#define PI 3.14159265359

uniform sampler2D tilde_h0_of_k_tx2D;
uniform sampler2D tilde_h0_of_minus_k_tx2D;
layout(rg32f, binding = 0) writeonly uniform image2D spectrum_x_tx2D;
layout(rg32f, binding = 1) writeonly uniform image2D spectrum_y_tx2D; // height displacement
layout(rg32f, binding = 2) writeonly uniform image2D spectrum_z_tx2D;

uniform uint grid_size;
uniform vec2 size;
uniform float T;
uniform float t;

vec2 multComplex(vec2 a, vec2 b)
{
	return vec2(a[0] * b[0] - a[1] * b[1], a[1] * b[0] + a[0] * b[1]);
}

vec2 conjugateComplex(vec2 a)
{
	return vec2(a.x,-a.y);
}

vec2 addComplex(vec2 a, vec2 b)
{
    return vec2( a.x + b.x, a.y + b.y);
}

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

void main()
{
	ivec3 gID = ivec3(gl_GlobalInvocationID.xyz);
	
	// Compute wave vector
	float n = float(gID.x) - float(grid_size)/2.0;
	float m = float(gID.y) - float(grid_size)/2.0;

	//n = float(gID.x);
	//m = float(gID.y);

	vec2 k = vec2( (2.0*PI*n)/size.x, (2.0*PI*m)/size.y );
	float k_magnitude = max(0.00001,length(k));
	
	vec2 h0_k = texelFetch(tilde_h0_of_k_tx2D, gID.xy, 0).xy;
	vec2 h0_minus_k = texelFetch(tilde_h0_of_minus_k_tx2D, gID.xy, 0).xy;
	//vec2 h0_minus_k = texelFetch(tilde_h0_of_k_tx2D, ivec2(grid_size-1) - gID.xy, 0).xy;
	
	// dispersion/phase
	float w_0 = (2.0*PI)/T;
	float w_k = sqrt(9.81 * k_magnitude);
	//w_k = floor(w_k/w_0)*w_0;

	vec2 dispersion_complex = vec2( cos(w_k*t) , sin(w_k*t) );
	vec2 dispersion_complex_inv = vec2( cos(w_k*t) , -sin(w_k*t) );

	vec2 h_k = addComplex( multComplex(h0_k,dispersion_complex), multComplex( conjugateComplex(h0_minus_k), dispersion_complex_inv));

	vec2 dx = multComplex( vec2(0.0,-k.x/k_magnitude) ,h_k) * 1.5;

	vec2 dz = multComplex( vec2(0.0,-k.y/k_magnitude) ,h_k) * 1.5;


	imageStore(spectrum_x_tx2D,gID.xy,vec4(dx,0.0,1.0));
	imageStore(spectrum_y_tx2D,gID.xy,vec4(h_k,0.0,1.0));
	imageStore(spectrum_z_tx2D,gID.xy,vec4(dz,0.0,1.0));
}