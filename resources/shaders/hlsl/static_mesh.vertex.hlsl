// Per-vertex data passed to the geometry shader.
struct VertexShaderOutput
{
    float4 pos     : SV_POSITION;
    float3 normal  : NORMAL;
    float3 camPos  : TEXCOORD0; //feels so hacky
    float3 worldPos: TEXCOORD1; //feels so hacky
};

// A constant buffer that stores the model transform.
cbuffer ModelConstantBuffer : register(b0)
{
    float4x4 model;

    float4x4 normalMatrix;

    float4   albedo_colour;
    float4   specular_colour;
    float    roughness;
    float3   padding10;
    float4   padding11;

    float4x4 padding2;
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
    float3 normal  : NORMAL;
    float3 pos     : POSITION;
    float4 tangent : TANGENT;
    float2 uv      : TEXCOORD0;
};

// Simple shader to do vertex processing on the GPU.
VertexShaderOutput Main(VertexShaderInput input)
{
    VertexShaderOutput output;

    // Transform the vertex position into world space.
    float4 pos = mul(float4(input.pos, 1.0f), model);
    output.worldPos = pos.xyz;

    // Correct for perspective and project the vertex position onto the screen.
    pos = mul(pos, viewProjection);
    output.pos = pos;

    // Transform normal (to world space)
    output.normal = normalize(mul((float3x3)normalMatrix, normalize(input.normal)));

    // Retrieve camera position from view matrix
    output.camPos = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), viewInv).xyz;

    return output;
}
