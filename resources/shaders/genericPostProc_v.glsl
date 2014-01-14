#version 330

in vec3 vPosition;
in vec2 vUVCoord;

out vec2 uvCoord;

void main()
{
	uvCoord = vUVCoord;
	gl_Position =  vec4(vPosition, 1.0);
}