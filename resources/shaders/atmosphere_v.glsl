#version 430

/*	Support handling for up to 128 instances, and reserve a fixed amount of uniforms for it */

uniform mat4 model_matrix[128];
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

in vec3 v_position;

out vec3 position;
flat out int instanceID;

void main()
{	
	position = (model_matrix[gl_InstanceID] * vec4(v_position,1.0)).xyz;
	instanceID = gl_InstanceID;

	gl_Position = projection_matrix * view_matrix * model_matrix[gl_InstanceID] * vec4(v_position, 1.0);
}