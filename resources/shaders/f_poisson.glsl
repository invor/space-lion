#version 330

uniform sampler2D previousFrame;
uniform sampler2D inputImage;
//
//	Define the domain of the missing image data by the coordinates of the lower left
//	and upper right corner of a rectangular domian.
//	 ----x
//	|    |
//	x---- 
//
uniform vec2 lowerBound;
uniform vec2 upperBound;

//	Normalized coordinates [0,1]x[0,1] of the fragment
in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	fragColour = vec4(texture2D(inputImage, uvCoord).xyz,1.0);
}