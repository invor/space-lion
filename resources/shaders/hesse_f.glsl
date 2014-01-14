/*
---------------------------------------------------------------------------------------------------
File: f_hesse.glsl
Author: Michael Becher
Date of (presumingly) last edit: 21.06.2013

This shader program is part of the space-lion framework.

Description: This GLSL fragment shader computes the hessian matrix from a given gradient vector.
---------------------------------------------------------------------------------------------------
*/

#version 330

/*	Texture containing the a gradient in the r and g channel */
uniform sampler2D inputImage;

/*	Contains the distance between two pixels/texels */
uniform vec2 h;

/*	Normalized coordinates [0,1]x[0,1] of the fragment */
in vec2 uvCoord;

out vec4 hesse;

void main()
{
	/*	Get stencil values */
	vec2 gradN = texture(inputImage, uvCoord + vec2(0.0,h.y)).xy;
	vec2 gradNE = texture(inputImage, uvCoord + vec2(h.x,h.y)).xy;
	vec2 gradE = texture(inputImage, uvCoord + vec2(h.x,0.0)).xy;
	vec2 gradSE = texture(inputImage, uvCoord + vec2(h.x,-h.y)).xy;
	vec2 gradS = texture(inputImage, uvCoord + vec2(0.0,-h.y)).xy;
	vec2 gradSW = texture(inputImage, uvCoord + vec2(-h.x,-h.y)).xy;
	vec2 gradW = texture(inputImage, uvCoord + vec2(-h.x,0.0)).xy;
	vec2 gradNW = texture(inputImage, uvCoord + vec2(-h.x,h.y)).xy;
	
	/*	
	/	Compute 2nd derivative using sobel masks
	/
	/	 --------         --------
	/	| 1  2  1|       | 1    -1|
	/	|        |  and  | 2    -2| 
	/	|-1 -2 -1|       | 1    -1|
	/	 --------         --------
	*/
	float dxx = gradNW.x + 2.0*gradW.x + gradSW.x - gradNE.x - 2.0*gradE.x - gradSE.x;
	float dxy = gradNW.x + 2.0*gradN.x + gradNE.x - gradSW.x - 2.0*gradS.x - gradSE.x;
	float dyy = gradNW.y + 2.0*gradN.y + gradNE.y - gradSW.y - 2.0*gradS.y - gradSE.y;

	hesse = vec4(dxx,dxy,dxy,dyy);
}