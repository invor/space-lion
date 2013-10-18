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


#define M_PI 3.1415926535897932384626433832795

struct Ray
{
  vec3 origin;
  vec3 direction;
};

struct Triangle
{
  vec3 v0;
  vec3 v1;
  vec3 v2;
  vec3 normal;
  vec4 colour;
};

Triangle createTriangle(vec3 v0, vec3 v1, vec3 v2, vec4 colour)
{
	Triangle new_triangle;
	
	new_triangle.v0 = v0;
	new_triangle.v1 = v1;
	new_triangle.v2 = v2;
	new_triangle.normal = normalize(cross(v0 - v2, v1 - v2));
	new_triangle.colour = colour;
	
	return new_triangle;
}

vec2 boundingBoxIntersection(Ray ray, vec3 lower_box_vertex, vec3 upper_box_vertex)
{
	// compute intersection of ray with all six box planes
    vec3 invR = vec3(1.0) / ray.direction;
    vec3 tbot = invR * (lower_box_vertex - ray.origin);
    vec3 ttop = invR * (upper_box_vertex - ray.origin);
	

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

bool planeIntersection(Ray ray, vec3 plane_center, vec3 plane_normal, out float t_intersection, out vec3 intersection_point)
{
	t_intersection = dot(plane_normal,(plane_center-ray.origin)) / dot(plane_normal,ray.direction);
	
	intersection_point = ray.origin + t_intersection*ray.direction;
	
	return (t_intersection > 0.0);
}

bool triangleIntersection(Ray ray, Triangle triangle, out float t_intersection)
{
	t_intersection = 0.0;
	
	float z = dot(ray.origin - triangle.v0, triangle.normal);
	float n = dot(ray.direction, triangle.normal);
	
	if (abs(n) > 1.0e-6)
	{
		t_intersection = -(z / n);
		
		/*	Compute the intersections point */
		vec3 intersectPoint = ray.origin + t_intersection * ray.direction;
	
		/*	
		/	Compute the vectors from the intersection point to the triangle
		/	edges.
		*/
		vec3 diffVec0 = normalize(triangle.v0 - intersectPoint);
		vec3 diffVec1 = normalize(triangle.v1 - intersectPoint);
		vec3 diffVec2 = normalize(triangle.v2 - intersectPoint);
		
		/*	Compute the angles between the vectors. */
		float angle0 = acos(dot(diffVec0,diffVec1));
		float angle1 = acos(dot(diffVec0,diffVec2));
		float angle2 = acos(dot(diffVec1,diffVec2));
		
		/*	
		/	If the intersection point is outside of the triangle, the angles
		/	won't sum up to 360°.
		*/
		if( (angle0+angle1+angle2) < (2.0*M_PI-1.0e-3) ) return false;
	    
		return t_intersection >= 0.0;
	}
	
	return false;
	
}

bool triangleBoxIntersection(Ray ray, vec3 lower_box_vertex, vec3 upper_box_vertex, out float t_intersection, out vec3 normal)
{
	Triangle t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12;
	
	/* Front */
	t1 = createTriangle(lower_box_vertex, 
						vec3(lower_box_vertex.x,upper_box_vertex.y,lower_box_vertex.z),
						vec3(upper_box_vertex.x,upper_box_vertex.y,lower_box_vertex.z), vec4(1.0));
	t2 = createTriangle(lower_box_vertex,
						vec3(upper_box_vertex.x,upper_box_vertex.y,lower_box_vertex.z),
						vec3(upper_box_vertex.x,lower_box_vertex.y,lower_box_vertex.z), vec4(1.0));
	/* Left */
	t3 = createTriangle(lower_box_vertex,
						vec3(lower_box_vertex.x,upper_box_vertex.y,lower_box_vertex.z),
						vec3(lower_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	t4 = createTriangle(vec3(lower_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z),
						vec3(lower_box_vertex.x,lower_box_vertex.y,lower_box_vertex.z),
						vec3(lower_box_vertex.x,lower_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	/* Right */
	t5 = createTriangle(vec3(upper_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z),
						vec3(upper_box_vertex.x,lower_box_vertex.y,lower_box_vertex.z),
						vec3(upper_box_vertex.x,lower_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	t6 = createTriangle(vec3(upper_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z),
						vec3(upper_box_vertex.x,lower_box_vertex.y,lower_box_vertex.z),
						vec3(upper_box_vertex.x,upper_box_vertex.y,lower_box_vertex.z), vec4(1.0));
	/* Back */
	t7 = createTriangle(vec3(lower_box_vertex.x,lower_box_vertex.y,upper_box_vertex.z),
						vec3(lower_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z),
						vec3(upper_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	t8 = createTriangle(vec3(lower_box_vertex.x,lower_box_vertex.y,upper_box_vertex.z),
						vec3(upper_box_vertex.x,lower_box_vertex.y,upper_box_vertex.z),
						vec3(upper_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	/* Bottom */
	t9 = createTriangle(vec3(lower_box_vertex.x,lower_box_vertex.y,lower_box_vertex.z),
						vec3(upper_box_vertex.x,lower_box_vertex.y,lower_box_vertex.z),
						vec3(upper_box_vertex.x,lower_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	t10 = createTriangle(vec3(upper_box_vertex.x,lower_box_vertex.y,upper_box_vertex.z),
						vec3(lower_box_vertex.x,lower_box_vertex.y,lower_box_vertex.z),
						vec3(lower_box_vertex.x,lower_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	/* Top */
	t11 = createTriangle(vec3(lower_box_vertex.x,upper_box_vertex.y,lower_box_vertex.z),
						vec3(upper_box_vertex.x,upper_box_vertex.y,lower_box_vertex.z),
						vec3(upper_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	t12 = createTriangle(vec3(upper_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z),
						vec3(lower_box_vertex.x,upper_box_vertex.y,lower_box_vertex.z),
						vec3(lower_box_vertex.x,upper_box_vertex.y,upper_box_vertex.z), vec4(1.0));
	
	
	bool intersection = false;
	float distance = 0.0;
	t_intersection = 2.0;
	
	if(triangleIntersection(ray,t1,distance))
	{
		if(distance<t_intersection)
		{
			normal = t1.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t2,distance))
	{
		if(distance<t_intersection)
		{
			normal = t2.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t3,distance))
	{
		if(distance<t_intersection)
		{
			normal = t3.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t4,distance))
	{
		if(distance<t_intersection)
		{
			normal = t4.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t5,distance))
	{
		if(distance<t_intersection)
		{
			normal = t5.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t6,distance))
	{
		if(distance<t_intersection)
		{
			normal = t6.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t7,distance))
	{
		if(distance<t_intersection)
		{
			normal = t7.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t8,distance))
	{
		if(distance<t_intersection)
		{
			normal = t8.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t9,distance))
	{
		if(distance<t_intersection)
		{
			normal = t9.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t10,distance))
	{
		if(distance<t_intersection)
		{
			normal = t10.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t11,distance))
	{
		if(distance<t_intersection)
		{
			normal = t11.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	if(triangleIntersection(ray,t12,distance))
	{
		if(distance<t_intersection)
		{
			normal = t12.normal;
			t_intersection = distance;
		}
		intersection = true;
	}
	
	return intersection;
}

vec3 lighting(vec3 surface_point, vec3 surface_normal, vec4 surface_color, vec3 light_position)
{
	return surface_color.xyz*abs(dot(surface_normal,normalize(light_position-surface_point)));
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
	Ray ray;
	ray.direction = normalize(fragmentTextureCoord-cameraTextureCoord);
	
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
	//if( (max(max(cameraTextureCoord.x,cameraTextureCoord.y),cameraTextureCoord.z))>1.0f ||
	//	(min(min(cameraTextureCoord.x,cameraTextureCoord.y),cameraTextureCoord.z))<0.0f )
	//{
	//	ray.origin = colour;
	//}
	//else
	//{
	//	ray.origin = cameraTextureCoord;
	//}
	ray.origin = ( (max(max(cameraTextureCoord.x,cameraTextureCoord.y),cameraTextureCoord.z))>1.0f ||
		(min(min(cameraTextureCoord.x,cameraTextureCoord.y),cameraTextureCoord.z))<0.0f ) ? colour : cameraTextureCoord;
	
	
	/*	Storage for output color. */
	vec4 rgbaOut = vec4(0.0);
	
	/*	Used to store the so far traveled distance */
	float traveled_distance = 0.0;
	/*	Normalisation factor for color and opacity accumulation */
	float density = 0.005/(1.0/91.0);
	/* calculate the distance from entry to exit point */
	float exit_distance = min(	min(	max((1.0 - ray.origin.x)/ ray.direction.x , ray.origin.x/(-ray.direction.x)),
										max((1.0 - ray.origin.y)/ ray.direction.y , ray.origin.y/(-ray.direction.y))	),
								max((1.0 - ray.origin.z)/ ray.direction.z , ray.origin.z/(-ray.direction.z))	);
	
	
	float clip_plane_intersection_t;
	vec3 clip_plane_intersection_point;
	vec3 clip_plane_center = vec3(0.5);
	vec3 clip_plane_normal = vec3(1.0,1.0,1.0);
	planeIntersection(ray,clip_plane_center,clip_plane_normal,clip_plane_intersection_t,clip_plane_intersection_point);
	
	/*	Manipulate entryPoint and exit_distance so that the sampling loop will only run for sampley below the clip-plane. */
	if( dot(normalize(ray.origin-clip_plane_center),clip_plane_normal) > 0.0)
	{
		if(clip_plane_intersection_t<0.0)
		{
			exit_distance = -1.0;
		}
		else
		{
			ray.origin = clip_plane_intersection_point;
			exit_distance -= clip_plane_intersection_t;
		}
	}
	else
	{
		if(clip_plane_intersection_t>0.0)
		{
			exit_distance = min(clip_plane_intersection_t,exit_distance);
		}
	}
	
	
	float t_fault_bouning_box;
	vec3 bounding_box_normal;
	vec4 bounding_box_colour = vec4(1.0,0.0,0.0,0.8);
						
	triangleBoxIntersection(ray,vec3(0.2),vec3(0.7),t_fault_bouning_box,bounding_box_normal);
	bounding_box_colour.xyz = lighting( (ray.origin+t_fault_bouning_box*ray.direction), bounding_box_normal, bounding_box_colour, vec3(15.0,10.0,12.5) );
	
	
	while(rgbaOut.a < 0.99)
	{	
		/*	Check if the exit point has been reached */
		if( traveled_distance > exit_distance ) break;
		
		/*	Obtain values from the 3d texture at the current location. */
		vec3 sampleCoord = ray.origin+(ray.direction*traveled_distance);
		vec4 rgbaVol = vec4(texture(volumeTexture,sampleCoord).x);
		
		//float maskValue = texture3D(mask,sampleCoord).x;
		//if( maskValue > 0.5 )
		//{
		//	rgbaVol = vec4(1.0,0.0,0.0,0.025);
		//}
		if(traveled_distance > t_fault_bouning_box)
		{
			rgbaOut.rgb += (1.0 - rgbaOut.a) * bounding_box_colour.rgb * bounding_box_colour.a;
			rgbaOut.a += (1.0 - rgbaOut.a) * bounding_box_colour.a;
			/* Set distance to bounding box to a larger value than max possible traveled distance */
			t_fault_bouning_box = 2.0;
		}
		
		/*	A simple but costly solution for "discarding" all sample above the clip plane. */
		//	if( dot(normalize(sampleCoord-clip_plane_center),clip_plane_normal) > 0.0)
		//		rgbaVol.a = 0.0;
		
		/*	Accumulate color */
		rgbaOut.rgb += (1.0 - rgbaOut.a) * rgbaVol.rgb * rgbaVol.a * density;
		/*	Accumulate opacity */
		rgbaOut.a += (1.0 - rgbaOut.a) * rgbaVol.a * density;
		
		/*	Add the distance to the next sample point to the so far traveled distance */
		traveled_distance += 0.0025;
		
		/* Check against bb intersection */
		//if(traveled_distance > intersections.x)
		//{
		//	rgbaOut.rgb += (1.0 - rgbaOut.a) * vec3(1.0);
		//	rgbaOut.a = 1.0;
		//}
	}
	
	fragColour = vec4(rgbaOut);
	
}