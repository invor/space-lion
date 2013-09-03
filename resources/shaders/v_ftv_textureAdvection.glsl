#version 330

in vec3 vPosition;
in vec2 vUVCoord;

out vec2 uvCoord;

uniform sampler2D guidanceField_tx2D;

void main()
{
	uvCoord = vUVCoord;
	vec2 displacePos = vPosition.xy + 1.0f*texture(guidanceField_tx2D,uvCoord).xy;
	gl_Position =  vec4(displacePos, vPosition.z, 1.0);
	//gl_Position =  vec4(vPosition, 1.0);
}