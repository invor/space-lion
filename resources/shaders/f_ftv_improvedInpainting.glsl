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

uniform sampler2D inputImage;
uniform sampler2D mask;
uniform sampler2D coherence;

uniform vec2 h;
uniform int stencilSize;


/*	Normalized coordinates [0,1]x[0,1] of the fragment. */
in vec2 uvCoord;

/*	Fragment shader output variable. */
out vec4 fragColour;

vec4 imageInpainting()
{
	vec4 rgbaAcc;
	float weightSum;

	vec4 rgbaValues;
	vec2 coherenceFlow;
	float coherenceStrength;
	
	vec2 lowerLeftPos = uvCoord - h*stencilSize;

	for(int i=0; i < (2*stencilSize); i++)
	{
		for(int j=0; j < (2*stencilSize); i++)
		{
			rgbaValues = texture(inputImage, lowerLeftPos + vec2(i,j));
			coherenceFlow = texture(coherence, lowerLeftPos + vec2(i,j)).xy;
			coherenceStrength = texture(coherence, lowerLeftPos + vec2(i,j)).z;
		}
	}
}

void main()
{
	/*
	/	Check if the fragment is inside the inpainting domain.
	*/
	if(texture2D(mask,uvCoord).x < 0.5f)
	{
		/*
		/	Get the rgba value from the inpainting function.
		*/
		fragColour = vec4(imageInpainting(uvCoord));
	}
	else
	{	
		fragColour = vec4(texture2D(inputImage,uvCoord).xyz,1.0);
	}
}