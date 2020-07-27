#version 450

uniform vec2 screen_resolution;

layout(rgba32f, binding = 0) writeonly uniform image2D lighting_tx2D;
layout(location = 0) uniform sampler2D normal_tx2D;
layout(location = 1) uniform sampler2D depth_tx2D;
layout(location = 2) uniform sampler2D albedoRGB_tx2D;
layout(location = 3) uniform sampler2D specularRGB_roughness_tx2D;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

void main()
{
	vec3 gID = gl_GlobalInvocationID.xyz;

	if(gID.x >= screen_resolution.x || gID.y >= screen_resolution.y)
		return;

	ivec2 base_pixel_coords = ivec2(gID.xy);
    
    float depth = texelFetch(depth_tx2D,base_pixel_coords,0).x;

	//early exit if empty pixel
	if( !(depth > 0.0) )
	{
		imageStore(lighting_tx2D,base_pixel_coords,vec4(0.0));
		return;
	}

    vec3 normal = (texelFetch(normal_tx2D,base_pixel_coords,0).xyz) * 2.0 - 1.0;
	vec3 albedo = texelFetch(albedoRGB_tx2D,base_pixel_coords,0).rgb;
	vec4 specular_roughness = texelFetch(specularRGB_roughness_tx2D,base_pixel_coords,0);

    imageStore(lighting_tx2D,base_pixel_coords,vec4(4000.0));
}