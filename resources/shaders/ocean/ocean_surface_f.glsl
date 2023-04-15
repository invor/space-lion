#version 430

struct SunlightProperties
{
    vec3 position;
    float illuminance; //(lux)
};

uniform sampler2D displacement_tx2D;
uniform sampler2D normal_tx2D;
uniform sampler2D fresnel_lut_tx2D;

uniform sampler2D depth_tx2D;
uniform vec2 gBuffer_resolution;

uniform sampler3D atmosphere_rayleigh_inscatter_tx3D;
uniform sampler3D atmosphere_mie_inscatter_tx3D;

uniform samplerCube reflectionMap_tx2D;
uniform SunlightProperties suns[8];
uniform int num_suns;

uniform mat4 view_matrix;

uniform float size;
uniform uint grid_size;

in vec3 position;
in vec3 world_position;
in vec2 uv;

layout (location = 0) out vec4 colour;

void main()
{
	vec4 displ = texture(displacement_tx2D,uv);
	vec4 grad_fold = texture(normal_tx2D,uv);

	vec3 cam_world_pos = inverse(transpose(mat3(inverse(view_matrix)))) * vec3(0.0);
	vec3 viewer_direction =  normalize(inverse(transpose(mat3(inverse(view_matrix)))) * -normalize(position));

	float texel_size = (size / float(grid_size) );
	vec3 normal = normalize(vec3(grad_fold.x, texel_size*2, grad_fold.y));

	float VdotN = dot(viewer_direction,normal);

	float fresnel = texture(fresnel_lut_tx2D,vec2(VdotN,0.5)).x;

	vec3 sun_streak;
	for(int i=0; i<num_suns; i++)
	{
		vec3 light_vector = normalize(suns[i].position - world_position);

		vec3 halfway_vector = normalize(light_vector+viewer_direction);

		sun_streak += pow(dot(halfway_vector,normal), 100.0);
	}

	vec2 normalized_frag_position = gl_FragCoord.xy / gBuffer_resolution;
	float depth = texture(depth_tx2D,normalized_frag_position).x;
	if(depth < 0.0001) depth = 10000.0;
	float s = depth - length(position);
	float absorption = clamp(exp(-0.1 * s),0.0,1.0);
	float alpha = 1.0 - absorption;

	//colour = vec4(mix( vec3(0.07,0.15,0.2), vec3(0.7), fresnel ) + sun_streak, length(position));
	colour = vec4( (mix( vec3(0.05,0.13,0.18) * alpha, vec3(0.7), fresnel ) + sun_streak) * (1.0-floor(absorption)), alpha );

	//colour = vec4( displ.xy, 0.0 ,1.0);

	//colour = vec4(sun_streak, length(position));
}