/*
---------------------------------------------------------------------------------------------------
File: f_improvedImageInpainting.glsl
Author: Michael Becher
Date of (presumingly) last edit: 04.06.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Describtion: This GLSL fragment shader applies post-processing to image data, using 
image inpainting techniques to cover up gaps in an image.
---------------------------------------------------------------------------------------------------
*/

#version 330

#define PI 3.1415926535897932384626433832795

uniform sampler2D inputImage;
uniform sampler2D mask;
uniform sampler2D coherenceImage;

uniform vec2 h;
uniform float stencilSize;


/*	Normalized coordinates [0,1]x[0,1] of the fragment. */
in vec2 uvCoord;

/*	Fragment shader output variable. */
out vec4 fragColour;

vec4 imageInpainting()
{
	vec4 rgbaAcc = vec4(0.0);
	float weightSum = 1.0;
	
	vec4 rgbaValues;
	vec3 coherence;
	float weight;
	vec2 currentPos;
	float epsilon = (2.0*stencilSize)+1.0;
	
	vec2 lowerLeftPos = uvCoord - h*stencilSize;
	
	for(float i=0.0; i < (2.0*stencilSize)+1.0; i++)
	{
		for(float j=0.0; j < (2.0*stencilSize)+1.0; j++)
		{
			currentPos = lowerLeftPos + vec2(h.x*i,h.y*j);
			rgbaValues = texture(inputImage, currentPos);
			coherence = texture(coherenceImage, currentPos).xyz;
			//weight = sqrt(PI/2.0) * (coherence.z/length(uvCoord - currentPos))
			//			* exp( -(coherence.z*coherence.z)/(2.0*epsilon*epsilon)
			//				* pow( abs( dot((uvCoord - currentPos),coherence.xy) ),2.0 ) );
			//weight = abs( dot( normalize(currentPos-uvCoord),normalize(coherence.xy) ) );//(length(uvCoord - currentPos)*length(uvCoord - currentPos));
			weight = coherence.z;
			
			rgbaAcc += rgbaValues;
			weightSum += weight;
		}
	}
	
	//rgbaAcc /= weightSum;
	rgbaAcc = vec4(weightSum);
	rgbaAcc = texture(inputImage,lowerLeftPos);
	return rgbaAcc;
}

void main()
{
	/*
	/	Check if the fragment is inside the inpainting domain.
	*/
	if(texture(mask,uvCoord).x < 0.5f)
	{
		/*
		/	Get the rgba value from the inpainting function.
		*/
		fragColour = imageInpainting();
	}
	else
	{	
		fragColour = texture(inputImage,uvCoord);
	}
}