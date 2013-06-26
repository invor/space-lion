/*---------------------------------------------------------------------------------------------------
File: f_ftv_mask.glsl
Author: Michael Becher
Date of (presumingly) last edit: 26.06.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: This GLSL fragment shader is used to create a mask image and distance map that are
necessary for image inpainting and poission image editing.
---------------------------------------------------------------------------------------------------*/

#version 330

/*
/	Each texel of this texture represents a single inpainting region, given by its lower left and upper right corner:
/	Lower left corner is specified by the r and g channels, containing the x-coordinate and y-coordinate respectivly.
/	Upper right is specified by the b and a channels.
*/
uniform sampler1D inpaintingRegions_tx1D;

/*	Number of overall inpainting regions */
uniform float regionCount;

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


vec4 calcDistance(vec4 region)
{
	vec4 rtn;
	
	rtn.x = uvCoord.x - region.x;
	rtn.y = uvCoord.y - region.y;
	rtn.z = region.z - uvCoord.x;
	rtn.w = region.w - uvCoord.y;
	
	return rtn;
}

void main()
{	
	inpaintingMask = vec2(1.0);
	distanceMap = vec4(0.0);
	
	/*
	/	Check all regions
	*/
	for(float i = 0.0f; i < regionCount; i++)
	{
		vec4 currentRegion = texture1D(inpaintingRegions_tx1D, i/regionCount);
			
		if( (uvCoord.x >= currentRegion.x) && (uvCoord.x <= currentRegion.z) &&
			(uvCoord.y >= currentRegion.y) && (uvCoord.y <= currentRegion.w) )
		{
			distanceMap = calcDistance(currentRegion);
			
			float minXDist = min(distanceMap.x,distanceMap.z);
			float minYDist = min(distanceMap.y,distanceMap.w);
			inpaintingMask = vec2(0.0,0.0);
			break;
		}
	}
}