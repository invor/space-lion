// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 colour : COLOR0;
    float4 gylph_space_position : TEXCOORD0;
    float4 sphere_params : TEXCOORD1; // world-space position (xyz) and radius (w) of the particle.
    nointerpolation float4 cam_position : TEXCOORD2; // glyph-space camera position
};

// A constant buffer that stores the model transform.
cbuffer ModelConstantBuffer : register(b0)
{
    float4x4 model;
    float4 color;

    float4 padding0_1;
    float4 padding0_2;
    float4 padding0_13;
    float4x4 padding1;
    float4x4 padding2;
};

// A constant buffer that stores each set of view and projection matrices in column-major format.
cbuffer ViewProjectionConstantBuffer : register(b1)
{
    float4x4 InvView;
    float4x4 InvProj;
    float4x4 ViewProjection;
};

//struct Particle
//{
//    float3 position;
//    float radius;
//};
//
//StructuredBuffer<Particle> particles : register(t2);

float3 LocalLighting(const in float3 ray, const in float3 normal,
    const in float3 light_dir, const in float3 colour)
{
    float nDOTl = dot(normal, light_dir);

    float3 r = normalize(2.0 * nDOTl * normal - light_dir);
    return 0.1 * colour
        + 0.4 * colour * max(nDOTl, 0.0)
        + 0.3 * float3(pow(max(dot(r, -ray), 0.0), 10.0).xxx);
}

struct PsOutput {
    float4 colour : SV_TARGET;
    float depth : SV_DEPTH;
};

// The pixel shader passes through the color data. The color data from
// is interpolated and assigned to a pixel at the rasterization step.
PsOutput Main(PixelShaderInput input) : SV_TARGET
{
    PsOutput retval = (PsOutput) 0;

    float3 normal;
    //float4 light_dir = normalize(input.cam_direction);
    float4 light_dir = float4(1.0,1.0,1.0,0.0);

    float4 cam_pos = input.cam_position;
    float4 sphere_pos = float4(input.sphere_params.xyz, 1.0);
    float sphere_rad = input.sphere_params.w;
    float sphere_rad_squared = sphere_rad * sphere_rad;

    // Compute the viewing ray.
    float3 ray = normalize(input.gylph_space_position.xyz - cam_pos.xyz);

    // calculate the geometry-ray-intersection
    float d1 = -dot(cam_pos.xyz, ray);                      // projected length of the cam-SphereParams-vector onto the ray
    float d2s = dot(cam_pos.xyz, cam_pos.xyz) - d1 * d1;    // off axis of cam-SphereParams-vector and ray
    float radicand = sphere_rad_squared - d2s;              // square of difference of projected length and lambda
    float lambda = d1 - sqrt(radicand);                     // lambda
    float3 sphere_intersection = 0.0.xxx;

    if ((radicand < 0.0f) || (lambda < 0.0f))
    {
//#define FILL_BILLBOARD
#if defined(FILL_BILLBOARD)
        retval.colour = float4(1.0, 0.0, 1.0, 1.0);
#else /* defined(FILL_BILLBOARD) */
        discard;
#endif /* defined(FILL_BILLBOARD) */
    } else {
        // chose color for lighting
        sphere_intersection = lambda * ray + cam_pos.xyz;    // intersection point
                                                           // "calc" normal at intersection point
        normal = sphere_intersection / sphere_rad;

        float4 baseColour = input.colour;
        retval.colour = baseColour;
        retval.colour = float4(LocalLighting(ray, normal, light_dir.xyz, baseColour.rgb), baseColour.a);
    }

    // calculate depth
    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math#matrix-ordering
    float4 intPos = float4(sphere_intersection + sphere_pos.xyz, 1.0);
    float dz = dot(ViewProjection._13_23_33_43, intPos);
    float dw = dot(ViewProjection._14_24_34_44, intPos);
    retval.depth = (dz/dw);

    return retval;
}
