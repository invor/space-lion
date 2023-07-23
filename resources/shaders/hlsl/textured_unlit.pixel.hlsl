// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos     : SV_POSITION;
    float2 uv      : TEXCOORD0;
};

// A constant buffer that stores the model transform.
cbuffer ModelConstantBuffer : register(b0)
{
    float4x4 model;
};

cbuffer ViewProjectionConstantBuffer : register(b1)
{
    float4x4 viewProjection;
    float4x4 viewInv;
};

Texture2D albedo_texture : register(t0);
SamplerState texture_sampler_state : register(s0);

#define PI 3.1415926535
#define INV_PI 0.318309886183

// The pixel shader passes through the color data. The color data from 
// is interpolated and assigned to a pixel at the rasterization step.
min16float4 Main(PixelShaderInput input) : SV_TARGET
{
    return min16float4(albedo_texture.Sample(texture_sampler_state,input.uv));
}
