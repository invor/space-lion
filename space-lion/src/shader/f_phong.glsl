//	temporary default fragment shader
#version 420

uniform sampler2d diffuseMap;
uniform sampler2d specularMap;
uniform sampler2d normalMap;

uniform vec3 lightPosition[10];
uniform vec4 lightColour[10];

in vec3 position;
in vec4 colour;
in vec2 uvCoord;
in vec3 viewerDirection
in mat3 tangentSpaceMatrix;

out vec4 fragColour;

vec3 phongShading(in float specFactor ,in vec3 sColour , in vec3 sNormal, in vec3 lightDirection, in vec4 lightColour)
{
	vec3 n = normalize(sNormal);
	vec3 reflection = reflect(-lightDirection, n);

	return lightColour.w *
		( sColour*max( dot(ligthDirection, n), 0.0) +
		specFactor*lightColour.xyz*pow(max(dot(reflection,viewerDirection),0.0),22.0) );
}

void main()
{
	fragColour = vec4(1.0);
	vec3 tLightDirection;

	//	fetch colour from diffuse map and blend it with vertex colour
	vec3 tColour = texture2D(diffuseMap, uvCoord).xyz;
	tColour += (colour.xyz * colour.w);

	//	fetch specular factor from specular map
	float tSpecFactor = texture2D(specularMap, uvCoord).x;

	//	fetch normal vector from normal map
	vec3 tNormal = texture2D(normalMap, uvCoord).xyz;

	for(i=0; i<10; i++)
	{
		//	computer phong lighting for each light source
		tLightDirection = tangentSpaceMatrix * (lightPosition[i] - position);
		fragColour += phongShading(tSpecFactor, tColour, tNormal, tLightDirection, tLightColour[i]);
	}	
	
	//fragColour = vec4(1.0);
}