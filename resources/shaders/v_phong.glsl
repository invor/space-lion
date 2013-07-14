/*---------------------------------------------------------------------------------------------------
File: f_phong.glsl
Author: Michael Becher
Date of (presumingly) last edit: 14.07.2013

Description: Temporary default vertex shader for space lion. Implements basic phong shading.
---------------------------------------------------------------------------------------------------*/
#version 330

uniform mat3 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform vec3 lightPosition;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec4 vColour;
in vec2 vUVCoord;

out vec3 position;
out vec4 colour;
out vec2 uvCoord;
out vec3 viewerDirection;
out vec3 lightDirection;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;

void main()
{
	/*	Just to be on the safe side, normalize input vectors again */
	normal = normalize(vNormal);
	normal = normalize(normalMatrix * normal);

	tangent = normalize(vTangent);
	tangent = normalize(normalMatrix * tangent);

	//bitangent = -normalize( cross(normal, tangent) );// * vTangent.w;
	bitangent = normalize(vBitangent);
	bitangent = normalize(normalMatrix * bitangent);

	mat3 tangentSpaceMatrix = mat3(
		tangent.x, bitangent.x, normal.x,
		tangent.y, bitangent.y, normal.y,
		tangent.z, bitangent.z, normal.z) ;
	
	position = (modelViewMatrix * vec4(vPosition,1.0)).xyz;
	viewerDirection = tangentSpaceMatrix * normalize( -position );
	lightDirection = tangentSpaceMatrix * normalize((viewMatrix * vec4(lightPosition,1.0)).xyz - position);
	
	colour = vColour;
	uvCoord = vUVCoord;
	
	gl_Position =  modelViewProjectionMatrix * vec4(vPosition, 1);
}