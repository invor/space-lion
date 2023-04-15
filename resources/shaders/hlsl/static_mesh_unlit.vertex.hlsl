// Per-vertex data passed to the geometry shader.
struct VertexShaderOutput
{
    float4 pos     : SV_POSITION;
    float2 uv      : TEXCOORD0;
};

// A constant buffer that stores the model transform.
cbuffer ModelConstantBuffer : register(b0)
{
    float4x4 model;
};

// A constant buffer that stores each set of view and projection matrices in column-major format.
cbuffer ViewProjectionConstantBuffer : register(b1)
{
    float4x4 viewProjection;
    float4x4 viewInv;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 pos     : POSITION;
    float2 uv      : TEXCOORD0;
};

// Simple shader to do vertex processing on the GPU.
VertexShaderOutput Main(VertexShaderInput input)
{
    VertexShaderOutput output;

    // Transform the vertex position into world space.
    float4 pos = mul(float4(input.pos, 1.0f), model);

    // Correct for perspective and project the vertex position onto the screen.
    pos = mul(pos, viewProjection);
    output.pos = pos;
    output.uv = input.uv;

    return output;
}
