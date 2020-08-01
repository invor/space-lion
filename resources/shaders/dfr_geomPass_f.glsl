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

void main()
{
	sampler2D base_tx_hndl = sampler2D(per_draw_data[draw_id].base_color_tx_hndl);
	sampler2D roughness_tx_hndl = sampler2D(per_draw_data[draw_id].roughness_tx_hndl);
    sampler2D normal_tx_hndl = sampler2D(per_draw_data[draw_id].normal_tx_hndl);

	vec2 metallicRoughness = texture(roughness_tx_hndl, uvCoord).bg;

	depth = length( (view_matrix * vec4(position,1.0)).rgb );
	
	vec3 tNormal = ( texture(normal_tx_hndl, uvCoord).rgb * 2.0) - 1.0;
	normal.xyz = (normalize(transpose(tangent_space_matrix) * tNormal)) * 0.5 + 0.5;
	
	albedoRGB = texture(base_tx_hndl, uvCoord).rgb;
	
	specularRGB_roughness = vec4( metallicRoughness, 0.0, 0.0 );	
}