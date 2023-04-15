#version 430

#define PI 3.14159265359

// Buffer containing (bit reversed) indices of first stage
layout(std430, binding = 0 ) buffer FFT_Indices { int indices[ ]; } ;

layout(rgba32f, binding = 0) writeonly uniform image2D twiddle_tx2D;

uniform uint grid_size;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

vec2 multComplex(vec2 a, vec2 b)
{
	return vec2(a[0] * b[0] - a[1] * b[1], a[1] * b[0] + a[0] * b[1]);
}

void main()
{
	// [0, grid_size-1]
	vec2 uv = gl_GlobalInvocationID.xy;
    ivec2 iuv = ivec2(uv);

    /*
    float k = mod( uv.y * (float(grid_size)/pow(2.0, uv.x+1)), float(grid_size));
    vec2 w = vec2( cos(2.0*PI*k/float(grid_size)), sin(2.0*PI*k/float(grid_size)) );

    int offset = int(pow(2, iuv.x ));

    int butterflywing;

    if( mod(uv.y, pow(2, uv.x +1)) < pow(2,uv.x) )
        butterflywing = 1;
    else
        butterflywing = 0;

    if(iuv.x == 0)
    {
        if(butterflywing==1)
            imageStore(twiddle_tx2D, ivec2(uv), vec4(w, indices[int(uv.y)],indices[int(uv.y+1)]));
        else
            imageStore(twiddle_tx2D, ivec2(uv), vec4(w, indices[int(uv.y-1)],indices[int(uv.y)]));
    }
    else
    {
        if(butterflywing==1)
            imageStore(twiddle_tx2D, ivec2(uv), vec4(w, uv.y, uv.y + offset));
        else
            imageStore(twiddle_tx2D, ivec2(uv), vec4(w, uv.y - offset, uv.y));
    }
    */   

    float k = mod( uv.y * pow(2.0, (log2(grid_size)-1) - uv.x) , grid_size/2);
    vec2 w = vec2( cos( (2.0*PI*k)/ float( grid_size ) ), sin( (2.0*PI*k)/ float( grid_size ) ) );

    int offset = int(pow(2, iuv.x ));

    if(iuv.x == 0)
    {
        imageStore(twiddle_tx2D, ivec2(uv), vec4(w, indices[int(uv.y*2)],indices[int(uv.y*2)+1]));
    }
    else
    {
        float index = uv.y + (floor( uv.y / offset) * offset);
        imageStore(twiddle_tx2D, ivec2(uv), vec4(w, index, index + offset));
    }

}