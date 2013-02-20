//	temporary default fragment shader
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

	//	fetch colour from diffuse map and blend it with vertex colour
	vec3 tColour = texture2D(diffuseMap, uvCoord).xyz;
	//tColour = mix(tColour,colour.xyz,colour.w);

	//	fetch specular factor from specular map
	float tSpecFactor = texture2D(specularMap, uvCoord).x;

	//	fetch normal vector from normal map
	vec3 tNormal = ((texture2D(normalMap, uvCoord).xyz)*2.0)-1.0;

	//	compute phong lighting
	fragColour = vec4(phongShading(tSpecFactor, tColour, tNormal, lightDirection, lightColour),1.0);
}