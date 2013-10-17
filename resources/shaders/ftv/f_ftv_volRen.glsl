#version 330

/* 	matrix that transforms a set of coordinates from world space to texture space */
uniform mat4 textureMatrix;
/* 	camera position in world space */
uniform vec3 cameraPosition;
/*	texture that houses the volume data */
uniform sampler3D volumeTexture;
/*	texture that housed the ftv mask */
uniform sampler3D mask;

in vec3 fragmentWorldPosition;
in vec3 colour;

out vec4 fragColour;


vec2 intersection(vec3 ray_direction, vec3 ray_origin, vec3 lower_box_vertex, vec3 upper_box_vertex)
{
	// compute intersection of ray with all six box planes
    vec3 invR = vec3(1.0) / ray_direction;
    vec3 tbot = invR * (lower_box_vertex - ray_origin);
    vec3 ttop = invR * (upper_box_vertex - ray_origin);
	

    // re-order intersections to find smallest and largest on each axis
    vec3 tmin;
    tmin.x = min(ttop.x, tbot.x);
    tmin.y = min(ttop.y, tbot.y);
    tmin.z = min(ttop.z, tbot.z);
    vec3 tmax;
    tmax.x = max(ttop.x, tbot.x);
    tmax.y = max(ttop.y, tbot.y);
    tmax.z = max(ttop.z, tbot.z);

    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

	float tnear = largest_tmin;
	float tfar = smallest_tmax;

	return vec2(tnear,tfar);
}

bool planeIntersection(vec3 ray_origin, vec3 ray_direction, vec3 plane_center, vec3 plane_normal, out float t_intersection, out vec3 intersection_point)
{
	t_intersection = dot(plane_normal,(plane_center-ray_origin)) / dot(plane_normal,ray_direction);
	
	intersection_point = ray_origin + t_intersection*ray_direction;
	
	return (t_intersection > 0.0);
}

vec3 lighting(vec3 surface_point, vec3 surface_normal, vec3 surface_color, vec3 light_position)
{
	return vec3(1.0);
}

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
	

	/*	Intersect with bounding box around fault region */
	vec2 intersections = intersection(rayDirection,entryPoint,vec3(0.0),vec3(0.5));	
	/* Failsafe for fault region bounding box rendering */
	if(intersections.y < intersections.x) intersections = vec2(exitDistance+1.0);
	
	
	
	float clip_plane_intersection_t;
	vec3 clip_plane_intersection_point;;
	vec3 clip_plane_center = vec3(1.5);
	vec3 clip_plane_normal = vec3(0.0,1.0,0.0);
	planeIntersection(entryPoint,rayDirection,clip_plane_center,clip_plane_normal,clip_plane_intersection_t,clip_plane_intersection_point);
	
	/*	Manipulate entryPoint and exitDistance so that the sampling loop will only run for sampley below the clip-plane. */
	if( dot(normalize(entryPoint-clip_plane_center),clip_plane_normal) > 0.0)
	{
		if(clip_plane_intersection_t<0.0)
		{
			exitDistance = -1.0;
		}
		else
		{
			entryPoint = clip_plane_intersection_point;
			exitDistance -= clip_plane_intersection_t;
		}
	}
	else
	{
		if(clip_plane_intersection_t>0.0)
		{
			exitDistance = min(clip_plane_intersection_t,exitDistance);
		}
	}
	
	//rgbaOut = vec4(vec3(dot(test,clip_plane_normal)),1.0);
	
	while(rgbaOut.a < 0.99)
	{	
		/*	Check if the exit point has been reached */
		if( traveledDistance > exitDistance )
		{
			break;
		}
		
		/*	Obtain values from the 3d texture at the current location. */
		vec3 sampleCoord = entryPoint+(rayDirection*traveledDistance);
		vec4 rgbaVol = vec4(texture(volumeTexture,sampleCoord).x);
		
		//float maskValue = texture3D(mask,sampleCoord).x;
		//if( maskValue > 0.5 )
		//{
		//	rgbaVol = vec4(1.0,0.0,0.0,0.025);
		//}
		
		/*	A simple but costly solution for "discarding" all sample above the clip plane. */
		//	if( dot(normalize(sampleCoord-clip_plane_center),clip_plane_normal) > 0.0)
		//		rgbaVol.a = 0.0;
		
		/*	Accumulate color */
		rgbaOut.rgb += (1.0 - rgbaOut.a) * rgbaVol.rgb * rgbaVol.a * density;
		/*	Accumulate opacity */
		rgbaOut.a += (1.0 - rgbaOut.a) * rgbaVol.a * density;
		
		/*	Add the distance to the next sample point to the so far traveled distance */
		traveledDistance += 0.0025;
		
		/* Check against bb intersection */
		//if(traveledDistance > intersections.x)
		//{
		//	rgbaOut.rgb += (1.0 - rgbaOut.a) * vec3(1.0);
		//	rgbaOut.a = 1.0;
		//}
	}
	
	fragColour = vec4(rgbaOut);
	
}