/*---------------------------------------------------------------------------------------------------
File: f_phong.glsl
Author: Michael Becher
Date of (presumingly) last edit: 14.07.2013

Description: Temporary default fragment shader for space lion. Implements basic phong shading.
---------------------------------------------------------------------------------------------------*/
#version 330

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

uniform vec4 lightColour;

in vec3 position;
in vec4 colour;
in vec2 uvCoord;
in vec3 viewerDirection;
in vec3 lightDirection;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;

out vec4 fragColour;

vec3 phongShading(in float specFactor ,in vec3 sColour , in vec3 sNormal, in vec3 lightDir, in vec4 lightColour)
{
	vec3 n = normalize(sNormal);
	vec3 reflection = reflect(-normalize(lightDir), n);

	return lightColour.w *
		( sColour*max( dot(normalize(lightDir), n), 0.0)) +
			(specFactor*lightColour.xyz*pow(max(dot(reflection,normalize(viewerDirection)),0.0),22.0) );
}

void main()
{
	fragColour = vec4(1.0);
	vec3 tLightDirection;

	/*	Fetch colour from diffuse map and blend it with vertex colour */
	vec3 tColour = texture2D(diffuseMap, uvCoord).xyz;
	//tColour = mix(tColour,colour.xyz,colour.w);

	/*	Fetch specular factor from specular map */
	float tSpecFactor = texture2D(specularMap, uvCoord).x;

	/*	Fetch normal vector from normal map */
	//vec3 tNormal = ((texture2D(normalMap, uvCoord).xyz)*2.0)-1.0;
	vec3 tNormal = texture2D(normalMap, uvCoord).xyz;

	/*	Calculate phong shading */
	fragColour = vec4(phongShading(tSpecFactor, tColour, tNormal, lightDirection, lightColour),1.0);

	/*	Let's do some debugging */
	//fragColour = vec4(vec3(tSpecFactor),1.0);
}