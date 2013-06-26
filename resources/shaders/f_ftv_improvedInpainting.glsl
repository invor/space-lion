/*
---------------------------------------------------------------------------------------------------
File: f_ftv_improvedImageInpainting.glsl
Author: Michael Becher
Date of (presumingly) last edit: 21.06.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: This GLSL fragment shader applies post-processing to image data, using 
image inpainting techniques to cover up gaps in an image.
---------------------------------------------------------------------------------------------------
*/

#version 330

#define PI 3.1415926535897932384626433832795

uniform sampler2D input_tx2D;
uniform sampler2D mask_tx2D;
uniform sampler2D coherence_tx2D;

uniform vec2 h;
uniform float stencilSize;


/*	Normalized coordinates [0,1]x[0,1] of the fragment. */
in vec2 uvCoord;

/*	Fragment shader output variable. */
out vec4 fragColour;

vec4 imageInpainting()
{
	vec4 rgbaAcc = vec4(0.0);
	float weightSum = 0.0;
	
	vec4 rgbaValues;
	float maskValue;
	vec3 coherence;
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
			
			rgbaValues = texture(input_tx2D, currentPos);
			maskValue = texture(mask_tx2D,currentPos).x;
			if(maskValue>0.0) coherence = texture(coherence_tx2D,currentPos).xyz;
			//else coherence = vec3(0.0,0.0,1.0);
			else coherence = vec3(normalize(vec2((uvCoord - currentPos).y,-(uvCoord - currentPos).x)),2.0);
			
			//weight = (coherence.z/length(uvCoord - currentPos)) * abs( dot(normalize(differenceVector),normalize(coherence.xy) ) );
			
			weight = sqrt(PI/2.0) * (coherence.z/length(uvCoord - currentPos)) *
						exp( -(coherence.z*coherence.z)/(2.0)
								* pow( abs( dot(normalize(uvCoord - currentPos),normalize(vec2(-coherence.y,coherence.x)) ) ),2.0 ) );
								
			weight = /*(coherence.z/length(vec2(differenceVector.x/h.x,differenceVector.y/h.y))) */ exp( -(coherence.z*coherence.z)/(1.0)
								* pow( abs( dot(normalize(uvCoord - currentPos),normalize(vec2(-coherence.y,coherence.x)) ) ),2.0 ) );
			//weight = exp(-pow( abs( dot(normalize(differenceVector),normalize(differenceVector) ) ),2.0 ) );
			
			rgbaAcc += rgbaValues * weight;
			weightSum += weight;
		}
	}
	
	rgbaAcc = rgbaAcc.yxzw/weightSum;
	//rgbaAcc = vec4(vec3(weight),1.0);
	//rgbaAcc = vec4(texture(coherence_tx2D,currentPos).z);
	
	return rgbaAcc;
}

void main()
{
	/*	Check if the fragment is inside the inpainting domain. */
	if(texture(mask_tx2D,uvCoord).x < 0.5f)
	{
		/*	Get the rgba value from the inpainting function. */
		fragColour = imageInpainting();
	}
	else
	{	
		fragColour = texture(input_tx2D,uvCoord);
	}
}