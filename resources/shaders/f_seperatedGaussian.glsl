/*
---------------------------------------------------------------------------------------------------
File: f_seperatedGaussian.glsl
Author: Michael Becher
Date of (presumingly) last edit: 04.06.2013

Description: Seperated gaussian blur filter. Called twice for a single
filtering step.
---------------------------------------------------------------------------------------------------
*/

#version 330

#define PI 3.1415926535897932384626433832795

uniform sampler2D inputImage;

/*
/	Contains the offset for one pixel in either horizontal or
/	vertical direction, depending on which part of the seperated
/	gaussian is computed.
*/
uniform vec2 pixelOffset;
/*
/	Specifies the stencil size for the gaussian filtering,
/	stencilRadius=1 equls a 3x3 stencil size.
*/
uniform int stencilRadius;
uniform float sigma;

/*	Normalized screen coordinates [0,1]x[0,1] of the fragment */
in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	vec4 rgbaAcc;
	vec2 centerPos = uvCoord;
	float coeffcientSum = 1.0;
	
	/*	Initial values are taken from the center pixel */
	rgbaAcc = texture(inputImage, centerPos);

	/*	This should be a smarter/faster way to get the gaussian coeffiencents. */
	vec3 g;  
	g.x = 1.0 / (sqrt(2.0 * PI) * sigma);  
	g.y = exp(-0.5 * pixelOffset.x * pixelOffset.x / (sigma * sigma));  
	g.z = g.y * g.y;
	
	for(int i = 1; i <= stencilRadius; i++)
	{
		/*	Negative direction (left/down) */
		rgbaAcc += (texture(inputImage, centerPos - float(i) * pixelOffset)) * g.x;
		coeffcientSum += g.x;
		
		/*	Positive direction (right/up) */
		rgbaAcc += (texture(inputImage, centerPos + float(i) * pixelOffset)) * g.x;
		coeffcientSum += g.x;
		
		g.xy *= g.yz;
	}
	
	/*	Normalization */
	rgbaAcc /= coeffcientSum;
	
	fragColour = rgbaAcc;
}