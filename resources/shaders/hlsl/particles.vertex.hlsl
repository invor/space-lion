/// <copyright>
/// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>
/// <summary>
/// Computes a matrix to orient a sprite or hemisphere towards the camera.
/// </summary>
/// <param name="objPos">The world-space position of the sprite to be rotated
/// towards the camera.</param>
/// <param name="viewInvMatrix">The inverse view matrix to reconstruct the
/// camera position and orientation.</param>
/// <returns>A matrix orienting the sprite towards the camera.</returns>
float4x4 OrientToCamera(const in float3 objPos,
        const in float4x4 viewInvMatrix) {
    float3 camPos = viewInvMatrix._14_24_34;
    float3 camUp = viewInvMatrix._12_22_32;

    float3 v = normalize(objPos - camPos);
    float3 u = normalize(camUp);
    float3 r = normalize(cross(v, u));
    u = normalize(cross(r, v));

    // http://richiesams.blogspot.de/2014/05/hlsl-turning-float4s-into-float4x4.html
    float4x4 retval = float4x4(
        r.x, u.x, v.x, 0.0f,
        r.y, u.y, v.y, 0.0f,
        r.z, u.z, v.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return retval;
}



// Per-vertex data passed to the geometry shader.
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 colour : COLOR0;
    float4 gylph_space_position : TEXCOORD0;
    float4 sphere_params : TEXCOORD1; // world-space position (xyz) and radius (w) of the particle.
    nointerpolation float4 cam_position : TEXCOORD2; // glyph-space camera position
    //nointerpolation float4 cam_direction : TEXCOORD3;
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
    float4x4 ViewProjection;
};

struct Particle
{
    float3 position;
    float radius;
};

StructuredBuffer<Particle> particles : register(t2);

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    uint id : SV_VertexID;
};

// Simple shader to do vertex processing on the GPU.
VertexShaderOutput Main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    uint particle_idx = input.id / 6;
    uint vertex_in_quad = input.id % 6;
    
    float3 position = float3(0.0, 0.0, 0.0);
    const float4 verts[6] = { 
        float4(-1,1,0,0),
        float4(1,1,1,0),
        float4(-1,-1,0,1),
        float4(-1,-1,0,1),
        float4(1,1,1,0),
        float4(1,-1,1,1)
    };

    position.xy = verts[vertex_in_quad].xy * particles[particle_idx].radius;
    
    float3 particle_world_position = mul(float4(particles[particle_idx].position, 1.0), model).xyz;
    
    // Orient the sprite towards the camera.
    float4x4 matOrient = OrientToCamera(particle_world_position, transpose(InvView));
    position = mul(matOrient, float4(position,1.0)).xyz;

    // Move sprite to world position.
    position += particle_world_position;
    position -= matOrient._13_23_33 * particles[particle_idx].radius;
    
    output.gylph_space_position = float4(position - particle_world_position, 1.0);
    
    output.position = mul(float4(position, 1.0), ViewProjection);
    output.colour = color;
    output.sphere_params = float4(particle_world_position, particles[particle_idx].radius);
    output.cam_position = mul(float4(0.0, 0.0, 0.0, 1.0), InvView) - float4(particle_world_position, 0.0);

    return output;
}
