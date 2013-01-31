//	temporary default fragment shader
#version 420

uniform sampler2d diffuseMap;
uniform sampler2d specularMap;
uniform sampler2d normalMap;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vColour;
in vec2 vUVCoord;

out vec4 fragColour;

vec3 phongShading(in vec3 colour, in vec3 normal, in vec3 lightPosition, in vec3 lightColour , in vec3 viewerPosition)
{
	//	placeholder return
	return colour;
}

void main()
{
	fragColour = vec4(1.0);
}