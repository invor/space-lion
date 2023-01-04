#version 430

uniform sampler2D displacement_tx2D;

uniform float choppy_scale;
uniform vec2 size;

layout(rgba32f, binding = 0) writeonly uniform image2D normal_tx2D;

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

void  main()
{
    ivec3 gID = ivec3(gl_GlobalInvocationID.xyz);

    vec3 displace_west  = texelFetch(displacement_tx2D, gID.xy + ivec2(-1,0),0).xyz;
	vec3 displace_east = texelFetch(displacement_tx2D, gID.xy + ivec2(1,0),0).xyz;
	vec3 displace_south  = texelFetch(displacement_tx2D, gID.xy + ivec2(0,-1),0).xyz;
	vec3 displace_north = texelFetch(displacement_tx2D, gID.xy + ivec2(1,1),0).xyz;
	
	vec2 gradient = {-(displace_east.y - displace_west.y), -(displace_north.y - displace_south.y)};
	
	vec2 Dx = (displace_east.xz - displace_west.xz) * choppy_scale * size.x;
	vec2 Dz = (displace_north.xz - displace_south.xz) * choppy_scale * size.y;
	float J = (1.0f + Dx.x) * (1.0f + Dz.y) - Dx.y * Dz.x;
	float fold = max(1.0f - J, 0);

    imageStore(normal_tx2D, gID.xy, vec4( gradient,fold,1.0 ));
}