//	temporary default vertex shader
#version 420

uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec4 vColour;
in vec2 vUVCoord;

out vec3 position;
out vec4 colour;
out vec2 uvCoord;
out vec3 viewerDirection;
out mat3 tangentSpaceMatrix;

void main()
{
	//	just to be on the safe side, normalize input vectors again
	vec3 normal = normalize(vNormal);
	normal = normalize( normalMatrix * normal);

	vec3 tangent = normalize(vTangent);
	tangent = normalize( normalMatrix * tangent);

	vec3 bitangent = normalize( cross(normal, tangent) ) * vTangent.w;

	tangentSpaceMatrix = mat3(
		tangent.x, bitangent.x, normal.x,
		tangent.y, bitangent.y, normal.y,
		tangent.z, bitangent.z, normal.z) ;
		
	position = (modelViewMatrix * vec4(vPosition,1.0)).xyz;
	viewerDirection = tangentSpaceMatrix * normalize( -position );

	colour = vColour;
	uvCoord = vUVCoord;
	
	gl_Position =  modelViewProjectionMatrix * vec4(vPosition, 1);
}