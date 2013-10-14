/*---------------------------------------------------------------------------------------------------
File: f_ftv_shrinkMask.glsl
Author: Michael Becher
Date of (presumingly) last edit: 28.06.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: 
---------------------------------------------------------------------------------------------------*/

#version 330

/*	Inpainting mask */
uniform sampler2D mask_tx2D;
/*	Disance Map */
uniform sampler2D distance_tx2D;

/*
/	Specifies the distance between two pixels in x and y direction
/	(in texture space [0,1] )
*/
uniform vec2 h;

/*	Normalized coordinates [0,1]x[0,1] of the fragment */
in vec2 uvCoord;

/*	Fragment shader output variables */
out vec2 inpaintingMask;
out vec4 distanceMap;


void main()
{	
	distanceMap = vec4(1.0);
	inpaintingMask = vec2(1.0);

	/*	Check if the fragment is inside an inpainting region */
	if(texture(mask_tx2D,uvCoord).x < 0.5)
	{
		/*	Start with decreasing all distances by one texel */
		vec4 currDists = texture(distance_tx2D, uvCoord);
		distanceMap = vec4( currDists.x-h.x,
							currDists.y-h.y,
							currDists.z-h.x,
							currDists.w-h.y );
							
							
		float minXDist = min(distanceMap.x,distanceMap.z);
		float minYDist = min(distanceMap.y,distanceMap.w);
		
		/*	Check if the fragment moved to the edge of the inpainting region */
		if(minXDist < 0.0 || minYDist < 0.0) inpaintingMask = vec2(1.0,1.0);
		else if(minXDist < h.x || minYDist < h.y) inpaintingMask = vec2(0.0,0.0);
		else inpaintingMask = vec2(0.0,1.0);
	}
}