//	temporary default vertex shader
#version 330

uniform mat4 modelViewProjectionMatrix;

in vec3 vPosition;
in vec3 vColour;

out vec3 colour;

void main()
{
	colour = vColour;
	gl_Position =  modelViewProjectionMatrix * vec4(vPosition, 1);
}