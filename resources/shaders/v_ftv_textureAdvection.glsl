#version 330

in vec3 vPosition;
in vec2 vUVCoord;

out vec2 uvCoord;

uniform sampler3D guidanceField_tx3D;

void main()
{
	uvCoord = vUVCoord;
	vec2 displacePos = vPosition.xy + 1.0f*texture(guidanceField_tx3D,vec3(uvCoord,1.0/50.0)).xy;

	//vec2 displacePos = vPosition.xy;
	//for(float i=0.0;i<10.0;i++)
	//{
	//	displacePos += 0.1f * texture(guidanceField_tx3D,vec3(uvCoord,i/10.0)).xy;
	//}

	gl_Position =  vec4(displacePos, vPosition.z, 1.0);
	//gl_Position =  vec4(vPosition, 1.0);
}