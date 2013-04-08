#version 330

uniform sampler2D inputImage;

//	Normalized coordinates [0,1]x[0,1] of the fragment
in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	fragColour = vec4(texture2D(inputImage, uvCoord).xyz,1.0);
}