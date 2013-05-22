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
	vec3 fragmentTextureCoord = colour;
	vec3 cameraTextureCoord = (textureMatrix * vec4(cameraPosition,1.0)).xyz;

	/*	Compute ray direction in texture space. */
	vec3 rayDirection = normalize(fragmentTextureCoord-cameraTextureCoord);
	
	/*
	/	Set the entry (or start) point based on the camera position.
	/
	/	If the camera is outside the volume bounding box, set the entry point
	/	to the colour of the fragment (which represents the position of the 
	/	surface point in texture space).
	/	If the camera is inside though, set the entry point to the position
	/	of the camera, since this is where we want to start to collect samples
	/	from the volume.
	*/
	vec3 entryPoint;
	if( (max(max(cameraTextureCoord.x,cameraTextureCoord.y),cameraTextureCoord.z))>1.0f ||
		(min(min(cameraTextureCoord.x,cameraTextureCoord.y),cameraTextureCoord.z))<0.0f )
	{
		entryPoint = colour;
	}
	else
	{
		entryPoint = cameraTextureCoord;
	}
	
	/*	Storage for output color. */
	vec4 rgbaOut = vec4(0.0);
	
	/*	Used to store the so far traveled distance */
	float traveledDistance = 0.0;
	/*	Normalisation factor for color and opacity accumulation */
	float density = 0.005/(1.0/91.0);
	/* calculate the distance from entry to exit point */
	float exitDistance = min(	min(	max((1.0 - entryPoint.x)/ rayDirection.x , entryPoint.x/(-rayDirection.x)),
										max((1.0 - entryPoint.y)/ rayDirection.y , entryPoint.y/(-rayDirection.y))	),
								max((1.0 - entryPoint.z)/ rayDirection.z , entryPoint.z/(-rayDirection.z))	);
	
	while(rgbaOut.a < 0.99)
	{	
		/*	Obtain values from the 3d texture at the current location. */
		vec3 sampleCoord = entryPoint+(rayDirection*traveledDistance);
		vec4 rgbaVol = texture3D(volumeTexture,sampleCoord);
		
		/*	Accumulate color */
		rgbaOut.rgb += (1.0 - rgbaOut.a) * rgbaVol.rgb * rgbaVol.a * density;
		/*	Accumulate opacity */
		rgbaOut.a += (1.0 - rgbaOut.a) * rgbaVol.a * density;
		
		/*	Add the distance to the next sample point to the so far traveled distance */
		traveledDistance += 0.005;
		
		/*	Check if the exit point has been reached */
		if( traveledDistance > exitDistance )
		{
			break;
		}
	}
	
	fragColour = vec4(rgbaOut);
}