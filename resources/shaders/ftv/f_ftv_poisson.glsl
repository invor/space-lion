/*---------------------------------------------------------------------------------------------------
File: f_ftv_poisson.glsl
Author: Michael Becher
Date of (presumingly) last edit: 26.06.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: This GLSL fragment shader applies post-processing to image data, using 
poisson image editing techniques to cover up gaps in an image.
Based on the techniques presented in "Poisson Image Editing" by Patrick Pérez, Michel Gangnet
and Andrew Blake (2003).
---------------------------------------------------------------------------------------------------*/

#version 330

/*	Texture containing the 2d guidance vector field */
uniform sampler2D guidanceField_tx2D;

/*	Texture containing the previously rendered frame */
uniform sampler2D prevFrame_tx2D;

/*	Texture containing the currently rendered frame */
uniform sampler2D currFrame_tx2D;

/*	Texture containing the inpainting mask */
uniform sampler2D mask_tx2D;

/*
/	Texture containing a distance map for the inpainting mask.
/	Contains for every point within an inpainting region the distances to each
/	edge of the region.
*/
uniform sampler2D distance_tx2D;

/*	Denotes the current iteration step */
uniform float iteration;

/*
/	Switches between image editing modes:
/	0 - Laplacian diffusion with guidance field
/	1 - Laplacian diffusiin only
*/
uniform int mode;

/*
/	Specifies the distance between two pixels in x and y direction
/	(in texture space [0,1] )
*/
uniform vec2 h;

/*	Normalized coordinates [0,1]x[0,1] of the fragment */
in vec2 uvCoord;

/*	Fragment shader output variable */
out vec4 fragColour;


vec3 calcGuidanceVectorX(vec2 pos, vec2 h)
{
	vec3 rgbW = texture2D(prevFrame_tx2D,pos+vec2(-h.x,0.0f)).rgb;
	vec3 rgbE = texture2D(prevFrame_tx2D,pos+vec2(h.x,0.0f)).rgb;
	
	return (rgbE - rgbW)/ (2.0f*h.x);
}

vec3 calcGuidanceVectorY(vec2 pos, vec2 h)
{
	vec3 rgbN = texture2D(prevFrame_tx2D,pos+vec2(0.0f,h.y)).rgb;
	vec3 rgbS = texture2D(prevFrame_tx2D,pos+vec2(0.0f,-h.y)).rgb;
	
	return (rgbN - rgbS)/ (2.0f*h.x);
}

vec3 poissonImageEditing(vec2 pos)
{
	vec2 vN = vec2(0.0f,h.y);
	vec2 vW = vec2(-h.x,0.0f);
	vec2 vE = vec2(h.x,0.0f);
	vec2 vS = vec2(0.0f,-h.y);

	vec3 rgbN = texture2D(currFrame_tx2D,pos+vN).xyz;
	vec3 rgbW = texture2D(currFrame_tx2D,pos+vW).xyz;
	vec3 rgbE = texture2D(currFrame_tx2D,pos+vE).xyz;
	vec3 rgbS = texture2D(currFrame_tx2D,pos+vS).xyz;
	
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
/	Computation is sped up by using a stencil size large enough to reach the edge
/	of the region to be inpainted. Based on the concept presented in
/	"A GPU Laplacian Solver for Diffusion Curves and Poisson Image Editing" by
/	Stefan Jeschke,David Cline and Peter Wonka (2009).
*/
vec3 acceleratedPoissonImageEditing(vec2 pos)
{
	vec4 dist;
	dist.wxyz = texture2D(distance_tx2D, pos);
	//dist.x = h.y+((dist.x-h.y)/pow(2.0,iteration));
	//dist.y = h.x+((dist.y-h.x)/pow(2.0,iteration));
	//dist.z = h.y+((dist.z-h.y)/pow(2.0,iteration));
	//dist.w = h.x+((dist.w-h.x)/pow(2.0,iteration));
	dist.x = h.y+((dist.x)/iteration);
	dist.y = h.x+((dist.y)/iteration);
	dist.z = h.y+((dist.z)/iteration);
	dist.w = h.x+((dist.w)/iteration);
	float verticalDist = dist.x+dist.z;
	float horizontalDist = dist.y+dist.w;
					 
	vec2 vN = vec2(0.0f,dist.x);
	vec2 vE = vec2(dist.y,0.0f);
	vec2 vS = vec2(0.0f,-dist.z);
	vec2 vW = vec2(-dist.w,0.0f);

	vec3 rgbN = texture2D(currFrame_tx2D,pos+vN).xyz;
	vec3 rgbE = texture2D(currFrame_tx2D,pos+vE).xyz;
	vec3 rgbS = texture2D(currFrame_tx2D,pos+vS).xyz;
	vec3 rgbW = texture2D(currFrame_tx2D,pos+vW).xyz;
	
	vec3 guideNx = calcGuidanceVectorX( (pos+pos+vN)*0.5f, h );
	vec3 guideNy = calcGuidanceVectorY( (pos+pos+vN)*0.5f, h );
	vec3 guideEx = calcGuidanceVectorX( (pos+pos+vE)*0.5f, h );
	vec3 guideEy = calcGuidanceVectorY( (pos+pos+vE)*0.5f, h );
	vec3 guideSx = calcGuidanceVectorX( (pos+pos+vS)*0.5f, h );
	vec3 guideSy = calcGuidanceVectorY( (pos+pos+vS)*0.5f, h );
	vec3 guideWx = calcGuidanceVectorX( (pos+pos+vW)*0.5f, h );
	vec3 guideWy = calcGuidanceVectorY( (pos+pos+vW)*0.5f, h );
	
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
	
	/*
	/	Use inverse distance weigthing for the stencil pixels.
	*/
	//vec3 rgbF = ( rgbN*(dist.z/verticalDist) +
	//			rgbW*(dist.y/horizontalDist) +
	//			rgbE*(dist.w/horizontalDist) +
	//			rgbS*(dist.x/verticalDist) ) / ( (dist.z/verticalDist)+
	//											 (dist.y/horizontalDist)+
	//											 (dist.w/horizontalDist)+
	//											 (dist.x/verticalDist) );
	//		//	+ (- projN - projW - projE - projS)*0.25f;
	
	/*	compute simple laplacian diffusian without guidance first */
	vec3 rgbF_woG = (rgbN + rgbW + rgbE + rgbS)*0.25f;
	
	/*	then add the guidance field */
	vec3 rgbF_wG = rgbF_woG + (- projN - projW - projE - projS)*0.25f;
	
	if(mode==0) return rgbF_wG;
	else return rgbF_woG;
}

vec3 seamlessCloning(vec2 pos)
{
	vec3 rgbN = texture2D(currFrame_tx2D,pos+vec2(0.0f,h.y)).xyz;
	vec3 rgbW = texture2D(currFrame_tx2D,pos+vec2(-h.x,0.0f)).xyz;
	vec3 rgbE = texture2D(currFrame_tx2D,pos+vec2(h.x,0.0f)).xyz;
	vec3 rgbS = texture2D(currFrame_tx2D,pos+vec2(0.0f,-h.y)).xyz;
	vec3 rgbG = (rgbN+rgbW+rgbE+rgbS) * 0.25;
	
	vec3 rgbNsrc = texture2D(prevFrame_tx2D,pos+vec2(0.0f,h.y)).xyz;
	vec3 rgbWsrc = texture2D(prevFrame_tx2D,pos+vec2(-h.x,0.0f)).xyz;
	vec3 rgbEsrc = texture2D(prevFrame_tx2D,pos+vec2(h.x,0.0f)).xyz;
	vec3 rgbSsrc = texture2D(prevFrame_tx2D,pos+vec2(0.0f,-h.y)).xyz;
	vec3 rgbGsrc = (rgbNsrc+rgbWsrc+rgbEsrc+rgbSsrc) * 0.25;
	
	vec3 rgbMsrc = texture2D(prevFrame_tx2D,pos).xyz;
	
	return (rgbMsrc + rgbG - rgbGsrc);
}

vec3 guidedPoissonImageEditing(vec2 pos)
{
	/*	Stencil directions */
	vec2 vN = vec2(0.0f,h.y);
	vec2 vNE = vec2(h.x,h.y);
	vec2 vE = vec2(h.x,0.0f);
	vec2 vSE = vec2(h.x,-h.y);
	vec2 vS = vec2(0.0f,-h.y);
	vec2 vSW = vec2(-h.x,-h.y);
	vec2 vW = vec2(-h.x,0.0f);
	vec2 vNW = vec2(-h.x,h.y);

	/*	Get stencil values*/
	vec3 rgbN = texture(currFrame_tx2D,pos+vN).xyz;
	vec3 rgbNE = texture(currFrame_tx2D,pos+vNE).xyz;
	vec3 rgbE = texture(currFrame_tx2D,pos+vE).xyz;
	vec3 rgbSE = texture(currFrame_tx2D,pos+vSE).xyz;
	vec3 rgbS = texture(currFrame_tx2D,pos+vS).xyz;
	vec3 rgbSW = texture(currFrame_tx2D,pos+vSW).xyz;
	vec3 rgbW = texture(currFrame_tx2D,pos+vW).xyz;
	vec3 rgbNW = texture(currFrame_tx2D,pos+vNW).xyz;

	/*	Get guidance field */
	//	vec2 guideVecN = texture(guidanceField_tx2D,pos+vN*0.5f).xy;
	//	vec2 guideVecE = texture(guidanceField_tx2D,pos+vE*0.5f).xy;
	//	vec2 guideVecS = texture(guidanceField_tx2D,pos+vS*0.5f).xy;
	//	vec2 guideVecW = texture(guidanceField_tx2D,pos+vW*0.5f).xy;
	
	vec2 guideVec = texture(guidanceField_tx2D,pos).xy;

	vec2 guideVecN = texture(guidanceField_tx2D,pos+vN).xy;
	vec2 guideVecNE = texture(guidanceField_tx2D,pos+vNE).xy;;
	vec2 guideVecE = texture(guidanceField_tx2D,pos+vE).xy;;
	vec2 guideVecSE = texture(guidanceField_tx2D,pos+vSE).xy;;
	vec2 guideVecS = texture(guidanceField_tx2D,pos+vS).xy;;
	vec2 guideVecSW = texture(guidanceField_tx2D,pos+vSW).xy;;
	vec2 guideVecW = texture(guidanceField_tx2D,pos+vW).xy;;
	vec2 guideVecNW = texture(guidanceField_tx2D,pos+vNW).xy;;

	/*	Compute stencil weights from the guidance field */
	float weightN = pow( abs(dot( normalize(guideVecN) , normalize(vN) ) ) , 22.0 );
	float weightNE = pow( abs(dot( normalize(guideVecNE) , normalize(vNE) ) ) , 22.0 );
	float weightE = pow( abs(dot( normalize(guideVecE) , normalize(vE) ) ) , 22.0 );
	float weightSE = pow( abs(dot( normalize(guideVecSE) , normalize(vSE) ) ) , 22.0 );
	float weightS = pow( abs(dot( normalize(guideVecS) , normalize(vS) ) ) , 22.0 );
	float weightSW = pow( abs(dot( normalize(guideVecSW) , normalize(vSW) ) ) , 22.0 );
	float weightW = pow( abs(dot( normalize(guideVecW) , normalize(vW) ) ) , 22.0 );
	float weightNW = pow( abs(dot( normalize(guideVecNW) , normalize(vNW) ) ) , 22.0 );
	float weightSum = weightN + weightNE + weightE + weightSE + weightS + weightSW + weightW + weightNW;

	/*	Now calculate the guided diffusion */
	//vec3 rgbF = ( (rgbN*weightN + rgbE*weightE + rgbS*weightS + rgbW*weightW)/weightSum ) * 0.25;
	//vec3 rgbF = ( (rgbN + rgbE + rgbS + rgbW) + (weightN + weightE + weightS + weightW) ) * 0.25;
	//vec3 rgbF = ( (weightN + weightE + weightS + weightW) ) * 0.25;
	//rgbF = vec3(texture(guidanceField_tx2D,pos).xy,0.0);
	vec3 rgbF = (	rgbN*weightN +
					rgbNE*weightNE +
					rgbE*weightE +
					rgbSE*weightSE +
					rgbS*weightS +
					rgbSW*weightSW +
					rgbW*weightW +
					rgbNW*weightNW	) /weightSum;

	return rgbF;
}


void main()
{
	//fragColour = vec4(acceleratedPoissonImageEditing(uvCoord),1.0);
	//fragColour = vec4(guidedPoissonImageEditing(uvCoord),1.0);
	
	fragColour = (texture2D(mask_tx2D,uvCoord).x < 0.5f) ? vec4(poissonImageEditing(uvCoord),1.0) : vec4(texture2D(currFrame_tx2D,uvCoord).xyz,1.0);
}