#version 330

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;

in vec3 vPosition;
in vec3 vColour;

out vec3 fragmentWorldPosition;
out vec3 colour;

void main()
{
	fragmentWorldPosition = (modelMatrix * vec4(vPosition, 1.0)).xyz;
	colour = vColour;
	gl_Position =  modelViewProjectionMatrix * vec4(vPosition, 1.0);
}