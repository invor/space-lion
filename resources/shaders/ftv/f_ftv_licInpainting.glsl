/*---------------------------------------------------------------------------------------------------
File: f_ftv_licInpainting.glsl
Author: Michael Becher
Date of (presumingly) last edit: 08.10.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: This GLSL fragment shader applies post-processing to image data, using a
LIC inspired techniques to cover up gaps in an image.
---------------------------------------------------------------------------------------------------*/

#version 330

/*	Texture containing the 2d guidance vector field */
uniform sampler2D guidanceField_tx2D;

/*	Texture containing the currently rendered frame */
uniform sampler2D currFrame_tx2D;

/*	Texture containing the inpainting mask */
uniform sampler2D mask_tx2D;

/*
/	Specifies the distance between two pixels in x and y direction
/	(in texture space [0,1] )
*/
uniform vec2 h;

/*	Normalized coordinates [0,1]x[0,1] of the fragment */
in vec2 uvCoord;

/*	Fragment shader output variable */
out vec4 fragColour;


vec3 licInpainting(vec2 pos)
{
	/*	Storage varibales for rgb and weight accumulation */
	vec3 rgbAcc = vec3(0.0);
	float weightSum = 0.0;

	/*
	/	Get guidance vector at inital/start position.
	/	Note that at the start point the same vector applies for foward and backward direction.
	*/
	vec2 forwardVec = normalize(texture(guidanceField_tx2D,pos).xy);
	vec2 backwardVec = forwardVec;
	/*	Get inital/start position */
	vec2 forwardPos = pos;
	vec2 backwardPos = pos;

	vec3 rgbForwardStep;
	vec3 rgbBackwardStep;

	/*	In case the pixel directions are varying in x- and y-direction, use the average */
	float hAvg = (h.x + h.y)/2.0;

	/*
	/	Move i steps along the vectorfield in forward and backward direction just like
	/	traditional LIC and gather samples at each step.
	*/
	for(float i=0.0; i<10.0; i++)
	{
		/*	Calculate new positions in forwad and backward direction */
		forwardPos += forwardVec*hAvg;
		backwardPos -= backwardVec*hAvg;

		/*	Get the rgb values at the current forward and backward positions */
		rgbForwardStep = texture(currFrame_tx2D,forwardPos).xyz;
		rgbBackwardStep = texture(currFrame_tx2D,backwardPos).xyz;

		/*	
		/	Add rgb values to the accumulator.
		/	Use a simple weighting scheme that reduces the weight of a sample based
		/	on it's distance from the start point along the streamline.
		/	Sum up the weight for normalization.
		*/
		rgbAcc += (rgbForwardStep + rgbBackwardStep)/(i+1.0);
		weightSum += 2.0/(i+1.0);

		/*	Get new guidance vectors for forward and backward direction for the next step */
		forwardVec = normalize(texture(guidanceField_tx2D,forwardPos).xy);
		backwardVec = normalize(texture(guidanceField_tx2D,backwardPos).xy);
	}

	/*	Normalize the accumulated rgb values */
	rgbAcc /= weightSum;

	return rgbAcc;
}


void main()
{	
	fragColour = (texture2D(mask_tx2D,uvCoord).x < 0.5f) ? vec4(licInpainting(uvCoord),1.0) : vec4(texture2D(currFrame_tx2D,uvCoord).xyz,1.0);
}