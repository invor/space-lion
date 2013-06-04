#version 330

uniform sampler2D structureTensor;

/*
/	Normalized coordinates [0,1]x[0,1] of the fragment.
*/
in vec2 uvCoord;

/*
/	Fragment shader output variable.
*/
out vec3 coherenceOuput;

mat2 getStructureTensor(vec2 position)
{
	return mat2(1.0);
}

void main()
{
	float dx = texture(structureTensor,uvCoord).x;
	float dy = texture(structureTensor,uvCoord).y;
	float gradientMag = sqrt( pow(dx,2.0)+pow(dy,2.0) );

	coherenceOuput = vec3(gradientMag);
}