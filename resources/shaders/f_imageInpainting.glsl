/*
---------------------------------------------------------------------------------------------------
File: f_imageInpainting.glsl
Author: Michael Becher
Date of (presumingly) last edit: 16.04.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Describtion: This GLSL fragment shader applies post-processing to image data, using 
image inpainting techniques to cover up gaps in an image.
---------------------------------------------------------------------------------------------------
*/

#version 330

uniform sampler2D inputImage;
uniform sampler2D mask;
uniform vec2 imgDim;

/*
/	Normalized coordinates [0,1]x[0,1] of the fragment.
*/
in vec2 uvCoord;

/*
/	Fragment shader output variable.
*/
out vec4 fragColour;


/*
/	Compute laplace operator for a given position pos and pixeldistances h.
*/
vec3 laplace(vec2 pos, vec2 h)
{
	vec2 vN = vec2(0.0f,h.y);
	vec2 vW = vec2(-h.x,0.0f);
	vec2 vE = vec2(h.x,0.0f);
	vec2 vS = vec2(0.0f,-h.y);
	
	vec3 rgbN = texture2D(inputImage,pos+vN).xyz;
	vec3 rgbW = texture2D(inputImage,pos+vW).xyz;
	vec3 rgbM = texture2D(inputImage,pos).xyz;
	vec3 rgbE = texture2D(inputImage,pos+vE).xyz;
	vec3 rgbS = texture2D(inputImage,pos+vS).xyz;
	
	return (rgbN+rgbW+rgbE+rgbS-(4.0f*rgbM));
}

vec3 imageInpainting(vec2 pos, vec2 h)
{
	/*
	/	Vectors to the neighbour texels
	*/
	vec2 vN = vec2(0.0f,h.y);
	vec2 vW = vec2(-h.x,0.0f);
	vec2 vE = vec2(h.x,0.0f);
	vec2 vS = vec2(0.0f,-h.y);
	
	/*
	/	Laplacian in the four neighbour texels
	*/
	vec3 lN = laplace(pos+vN,h);
	vec3 lW = laplace(pos+vW,h);
	vec3 lE = laplace(pos+vE,h);
	vec3 lS = laplace(pos+vS,h);
	
	/*
	/	Color values of the stencil
	*/
	vec3 rgbN = texture2D(inputImage,pos+vN).xyz;
	vec3 rgbW = texture2D(inputImage,pos+vW).xyz;
	vec3 rgbM = texture2D(inputImage,pos).xyz;
	vec3 rgbE = texture2D(inputImage,pos+vE).xyz;
	vec3 rgbS = texture2D(inputImage,pos+vS).xyz;
	
	/*
	/	Compute L and N vectors and Beta for each color channel
	*/
	vec2 rL = vec2(lE.r-lW.r,lN.r-lS.r);
	vec2 gL = vec2(lE.g-lW.g,lN.g-lS.g);
	vec2 bL = vec2(lE.b-lW.b,lN.b-lS.b);
	
	vec2 rN = vec2(-(rgbN.r-rgbS.r)/(2.0*h.y),(rgbE.r-rgbW.r)/(2.0*h.x));
	//vec2 rN = vec2(-(rgbE.r-rgbW.r)/(2.0*h.x),-(rgbN.r-rgbS.r)/(2.0*h.y));
	
	vec2 gN = vec2(-(rgbN.g-rgbS.g)/(2.0*h.y),(rgbE.g-rgbW.g)/(2.0*h.x));
	vec2 bN = vec2(-(rgbN.b-rgbS.b)/(2.0*h.y),(rgbE.b-rgbW.b)/(2.0*h.x));
	
	float rBeta = dot(rL,normalize(rN));
	float gBeta = dot(gL,normalize(gN));
	float bBeta = dot(bL,normalize(bN));
	
	float rNabla;
	float gNabla;
	float bNabla;
	
	if(rBeta > 0.0)
	{
		rNabla = sqrt(	pow(min(0.0f,(rgbM.r-rgbW.r)/h.x),2) +
						pow(max(0.0f,(rgbE.r-rgbM.r)/h.x),2) +
						pow(min(0.0f,(rgbM.r-rgbS.r)/h.y),2) +
						pow(max(0.0f,(rgbN.r-rgbM.r)/h.y),2));
	}
	else
	{
		rNabla = sqrt(	pow(max(0.0f,(rgbM.r-rgbW.r)/h.x),2) +
						pow(min(0.0f,(rgbE.r-rgbM.r)/h.x),2) +
						pow(max(0.0f,(rgbM.r-rgbS.r)/h.y),2) +
						pow(min(0.0f,(rgbN.r-rgbM.r)/h.y),2));
	}
	
	if(gBeta > 0.0)
	{
		gNabla = sqrt(	pow(min(0.0f,(rgbM.g-rgbW.g)/h.x),2) +
						pow(max(0.0f,(rgbE.g-rgbM.g)/h.x),2) +
						pow(min(0.0f,(rgbM.g-rgbS.g)/h.y),2) +
						pow(max(0.0f,(rgbN.g-rgbM.g)/h.y),2));
	}
	else
	{
		gNabla = sqrt(	pow(max(0.0f,(rgbM.g-rgbW.g)/h.x),2) +
						pow(min(0.0f,(rgbE.g-rgbM.g)/h.x),2) +
						pow(max(0.0f,(rgbM.g-rgbS.g)/h.y),2) +
						pow(min(0.0f,(rgbN.g-rgbM.g)/h.y),2));
	}
	
	if(bBeta > 0.0)
	{
		bNabla = sqrt(	pow(min(0.0f,(rgbM.b-rgbW.b)/h.x),2) +
						pow(max(0.0f,(rgbE.b-rgbM.b)/h.x),2) +
						pow(min(0.0f,(rgbM.b-rgbS.b)/h.y),2) +
						pow(max(0.0f,(rgbN.b-rgbM.b)/h.y),2));
	}
	else
	{
		bNabla = sqrt(	pow(max(0.0f,(rgbM.b-rgbW.b)/h.x),2) +
						pow(min(0.0f,(rgbE.b-rgbM.b)/h.x),2) +
						pow(max(0.0f,(rgbM.b-rgbS.b)/h.y),2) +
						pow(min(0.0f,(rgbN.b-rgbM.b)/h.y),2));
	}
	/*
	if(rBeta > 0.0)
	{
		rNabla = sqrt(	pow(min(0.0f,(rgbM.r-rgbW.r)),2) +
						pow(max(0.0f,(rgbE.r-rgbM.r)),2) +
						pow(min(0.0f,(rgbM.r-rgbS.r)),2) +
						pow(max(0.0f,(rgbN.r-rgbM.r)),2));
	}
	else
	{
		rNabla = sqrt(	pow(max(0.0f,(rgbM.r-rgbW.r)),2) +
						pow(min(0.0f,(rgbE.r-rgbM.r)),2) +
						pow(max(0.0f,(rgbM.r-rgbS.r)),2) +
						pow(min(0.0f,(rgbN.r-rgbM.r)),2));
	}
	
	if(gBeta > 0.0)
	{
		gNabla = sqrt(	pow(min(0.0f,(rgbM.g-rgbW.g)),2) +
						pow(max(0.0f,(rgbE.g-rgbM.g)),2) +
						pow(min(0.0f,(rgbM.g-rgbS.g)),2) +
						pow(max(0.0f,(rgbN.g-rgbM.g)),2));
	}
	else
	{
		gNabla = sqrt(	pow(max(0.0f,(rgbM.g-rgbW.g)),2) +
						pow(min(0.0f,(rgbE.g-rgbM.g)),2) +
						pow(max(0.0f,(rgbM.g-rgbS.g)),2) +
						pow(min(0.0f,(rgbN.g-rgbM.g)),2));
	}
	
	if(bBeta > 0.0)
	{
		bNabla = sqrt(	pow(min(0.0f,(rgbM.b-rgbW.b)),2) +
						pow(max(0.0f,(rgbE.b-rgbM.b)),2) +
						pow(min(0.0f,(rgbM.b-rgbS.b)),2) +
						pow(max(0.0f,(rgbN.b-rgbM.b)),2));
	}
	else
	{
		bNabla = sqrt(	pow(max(0.0f,(rgbM.b-rgbW.b)),2) +
						pow(min(0.0f,(rgbE.b-rgbM.b)),2) +
						pow(max(0.0f,(rgbM.b-rgbS.b)),2) +
						pow(min(0.0f,(rgbN.b-rgbM.b)),2));
	}*/

	vec3 I = vec3(0.0f,0.0f,0.0f);
	
	return vec3(min(1.0,rgbM.x+I.x),rgbM.y,rgbM.z);
}

void main()
{
	if(texture2D(mask,uvCoord).x < 0.5f)
	{
		vec2 h = vec2(1.0f/imgDim.x, 1.0f/imgDim.y);
		fragColour = vec4(imageInpainting(uvCoord, h),1.0);
	}
	else
	{
		fragColour = vec4(texture2D(inputImage,uvCoord).xyz,1.0);
	}
}