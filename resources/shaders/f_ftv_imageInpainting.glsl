/*
---------------------------------------------------------------------------------------------------
File: f_imageInpainting.glsl
Author: Michael Becher
Date of (presumingly) last edit: 03.07.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: This GLSL fragment shader applies post-processing to image data, using 
image inpainting techniques to cover up gaps in an image.
---------------------------------------------------------------------------------------------------
*/

#version 330

uniform sampler2D input_tx2D;
uniform sampler2D mask_tx2D;
uniform sampler2D gradient_tx2D;

uniform vec2 h;
uniform float stencilSize;

/*
/	Normalized coordinates [0,1]x[0,1] of the fragment.
*/
in vec2 uvCoord;

/*
/	Fragment shader output variable.
*/
out vec4 fragColour;


vec4 imageInpainting()
{
	vec4 rgbaAcc = vec4(0.0);
	float weightSum = 0.0;
	
	vec4 rgbaValues;
	float maskValue;
	vec2 gradient;
	float weight;
	vec2 currentPos;
	vec2 differenceVector;
	float epsilon = (2.0*stencilSize)+1.0;
	
	vec2 lowerLeftPos = uvCoord - h*stencilSize;
	
	
	for(float i = 0.0; i < epsilon; i++)
	{
		for(float j = 0.0; j < epsilon; j++)
		{
			currentPos = lowerLeftPos + vec2(h.x*i,h.y*j);
			differenceVector = uvCoord - currentPos;
			
			maskValue = texture(mask_tx2D,currentPos).x;
			if(maskValue>0.0)
			{
				rgbaValues = texture(input_tx2D, currentPos);
				gradient = texture(gradient_tx2D,currentPos).xy;
									
				weight = (1.0/length(vec2(differenceVector.x/h.x,differenceVector.y/h.y))) * abs( dot(normalize(differenceVector),normalize(gradient)) );
				
				weight = (1.0/length(vec2(differenceVector.x/h.x,differenceVector.y/h.y))) *
							exp( -5.0 * abs( dot(normalize(differenceVector),normalize(vec2(-gradient.y,gradient.x))) ) );
				
				rgbaAcc += rgbaValues * weight;
				weightSum += weight;
			}
			
		}
	}
	
	rgbaAcc = rgbaAcc.xyzw/weightSum;
	
	return rgbaAcc;
}

void main()
{
	/*	Check if the fragment is on the edge of the inpainting domain. */
	if(texture(mask_tx2D,uvCoord).y < 0.5f)
	{
		/*	Get the rgba value from the inpainting function. */
		fragColour = imageInpainting();
	}
	else
	{	
		fragColour = texture(input_tx2D,uvCoord);
	}
}