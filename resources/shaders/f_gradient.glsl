#version 330

uniform sampler2D inputImage;

/*
/	Contains the distance between two pixels/texels.
*/
uniform vec2 h;

/*	Normalized coordinates [0,1]x[0,1] of the fragment */
in vec2 uvCoord;

out vec2 gradient;

float luminance(vec4 color)
{
	return (0.2126f*color.r + 0.7152f*color.g + 0.0722f*color.b);
}

void main()
{
	/*
	/	Get stencil values and use luminance values instead of seperate
	/	gradients for each color channel.
	*/
	float lumN = luminance( texture(inputImage, uvCoord + vec2(0.0,h.y)) );
	float lumNE = luminance( texture(inputImage, uvCoord + vec2(h.x,h.y)) );
	float lumE = luminance( texture(inputImage, uvCoord + vec2(h.x,0.0)) );
	float lumSE = luminance( texture(inputImage, uvCoord + vec2(h.x,-h.y)) );
	float lumS = luminance( texture(inputImage, uvCoord + vec2(0.0,-h.y)) );
	float lumSW = luminance( texture(inputImage, uvCoord + vec2(-h.x,-h.y)) );
	float lumW = luminance( texture(inputImage, uvCoord + vec2(-h.x,0.0)) );
	float lumNW = luminance( texture(inputImage, uvCoord + vec2(-h.x,h.y)) );
	
	/*	
	/	Compute gradient using sobel masks
	/
	/	 --------         --------
	/	| 1  2  1|       | 1    -1|
	/	|        |  and  | 2    -2| 
	/	|-1 -2 -1|       | 1    -1|
	/	 --------         --------
	*/
	float dx = lumNW + 2.0*lumW + lumSW - lumNE - 2.0*lumE - lumSE;
	float dy = lumNW + 2.0*lumN + lumNE - lumSW - 2.0*lumS - lumSE;


	gradient = vec2(dx,dy);
}