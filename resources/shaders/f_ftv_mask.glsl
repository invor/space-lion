#version 330

uniform sampler1D inpaintingRegions;
uniform float regionCount;

/*
/	Normalized coordinates [0,1]x[0,1] of the fragment.
*/
in vec2 uvCoord;

/*
/	Fragment shader output variable.
*/
out vec4 inpaintingMask;
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
	inpaintingMask = vec4(1.0);
	distanceMap = vec4(0.0);
	
	/*
	/	Check all regions
	*/
	for(float i = 0.0f; i < regionCount; i++)
	{
		vec4 currentRegion = texture1D(inpaintingRegions, i/regionCount);
			
		if( (uvCoord.x >= currentRegion.x) && (uvCoord.x <= currentRegion.z) &&
			(uvCoord.y >= currentRegion.y) && (uvCoord.y <= currentRegion.w) )
		{
			inpaintingMask = vec4(0.0,0.0,0.0,1.0);
			distanceMap = calcDistance(currentRegion);
			break;
		}
	}
}