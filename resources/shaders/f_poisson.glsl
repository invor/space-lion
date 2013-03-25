#version 330

in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	fragColour = vec4(uvCoord,0.0,1.0);
}