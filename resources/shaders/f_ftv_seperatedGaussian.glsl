/*
---------------------------------------------------------------------------------------------------
File: f_ftv_seperatedGaussian.glsl
Author: Michael Becher
Date of (presumingly) last edit: 04.06.2013

Description: Seperated gaussian blur filter. Called twice for a single
filtering step. Additonally this ftv version takes a mask image to decide
whether or not a texel contains a valid value.
---------------------------------------------------------------------------------------------------
*/

#version 330

#define PI 3.1415926535897932384626433832795

uniform sampler2D inputImage;
uniform sampler2D maskImage;
uniform sampler2D distanceMap;

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
	/*
	/	Careful, dirty hack!
	/
	/	For use with ftv, the seperated gaussian has to change its behaviour
	/	during the second (vertical) execution. Some pixels inside on the border
	/	of the inpainting region were filled with color values during the first
	/	execution. This is desired and those pixel shouldn't be seen as invalid
	/	during the second pass.
	/	Therefore we need to deteced whether we are in the second pass and if
	/	we are located on the vertical border of the inpainting region.
	/	If we are, we add an offset to the mask texture lookup so that the pixels
	/	will be used.
	/	Note that the offset will be horizontal, while the 1D gaussian stencil
	/	will be vertical at this stage, so they won't mess each other up.
	*/
	vec2 maskOffset;
	vec4 dist = texture(distanceMap,uvCoord);
	if(pixelOffset.y > 0.0)
	{	
		/*
		/	This should actually be the width of one pixel! Will only work for
		/	quadratic images. FIX IT!
		*/
		if(dist.x > 0.0/400.0 && dist.x < 1.0/400.0)
		{
			maskOffset = vec2(-1.0/400.0,0.0);
		}
		if(dist.z > 0.0/400.0 && dist.z < 1.0/400.0)
		{
			maskOffset = vec2(1.0/400.0,0.0);
		}
	}

	vec4 rgbaAcc;
	float maskValue;
	vec2 centerPos = uvCoord;
	float coeffcientSum = 1.0;
	
	/*	Initial values are taken from the center pixel */
	rgbaAcc = texture(inputImage, centerPos);
	/*	maskValue should either be 1.0 or 0.0 */
	maskValue = texture(maskImage, centerPos + maskOffset).x;
	
	rgbaAcc *= maskValue;
	coeffcientSum *= maskValue;
    
	/*	This should be a smarter/faster way to get the gaussian coeffiencents. */
	float h = max(pixelOffset.x,pixelOffset.y);
	vec3 g;  
	g.x = 1.0 / (sqrt(2.0 * PI) * h * sigma);  
	g.y = exp(-0.5 * h * h / (h * h * sigma * sigma));  
	g.z = g.y * g.y;
	
	for(int i = 1; i <= stencilRadius; i++)
	{
		/*	Negative direction (left/down) */
		maskValue = (texture(maskImage, centerPos - float(i) * pixelOffset + maskOffset)).x;
		rgbaAcc += (texture(inputImage, centerPos - float(i) * pixelOffset)) * g.x * maskValue;
		coeffcientSum += g.x * maskValue;
		
		/*	Positive direction (right/up) */
		maskValue = (texture(maskImage, centerPos + float(i) * pixelOffset + maskOffset)).x;
		rgbaAcc += (texture(inputImage, centerPos + float(i) * pixelOffset)) * g.x * maskValue;
		coeffcientSum += g.x * maskValue;
		
		g.xy *= g.yz;
	}
	
	/*	Normalization */
	rgbaAcc /= coeffcientSum;
	
	fragColour = rgbaAcc;
}