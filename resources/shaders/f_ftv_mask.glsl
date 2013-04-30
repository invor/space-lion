#version 330

uniform sampler1D faultRegions;
uniform vec2 imgDim;

/*
/	Normalized coordinates [0,1]x[0,1] of the fragment.
*/
in vec2 uvCoord;

/*
/	Fragment shader output variable.
*/
out vec4 fragColour;


vec4 calcDistance(float region)
{
	vec2 h = vec2(1.0f/imgDim.x, 1.0f/imgDim.y);
	
	vec4 rtn;
	
	float verticalTraveledDist = 0.0f;
	float horizontalTraveledDist = 0.0f;
	vec2 verticalPos_n = uvCoord;
	vec2 verticalPos_p = uvCoord;
	vec2 horizontalPos_n = uvCoord;
	vec2 horizontalPos_p = uvCoord;
	bool verticalSet_n = false;
	bool verticalSet_p = false;
	bool horizontalSet_n = false;
	bool horizontalSet_p = false;
	
	while ( (verticalPos_n.y > 0.0) && (verticalPos_p.y < 1.0) && (horizontalPos_n.x > 0.0) && (horizontalPos_p.x < 1.0) )
	{
		verticalPos_n.y -= h.y;
		verticalPos_p.y += h.y;
		horizontalPos_n.x -= h.x;
		horizontalPos_p.x += h.x;
		
		verticalTraveledDist += h.y;
		horizontalTraveledDist += h.x;
		
		if( (texture1D(faultRegions,region).x > 0.5f) && (!verticalSet_p) )
		{
			rtn.x = verticalTraveledDist;
			verticalSet_p = true;
		}
		if( (texture1D(faultRegions,region).y > 0.5f) && (!horizontalSet_p) )
		{
			rtn.y = horizontalTraveledDist;
			horizontalSet_p = true;
		}
		if( (texture1D(faultRegions,region).z > 0.5f) && (!verticalSet_n) )
		{
			rtn.z = verticalTraveledDist;
			verticalSet_n = true;
		}
		if( (texture1D(faultRegions,region).w > 0.5f) && (!horizontalSet_n) )
		{
			rtn.w = horizontalTraveledDist;
			horizontalSet_n = true;
		}
	}
	
	return rtn;
}

void main()
{
	fragColour = vec4(0.0,0.0,0.0,1.0);
}