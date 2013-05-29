#version 330

uniform sampler2D inputImage;

/*
/	Contains the offset value for one pixel in either horizontal or
/	vertical direction, depending on which part of the seperated
/	gaussian in computed.
*/
uniform vec2 pixelOffset;

/*	Normalized coordinates [0,1]x[0,1] of the fragment */
in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	/*	values in negative direction */
	vec4 rgbaN = texture(inputImage, uvCoord - pixelOffset);
	/*	values at the center position of the stencil */
	vec4 rgbaC = texture(inputImage, uvCoord);
	/*	values in positive direction */
	vec4 rgbaP = texture(inputImage, uvCoord + pixeloffset);
	
	/*	
	/	Hardcoded 3x3 seperate gaussian.
	/
	/				  -------        -------
	/				 |   2   |      |       |
	/	(1.0/16.0) * |   4   |  or  | 2 4 2 | 
	/				 |   2   |      |       |
	/				  -------        -------
	*/
	vec4 rgbF = (rgbaN/8.0) + (rgbaC/4.0) + (rgbaP/8.0);


	fragColour = rgbF;
}