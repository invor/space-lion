#version 330

uniform mat4 model_view_proj_matrix[32];

in vec3 vPosition;
in vec3 vColour;

out vec3 colour;

void main()
{
	colour = vColour;
	gl_Position =  model_view_proj_matrix[gl_InstanceID] * vec4(vPosition, 1.0);
}