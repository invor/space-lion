//	temporary default vertex shader
#version 420

uniform mat4 modelViewProjectionMatrix;

in vec3 in_vPosition;
in vec3 in_vNormal;
in vec3 in_vTangent;
in vec3 in_vColour;
in vec2 in_vUVCoord;

out vec3 vPosition;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vColour;
out vec2 vUVCoord;

void main()
{
	vPosition = modelViewProjectionMatrix * vec4(in_vPosition, 1);
	vNormal = in_vNormal;
	vTangent = in_vTangent;
	vColour = in_vColour;
	vUVCoord = in_vUVCoord;
	
	gl_Position = vPosition;
}