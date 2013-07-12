//	temporary default fragment shader
#version 330

in vec3 colour;

out vec4 fragColour;

void main()
{
	fragColour = vec4(colour,1.0);
	fragColour = vec4(1.0);
}