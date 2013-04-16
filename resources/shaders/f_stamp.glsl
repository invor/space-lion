#version 330

uniform sampler2D inputImage;
uniform sampler2D mask;
uniform vec2 imgDim;

//	Normalized coordinates [0,1]x[0,1] of the fragment
in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	vec2 h = vec2(1.0f/imgDim.x, 1.0f/imgDim.y);
	
	/*
	/	Vectors to the neighbour texels
	*/
	vec2 vN = vec2(0.0f,h.y);
	vec2 vW = vec2(-h.x,0.0f);
	vec2 vE = vec2(h.x,0.0f);
	vec2 vS = vec2(0.0f,-h.y);
	
	/*
	/	Color values of the stencil
	*/
	vec3 rgbN = texture2D(mask,uvCoord+vN).xyz;
	vec3 rgbW = texture2D(mask,uvCoord+vW).xyz;
	vec3 rgbM = texture2D(mask,uvCoord).xyz;
	vec3 rgbE = texture2D(mask,uvCoord+vE).xyz;
	vec3 rgbS = texture2D(mask,uvCoord+vS).xyz;
	
	vec3 rgbSum = rgbN + rgbW + rgbM + rgbE + rgbS;
	
	if(rgbM.x < 0.5f)
	{
		fragColour = vec4(0.0,0.0,0.0,1.0);
	}
	else
	{
		fragColour = vec4(texture2D(inputImage,uvCoord).xyz,1.0);
	}
}