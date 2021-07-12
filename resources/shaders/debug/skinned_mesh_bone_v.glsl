#version 450
#extension GL_ARB_shader_draw_parameters : require

struct PerDrawData
{
	mat4 model_matrix;

	uvec2 base_color_tx_hndl;
	uvec2 roughness_tx_hndl;
	uvec2 normal_tx_hndl;

	uvec2 padding;
};

layout(std430, binding = 0) readonly buffer PerDrawDataBuffer { PerDrawData per_draw_data[]; };

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

in vec3 v_position;
in vec3 v_normal;

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;

void main()
{   
	// Construct matrices that use the model matrix
	mat3 normal_matrix = transpose(inverse(mat3(per_draw_data[gl_DrawIDARB].model_matrix)));

	// Just to be on the safe side, normalize input vectors again
	normal = normalize(v_normal);

	// Transform input vectors into world space
	normal = normalize(normal_matrix * normal);

	// Transform vertex position to world space
	position = (per_draw_data[gl_DrawIDARB].model_matrix * vec4(v_position,1.0)).xyz;
	
	gl_Position =  projection_matrix * view_matrix * vec4(position, 1.0);
}