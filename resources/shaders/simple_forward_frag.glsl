#version 450
#extension GL_ARB_bindless_texture : require

struct PerDrawData
{
	mat4 model_matrix;

	uvec2 base_color_tx_hndl;
	uvec2 roughness_tx_hndl;
    uvec2 normal_tx_hndl;
};

layout(std430, binding = 0) readonly buffer PerDrawDataBuffer { PerDrawData per_draw_data[]; };

in vec3 position;
in vec2 uvCoord;
in mat3 tangent_space_matrix;

flat in int draw_id;

out vec4 frag_colour;

void main()
{
    sampler2D tx_hndl = sampler2D(per_draw_data[draw_id].base_color_tx_hndl);
    vec4 tx_col = texture(tx_hndl, uvCoord);

    //frag_colour = vec4(uvCoord,0.0,1.0);
    frag_colour = vec4(tx_col.rgb,1.0);
}