#version 450
#extension GL_ARB_bindless_texture : require

#define PI 3.1415926535
#define INV_PI 0.318309886183

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

in vec3 position;
in vec2 uvCoord;
in mat3 tangent_space_matrix;

flat in int draw_id;

layout (location = 0) out vec4 normal;
layout (location = 1) out float depth;
layout (location = 2) out vec3 albedoRGB;
layout (location = 3) out vec4 specularRGB_roughness;


// Source: https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl
// Converts a color from sRGB gamma to linear light gamma
vec4 toLinear(vec4 sRGB)
{
    bvec4 cutoff = lessThan(sRGB, vec4(0.04045));
    vec4 higher = pow((sRGB + vec4(0.055))/vec4(1.055), vec4(2.4));
    vec4 lower = sRGB/vec4(12.92);

    return mix(higher, lower, cutoff);
}


void main()
{
	sampler2D base_tx_hndl = sampler2D(per_draw_data[draw_id].base_color_tx_hndl);
	sampler2D roughness_tx_hndl = sampler2D(per_draw_data[draw_id].roughness_tx_hndl);
    sampler2D normal_tx_hndl = sampler2D(per_draw_data[draw_id].normal_tx_hndl);

	vec4 albedo_tx_value = texture(base_tx_hndl, uvCoord);
	vec4 roughness_tx_value = texture(roughness_tx_hndl, uvCoord);
	vec4 normal_tx_value = texture(normal_tx_hndl, uvCoord);

	bool is_sRGB = true;
	if(is_sRGB){
		albedo_tx_value = toLinear(albedo_tx_value);
		//roughness_tx_value = toLinear(roughness_tx_value);
	}

	vec2 metallicRoughness = roughness_tx_value.bg;

	depth = length( (view_matrix * vec4(position,1.0)).rgb );
	
	vec3 tNormal = ( normal_tx_value.rgb * 2.0) - 1.0;
	normal.xyz = (normalize(transpose(tangent_space_matrix) * tNormal)) * 0.5 + 0.5;
	
	albedoRGB = albedo_tx_value.rgb;
	
	specularRGB_roughness = vec4( metallicRoughness, 0.0, 0.0 );	
}