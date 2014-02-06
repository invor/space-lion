#version 430

#define TESS_LVL 12

layout (vertices = 4) out;

uniform float inner_tess_lvl;
uniform float outer_tess_lvl;

in vec3 position[];

out vec3 eval_position[];

void main()
{
	eval_position[gl_InvocationID] = position[gl_InvocationID];
	
	//gl_TessLevelInner[0] = inner_tess_lvl;
	//gl_TessLevelInner[1] = inner_tess_lvl;
	//gl_TessLevelOuter[0] = outer_tess_lvl;
	//gl_TessLevelOuter[1] = outer_tess_lvl;
	//gl_TessLevelOuter[2] = outer_tess_lvl;
	//gl_TessLevelOuter[3] = outer_tess_lvl;
	//
	
	gl_TessLevelInner[0] = TESS_LVL;
	gl_TessLevelInner[1] = TESS_LVL;
	gl_TessLevelOuter[0] = TESS_LVL;
	gl_TessLevelOuter[1] = TESS_LVL;
	gl_TessLevelOuter[2] = TESS_LVL;
	gl_TessLevelOuter[3] = TESS_LVL;
}