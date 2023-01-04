#version 430

uniform sampler2D displacement_tx2D;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

uniform float size;

in vec3 v_position;
in vec2 v_uv;

out vec3 position;
out vec3 world_position;
out vec2 uv;

void main()
{
	uv = v_uv;

	vec4 displacement = texture(displacement_tx2D,uv);

	world_position = v_position * size + vec3(displacement.xyz) + vec3(-32.0,-30.0,-32.0);
	position = (view_matrix * vec4( world_position,1.0)).xyz;
	//position = (view_matrix * vec4( v_position * size ,1.0)).xyz;

	//position = (view_matrix * vec4( v_position * size + vec3(0.0,displacement.y,0.0),1.0)).xyz;

	gl_Position = projection_matrix * vec4(position,1.0);
}