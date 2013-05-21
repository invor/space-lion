#version 330

/* 	matrix that transforms a set of coordinates from world space to texture space */
uniform mat4 textureMatrix;
/* 	camera position in world space */
uniform vec3 cameraPosition;
/*	texture that houses the volume data */
uniform sampler3D volumeTexture;

in vec3 fragmentWorldPosition;
in vec3 colour;

out vec4 fragColour;

void main()
{
	/*
	/	Compute camera and fragment position in texture space.
	/	That is, the space in which the bounding box ranges from
	/	(0,0,0)^T to (1,1,1)^T.
	*/
	vec3 fragmentTextureCoord = (textureMatrix * vec4(fragmentWorldPosition,1.0)).xyz;
	//vec3 fragmentTextureCoord = colour;
	vec3 cameraTextureCoord = (textureMatrix * vec4(cameraPosition,1.0)).xyz;

	/*	Compute ray direction in texture space. */
	vec3 rayDirection = normalize(fragmentTextureCoord-cameraTextureCoord);
	
	/*	Storage for output color. */
	vec4 rgbaOut = vec4(0.0);
	
	/*	Helpers for traveling through the volume */
	float traveledDistance = 0.0;
	float opacity = 0.0;
	
	while(opacity < 0.99)
	{	
		/*	Obtain values from the 3d texture at the current location. */
		vec3 sampleCoord = colour+(rayDirection*traveledDistance);
		vec4 rgbaVol = texture3D(volumeTexture,sampleCoord);
		
		/*	Accumulate color */
		rgbaOut += (1.0 - opacity) * rgbaVol;
		/*	Accumulate opacity */
		opacity += (1.0 - opacity) * rgbaVol.w;
		
		traveledDistance += 0.005;
		
		//TODO: Speedup the test for the break condition
		//Check if still inside bounding box
		if( sampleCoord.x > 1.0
			|| sampleCoord.y > 1.0
			|| sampleCoord.z > 1.0		
			|| sampleCoord.x < 0.0
			|| sampleCoord.y < 0.0
			|| sampleCoord.z < 0.0)
		{
			break;
		}
	}
	
	//fragColour = vec4(fragmentTextureCoord.xyz,1.0);
	fragColour = vec4(rgbaOut);
}