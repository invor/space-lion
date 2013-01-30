//	temporary default vertex shader
#version 420

uniform mat4 modelViewProjectionMatrix;

in vec3 vertexPosition;

void main()
{
	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1);
}