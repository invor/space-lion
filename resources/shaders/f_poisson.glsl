/*
---------------------------------------------------------------------------------------------------
File: f_poisson.glsl
Author: Michael Becher
Date of (presumingly) last edit: 16.04.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Describtion: This GLSL fragment shader applies post-processing to image data, using 
poisson image editing and image inpainting techniques to cover up gaps in an image.
---------------------------------------------------------------------------------------------------
*/

#version 330

uniform sampler2D previousFrame;
uniform sampler2D inputImage;
uniform sampler2D mask;
uniform float iteration;
uniform vec2 imgDim;

/*
/	Define the domain of the missing image data by the coordinates of the lower left
/	and upper right corner of a rectangular domian.
/	 ----x
/	|    |
/	x---- 
*/
uniform vec2 lowerBound;
uniform vec2 upperBound;

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
	vec3 rgbM = texture2D(inputImage,pos);
	vec3 rgbE = texture2D(inputImage,pos+vE).xyz;
	vec3 rgbS = texture2D(inputImage,pos+vS).xyz;
	
	return (rgbN+rgbW+rgbE+rgbS-(4.0f*rgbM));
}

vec3 calcGuidanceVectorX(vec2 pos, vec2 h)
{
	vec3 rgbW = texture2D(previousFrame,pos+vec2(-h.x,0.0f)).rgb;
	vec3 rgbE = texture2D(previousFrame,pos+vec2(h.x,0.0f)).rgb;
	
	return (rgbE - rgbW)/ (2.0f*h.x);
}

vec3 calcGuidanceVectorY(vec2 pos, vec2 h)
{
	vec3 rgbN = texture2D(previousFrame,pos+vec2(0.0f,h.y)).rgb;
	vec3 rgbS = texture2D(previousFrame,pos+vec2(0.0f,-h.y)).rgb;
	
	return (rgbN - rgbS)/ (2.0f*h.x);
}

vec3 poissonImageEditing(vec2 pos, vec2 h)
{
	vec2 vN = vec2(0.0f,h.y);
	vec2 vW = vec2(-h.x,0.0f);
	vec2 vE = vec2(h.x,0.0f);
	vec2 vS = vec2(0.0f,-h.y);

	vec3 rgbN = texture2D(inputImage,pos+vN).xyz;
	vec3 rgbW = texture2D(inputImage,pos+vW).xyz;
	vec3 rgbE = texture2D(inputImage,pos+vE).xyz;
	vec3 rgbS = texture2D(inputImage,pos+vS).xyz;
	
	vec3 guideNx = calcGuidanceVectorX( (pos+pos+vN)*0.5f, h );
	vec3 guideNy = calcGuidanceVectorY( (pos+pos+vN)*0.5f, h );
	vec3 guideWx = calcGuidanceVectorX( (pos+pos+vW)*0.5f, h );
	vec3 guideWy = calcGuidanceVectorY( (pos+pos+vW)*0.5f, h );
	vec3 guideEx = calcGuidanceVectorX( (pos+pos+vE)*0.5f, h );
	vec3 guideEy = calcGuidanceVectorY( (pos+pos+vE)*0.5f, h );
	vec3 guideSx = calcGuidanceVectorX( (pos+pos+vS)*0.5f, h );
	vec3 guideSy = calcGuidanceVectorY( (pos+pos+vS)*0.5f, h );
	
	vec3 projN = vec3( dot(vec2(guideNx.r,guideNy.r), vN),
					dot(vec2(guideNx.g,guideNy.g), vN),
					dot(vec2(guideNx.b,guideNy.b), vN));
	vec3 projW = vec3( dot(vec2(guideWx.r,guideWy.r), vW),
					dot(vec2(guideWx.g,guideWy.g), vW),
					dot(vec2(guideWx.b,guideWy.b), vW));				
	vec3 projE = vec3( dot(vec2(guideEx.r,guideEy.r), vE),
					dot(vec2(guideEx.g,guideEy.g), vE),
					dot(vec2(guideEx.b,guideEy.b), vE));
	vec3 projS = vec3( dot(vec2(guideSx.r,guideSy.r), vS),
					dot(vec2(guideSx.g,guideSy.g), vS),
					dot(vec2(guideSx.b,guideSy.b), vS));
	
	vec3 rgbF = (rgbN + rgbW + rgbE + rgbS - projN - projW - projE - projS) * 0.25;
	//rgbF = (rgbN + rgbW + rgbE + rgbS) * 0.25;
	
	return rgbF;
}

/*
/	Computation is sped up by using a stencil size large enough to reach the border
/	of the region to be inpainted. Based on the concept presented in
/	"A GPU Laplacian Solver for Diffusion Curves and Poisson Image Editing" by
/	Stefan Jeschke,David Cline and Peter Wonka (2009).
*/
vec3 acceleratedPoissonImageEditing(vec2 pos, vec2 h, float i)
{
	float dist = min(min(abs(uvCoord.x-lowerBound.x),abs(upperBound.x-uvCoord.x)),
					 min(abs(uvCoord.y-lowerBound.y),abs(upperBound.y-uvCoord.y)));
	float minPixelDist = min(h.x,h.y);
	//dist = (1.0/512.0)+((dist)/(pow(2.0,i)));
	dist = minPixelDist+((dist-minPixelDist)/i);
					 
	vec2 vN = vec2(0.0f,dist);
	vec2 vW = vec2(-dist,0.0f);
	vec2 vE = vec2(dist,0.0f);
	vec2 vS = vec2(0.0f,-dist);

	vec3 rgbN = texture2D(inputImage,pos+vN).xyz;
	vec3 rgbW = texture2D(inputImage,pos+vW).xyz;
	vec3 rgbE = texture2D(inputImage,pos+vE).xyz;
	vec3 rgbS = texture2D(inputImage,pos+vS).xyz;
	
	vec3 guideNx = calcGuidanceVectorX( (pos+pos+vN)*0.5f, vec2(dist) );
	vec3 guideNy = calcGuidanceVectorY( (pos+pos+vN)*0.5f, vec2(dist) );
	vec3 guideWx = calcGuidanceVectorX( (pos+pos+vW)*0.5f, vec2(dist) );
	vec3 guideWy = calcGuidanceVectorY( (pos+pos+vW)*0.5f, vec2(dist) );
	vec3 guideEx = calcGuidanceVectorX( (pos+pos+vE)*0.5f, vec2(dist) );
	vec3 guideEy = calcGuidanceVectorY( (pos+pos+vE)*0.5f, vec2(dist) );
	vec3 guideSx = calcGuidanceVectorX( (pos+pos+vS)*0.5f, vec2(dist) );
	vec3 guideSy = calcGuidanceVectorY( (pos+pos+vS)*0.5f, vec2(dist) );
	
	vec3 projN = vec3( dot(vec2(guideNx.r,guideNy.r), vN),
					dot(vec2(guideNx.g,guideNy.g), vN),
					dot(vec2(guideNx.b,guideNy.b), vN));
	vec3 projW = vec3( dot(vec2(guideWx.r,guideWy.r), vW),
					dot(vec2(guideWx.g,guideWy.g), vW),
					dot(vec2(guideWx.b,guideWy.b), vW));				
	vec3 projE = vec3( dot(vec2(guideEx.r,guideEy.r), vE),
					dot(vec2(guideEx.g,guideEy.g), vE),
					dot(vec2(guideEx.b,guideEy.b), vE));
	vec3 projS = vec3( dot(vec2(guideSx.r,guideSy.r), vS),
					dot(vec2(guideSx.g,guideSy.g), vS),
					dot(vec2(guideSx.b,guideSy.b), vS));
	
	vec3 rgbF = (rgbN + rgbW + rgbE + rgbS - projN - projW - projE - projS) * 0.25;
	//rgbF = (rgbN + rgbW + rgbE + rgbS) * 0.25;
	
	return rgbF;
}

vec3 seamlessCloning(vec2 pos, vec2 h)
{
	vec3 rgbN = texture2D(inputImage,pos+vec2(0.0f,h.y)).xyz;
	vec3 rgbW = texture2D(inputImage,pos+vec2(-h.x,0.0f)).xyz;
	vec3 rgbE = texture2D(inputImage,pos+vec2(h.x,0.0f)).xyz;
	vec3 rgbS = texture2D(inputImage,pos+vec2(0.0f,-h.y)).xyz;
	vec3 rgbG = (rgbN+rgbW+rgbE+rgbS) * 0.25;
	
	vec3 rgbNsrc = texture2D(previousFrame,pos+vec2(0.0f,h.y)).xyz;
	vec3 rgbWsrc = texture2D(previousFrame,pos+vec2(-h.x,0.0f)).xyz;
	vec3 rgbEsrc = texture2D(previousFrame,pos+vec2(h.x,0.0f)).xyz;
	vec3 rgbSsrc = texture2D(previousFrame,pos+vec2(0.0f,-h.y)).xyz;
	vec3 rgbGsrc = (rgbNsrc+rgbWsrc+rgbEsrc+rgbSsrc) * 0.25;
	
	vec3 rgbMsrc = texture2D(previousFrame,pos).xyz;
	
	return (rgbMsrc + rgbG - rgbGsrc);
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
		//fragColour = vec4(poissonImageEditing(uvCoord, h),1.0);
		fragColour = vec4(imageInpainting(uvCoord, h),1.0);
	}
	else
	{
		fragColour = vec4(texture2D(inputImage,uvCoord).xyz,1.0);
	}
	
	
	//if(    greaterThan(uvCoord,lowerBound).x 
	//	&& greaterThan(uvCoord,lowerBound).y
	//	&& lessThan(uvCoord,upperBound).x
	//	&& lessThan(uvCoord,upperBound).y	)
	//{
	//	vec2 h = vec2(1.0f/imgDim.x, 1.0f/imgDim.y);
	//	
	//	//fragColour = vec4(poissonImageEditing(uvCoord, h),1.0);
	//	fragColour = vec4(acceleratedPoissonImageEditing(uvCoord, h, iteration),1.0);
	//	//fragColour = vec4(seamlessCloning(uvCoord, h),1.0);
	//}
	//else
	//{
	//	fragColour = vec4(texture2D(inputImage,uvCoord).xyz,1.0);
	//}
}