#version 450
#extension GL_ARB_shader_draw_parameters : require

struct PerDrawData
{
	mat4 model_matrix;

	int joint_index_offset;

	int padding0;
	int padding1;
	int padding2;
};

layout(std430, binding = 0) readonly buffer PerDrawDataBuffer { PerDrawData per_draw_data[]; };

layout(std430, binding = 1) readonly buffer JointTransfromBuffer { mat4 joint_transforms[]; };

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

in vec4 v_joints;
in vec3 v_normal;
in vec3 v_position;
in vec4 v_joint_weights;

layout (location = 0) out vec3 w_position;
layout (location = 1) out vec3 w_normal;

flat out int draw_id;

void main()
{   
	draw_id = gl_DrawIDARB;

	// Construct matrices that use the model matrix
	mat3 normal_matrix = transpose(inverse(mat3(per_draw_data[gl_DrawIDARB].model_matrix)));

	// Just to be on the safe side, normalize input vectors again
	w_normal = normalize(v_normal);

	// Transform input vectors into world space
	w_normal = normalize(normal_matrix * w_normal);

	// Compute skin matrix
	mat4 skinMatrix = 
		v_joint_weights.x * joint_transforms[per_draw_data[gl_DrawIDARB].joint_index_offset + int(v_joints.x)] +
		v_joint_weights.y * joint_transforms[per_draw_data[gl_DrawIDARB].joint_index_offset + int(v_joints.y)] +
		v_joint_weights.z * joint_transforms[per_draw_data[gl_DrawIDARB].joint_index_offset + int(v_joints.z)] +
		v_joint_weights.w * joint_transforms[per_draw_data[gl_DrawIDARB].joint_index_offset + int(v_joints.w)];

	// Transform vertex position to world space
	w_position = (per_draw_data[gl_DrawIDARB].model_matrix * skinMatrix * vec4(v_position,1.0)).xyz;
	//w_position = (per_draw_data[gl_DrawIDARB].model_matrix * vec4(v_position,1.0)).xyz;
	
	gl_Position =  projection_matrix * view_matrix * vec4(w_position, 1.0);
}