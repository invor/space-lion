/*
---------------------------------------------------------------------------------------------------
File: f_ftv_seperatedGaussian.glsl
Author: Michael Becher
Date of (presumingly) last edit: 04.06.2013

Describtion: Seperated gaussian blur filter. Called twice for a single
filtering step. Additonally this ftv version takes a mask image to decide
whether or not a texel contains a valid value.
---------------------------------------------------------------------------------------------------
*/

#version 330

#define PI 3.1415926535897932384626433832795

uniform sampler2D inputImage;
uniform sampler2D maskImage;

/*
/	Contains the offset value for one pixel in either horizontal or
/	vertical direction, depending on which part of the seperated
/	gaussian in computed.
*/
uniform vec2 pixelOffset;
/*
/	Specifies the stencil size for the gaussian filtering,
/	stencilRadius=1 equls a 3x3 stencil size.
*/
uniform int stencilRadius;
uniform float sigma;

/*	Normalized coordinates [0,1]x[0,1] of the fragment */
in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	vec4 rgbaAcc;
	float maskValue;
	vec2 centerPos = uvCoord;
	float coeffcientSum = 1.0;
	
	/*	Initial values are taken from the center pixel */
	rgbaAcc = texture(inputImage, centerPos);
	/*	maskValue should either be 1.0 or 0.0 */
	maskValue = texture(maskImage, centerPos).x;
	
	rgbaAcc *= maskValue;
	coeffcientSum *= maskValue;

	/*	This should be a smarter/faster way to get the gaussian coeffiencents. */
	vec3 g;  
	g.x = 1.0 / (sqrt(2.0 * PI) * sigma);  
	g.y = exp(-0.5 * pixelOffset.x * pixelOffset.x / (sigma * sigma));  
	g.z = g.y * g.y;
	
	for(int i = 1; i <= stencilRadius; i++)
	{
		/*	Negative direction (left/down) */
		maskValue = (texture(maskImage, centerPos - float(i) * pixelOffset)).x;
		rgbaAcc += (texture(inputImage, centerPos - float(i) * pixelOffset)) * g.x * maskValue;
		coeffcientSum += g.x * maskValue;
		
		/*	Positive direction (right/up) */
		maskValue = (texture(maskImage, centerPos + float(i) * pixelOffset)).x;
		rgbaAcc += (texture(inputImage, centerPos + float(i) * pixelOffset)) * g.x * maskValue;
		coeffcientSum += g.x * maskValue;
		
		g.xy *= g.yz;
	}
	
	/*	Normalization */
	rgbaAcc /= coeffcientSum;
	
	fragColour = rgbaAcc;
}