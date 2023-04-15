#version 430

layout(rg32f, binding = 0) readonly uniform image2D ifft_x_tx2D;
layout(rg32f, binding = 1) readonly uniform image2D ifft_y_tx2D;
layout(rg32f, binding = 2) readonly uniform image2D ifft_z_tx2D;

layout(rg32f, binding = 3) readonly uniform image2D ifft_x_b_tx2D;
layout(rg32f, binding = 4) readonly uniform image2D ifft_y_b_tx2D;
layout(rg32f, binding = 5) readonly uniform image2D ifft_z_b_tx2D;

layout(rgba32f, binding = 6) writeonly uniform image2D displacement_tx2D;

uniform uint grid_size;

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

void main()
{
    ivec3 gID = ivec3(gl_GlobalInvocationID.xyz);

    vec2 ifft_x = imageLoad(ifft_x_tx2D,gID.xy).xy;
    vec2 ifft_y = imageLoad(ifft_y_tx2D,gID.xy).xy;
    vec2 ifft_z = imageLoad(ifft_z_tx2D,gID.xy).xy;

    vec2 ifft_b_x = imageLoad(ifft_x_b_tx2D,gID.xy).xy;
    vec2 ifft_b_y = imageLoad(ifft_y_b_tx2D,gID.xy).xy;
    vec2 ifft_b_z = imageLoad(ifft_z_b_tx2D,gID.xy).xy;

    float N2 = float(grid_size)*float(grid_size);
    int sign_correction = bool((gID.x + gID.y) & 1) ? -1 : 1;

    vec3 displacement = vec3( -ifft_b_x.x, ifft_b_y.x, -ifft_b_z.x) * float(sign_correction) * 50.0 /N2;

    imageStore(displacement_tx2D, gID.xy, vec4( displacement ,1.0));
}