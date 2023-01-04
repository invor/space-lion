#version 430

uniform sampler2D twiddle_tx2D;
layout(rg32f, binding = 0) coherent uniform image2D src_x_tx2D;
layout(rg32f, binding = 1) coherent uniform image2D src_y_tx2D;
layout(rg32f, binding = 2) coherent uniform image2D src_z_tx2D;

layout(rg32f, binding = 3) coherent uniform image2D tgt_x_tx2D;
layout(rg32f, binding = 4) coherent uniform image2D tgt_y_tx2D;
layout(rg32f, binding = 5) coherent uniform image2D tgt_z_tx2D;

uniform int ifft_stage;
uniform int ifft_direction; // 0 = horizontal, 1 = vertical

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

vec2 multComplex(vec2 a, vec2 b)
{
	return vec2(a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y);
}

vec2 addComplex(vec2 a, vec2 b)
{
    return vec2( a.x + b.x, a.y + b.y);
}

vec2 subComplex(vec2 a, vec2 b)
{
    return vec2( a.x - b.x, a.y - b.y );
}

void vertical()
{
    vec2 gID = gl_GlobalInvocationID.xy;

    vec4 precomputed_data = texelFetch(twiddle_tx2D,ivec2(ifft_stage,int(gID.y)),0); 
    vec2 w = precomputed_data.xy;

    vec2 a = imageLoad(src_y_tx2D, ivec2(gID.x,precomputed_data.z) ).xy;
    vec2 b = imageLoad(src_y_tx2D, ivec2(gID.x,precomputed_data.w) ).xy;

    vec2 h = addComplex(a,multComplex(w,b));

    imageStore(tgt_y_tx2D, ivec2(gID), vec4(h,0.0,1.0));
}

void horizontal()
{
    vec2 gID = gl_GlobalInvocationID.xy;

    vec4 precomputed_data = texelFetch(twiddle_tx2D,ivec2(ifft_stage,int(gID.x)),0); 
    vec2 w = precomputed_data.xy;

    vec2 a = imageLoad(src_y_tx2D, ivec2(precomputed_data.z,gID.y) ).xy;
    vec2 b = imageLoad(src_y_tx2D, ivec2(precomputed_data.w,gID.y) ).xy;

    vec2 h = addComplex(a,multComplex(w,b));

    imageStore(tgt_y_tx2D, ivec2(gID), vec4(h,0.0,1.0));
}

void main()
{
    //  if(ifft_direction == 0)
    //      horizontal();
    //  else
    //      vertical();


    
    vec2 gID = gl_GlobalInvocationID.xy;

    vec4 precomputed_data = texelFetch(twiddle_tx2D,ivec2(ifft_stage,int(gID.y)),0); 

    int a_idx = int(precomputed_data.z);
    int b_idx = int(precomputed_data.w);
    vec2 w = precomputed_data.xy;

    int A_idx, B_idx;
    if(ifft_stage == 0)
    {
        A_idx = int(gID.y) * 2;
        B_idx = int(gID.y) * 2 + 1;
    }
    else
    {
        A_idx = int(a_idx);
        B_idx = int(b_idx);
    }

    //A_idx = int(gID.y) * 2;
    //B_idx = int(gID.y) * 2 + 1;

    ivec2 a_uv = ivec2(ifft_direction,1-ifft_direction) * ivec2(a_idx)
                + ivec2(1-ifft_direction,ifft_direction) * ivec2(gID.x);

    ivec2 b_uv = ivec2(ifft_direction,1-ifft_direction) * ivec2(b_idx)
                + ivec2(1-ifft_direction,ifft_direction) * ivec2(gID.x);

    vec2 a_x = imageLoad(src_x_tx2D, a_uv).xy;
    vec2 b_x = imageLoad(src_x_tx2D, b_uv).xy;

    vec2 a_y = imageLoad(src_y_tx2D, a_uv).xy;
    vec2 b_y = imageLoad(src_y_tx2D, b_uv).xy;

    vec2 a_z = imageLoad(src_z_tx2D, a_uv).xy;
    vec2 b_z = imageLoad(src_z_tx2D, b_uv).xy;

    vec2 A_x = addComplex(a_x, multComplex(w,b_x));
    vec2 A_y = addComplex(a_y, multComplex(w,b_y));
    vec2 A_z = addComplex(a_z, multComplex(w,b_z));

    vec2 B_x = subComplex(a_x, multComplex(w,b_x));
    vec2 B_y = subComplex(a_y, multComplex(w,b_y));
    vec2 B_z = subComplex(a_z, multComplex(w,b_z));

    ivec2 A_uv = ivec2(ifft_direction,1-ifft_direction) * ivec2(A_idx)
        + ivec2(1-ifft_direction,ifft_direction) * ivec2(gID.x);
    ivec2 B_uv = ivec2(ifft_direction,1-ifft_direction) * ivec2(B_idx)
        + ivec2(1-ifft_direction,ifft_direction) * ivec2(gID.x);


    imageStore(tgt_x_tx2D, A_uv, vec4(A_x,0.0,1.0));
    imageStore(tgt_x_tx2D, B_uv, vec4(B_x,0.0,1.0));

    imageStore(tgt_y_tx2D, A_uv, vec4(A_y,0.0,1.0));
    imageStore(tgt_y_tx2D, B_uv, vec4(B_y,0.0,1.0));

    imageStore(tgt_z_tx2D, A_uv, vec4(A_z,0.0,1.0));
    imageStore(tgt_z_tx2D, B_uv, vec4(B_z,0.0,1.0));
    
}