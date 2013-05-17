#version 330

uniform sampler2D inputImage;

/*
/	Normalized coordinates [0,1]x[0,1] of the fragment.
*/
in vec2 uvCoord;

/*
/	Fragment shader output variable.
*/
out vec4 fragColour;

vec2 getGradient(vec2 position)
{
}

mat2 getStructureTensor(vec2 position)
{
}

void main()
{
	fragColour = vec4(1.0);
}