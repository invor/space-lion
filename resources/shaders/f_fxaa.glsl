/*
/	FXAA Post Processing Shader
/	
/	Implementation based on NVIDIA Whitepaper by Timithy Lottes
/	http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
/
*/

#version 330

/*
/	FXAA setting, defined via preprocessor variables
*/
#ifndef FXAA_PRESET
    #define FXAA_PRESET 3
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 0)
    #define FXAA_EDGE_THRESHOLD      (1.0/4.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/12.0)
    #define FXAA_SEARCH_STEPS        2
    #define FXAA_SEARCH_ACCELERATION 4
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       1
    #define FXAA_SUBPIX_CAP          (2.0/3.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 1)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/16.0)
    #define FXAA_SEARCH_STEPS        4
    #define FXAA_SEARCH_ACCELERATION 3
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 2)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        8
    #define FXAA_SEARCH_ACCELERATION 2
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 3)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        16
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 4)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        24
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 5)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        32
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#define FXAA_SUBPIX_TRIM_SCALE (1.0/(1.0 - FXAA_SUBPIX_TRIM))

/*
/	Uniform input variables
*/
uniform sampler2D inputImage;
//	Dimensions of the input image, given in number of texels
uniform vec2 imgDim;

//	Normalized coordinates [0,1]x[0,1] of the fragment
in vec2 uvCoord;

out vec4 fragColour;

float FxaaLuma(vec3 rgb)
{
	return rgb.y * (0.587f/0.299f) + rgb.x;
}

vec3 fxaa()
{
	//	Compute distance between pixels in texture space
	vec2 h = vec2(1.0f/imgDim.x,1.0f/imgDim.y);
	
	/*
	/	LOCAL CONTRAST CHECK FOR EARLY EXIT
	*/
	vec3 rgbN = texture2D(inputImage,uvCoord+vec2(0.0f,h.y)).xyz;
	vec3 rgbW = texture2D(inputImage,uvCoord+vec2(-h.x,0.0f)).xyz;
	vec3 rgbM = texture2D(inputImage,uvCoord).xyz;
	vec3 rgbE = texture2D(inputImage,uvCoord+vec2(h.x,0.0f)).xyz;
	vec3 rgbS = texture2D(inputImage,uvCoord+vec2(0.0f,-h.y)).xyz;
	float lumaN = FxaaLuma(rgbN);
	float lumaW = FxaaLuma(rgbW);
	float lumaM = FxaaLuma(rgbM);
	float lumaE = FxaaLuma(rgbE);
	float lumaS = FxaaLuma(rgbS);
	float rangeMin = min( lumaM,min( min(lumaN,lumaW),min(lumaS,lumaE) ) );
	float rangeMax = max( lumaM,max( max(lumaN,lumaW),max(lumaS,lumaE) ) );
	float range = rangeMax - rangeMin;
	if(range < max( FXAA_EDGE_THRESHOLD_MIN, rangeMax*FXAA_EDGE_THRESHOLD ))
	{
		return rgbM;
	}
	#if FXAA_SUBPIX > 0
		#if FXAA_SUBPIX_FASTER 
			//vec3 rgbL = (rgbN + rgbW + rgbE +rgbS +rgbM) * vec3(1.0f/5.0f);
			vec3 rgbL = (rgbN + rgbW + rgbE +rgbS +rgbM) * (1.0f/5.0f);
		#else
			vec3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
		#endif
	#endif
	
	/*
	/	COMPUTE LOCAL LOWPASS
	*/
	#if FXAA_SUBPIX != 0
		float lumaL = (lumaN+lumaW+lumaE+lumaS) * 0.25f;
		float rangeL = abs(lumaL - lumaM);
	#endif
	#if FXAA_SUBPIX == 1
		float blendL = max(0.0f, (rangeL/range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
		blendL = min(FXAA_SUBPIX_CAP, blendL);
	#endif
	#if FXAA_SUBPIX == 2
		float blendL = rangeL / range;
	#endif
	
	/*
	/	VERTICAL AND HORIZONTAL EDGE SEARCH
	*/
	vec3 rgbNW = texture2D(inputImage,uvCoord+vec2(-h.x,h.y)).xyz;
	vec3 rgbNE = texture2D(inputImage,uvCoord+vec2(h.x,h.y)).xyz;
	vec3 rgbSW = texture2D(inputImage,uvCoord+vec2(-h.x,-h.y)).xyz;
	vec3 rgbSE = texture2D(inputImage,uvCoord+vec2(h.x,-h.y)).xyz;
	#if (FXAA_SUBPIX_FASTER == 0) && (FXAA_SUBPIX >0)
		rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
		//rgbL *= vec3(1.0f/9.0f);
		rgbL *= (1.0f/9.0f);
	#endif
	float lumaNW = FxaaLuma(rgbNW);
    float lumaNE = FxaaLuma(rgbNE);
    float lumaSW = FxaaLuma(rgbSW);
    float lumaSE = FxaaLuma(rgbSE);
	
	float edgeVertical = abs( (0.25f * lumaNW) + (-0.5f * lumaN) + (0.25f * lumaNE) ) +
					 abs( (0.5f * lumaW) + (-1.0f * lumaM) + (0.5f * lumaE) ) +
					 abs( (0.25f * lumaSW) + (-0.5f *lumaS) + (0.25f * lumaSE));
	float edgeHorizontal = abs( (0.25f * lumaNW) + (-0.5f * lumaN) + (0.25f * lumaSW) ) +
					 abs( (0.5f * lumaN) + (-1.0f * lumaM) + (0.5f * lumaS) ) +
					 abs( (0.25f * lumaNE) + (-0.5f *lumaE) + (0.25f * lumaSE));
	bool horzSpan = edgeHorizontal >= edgeVertical;
	//if(horzSpan)
	//{
	//	return vec3(1.0,0.0,0.0);
	//}
	//else
	//{
	//	return vec3(0.0,0.0,1.0);
	//}
	//float lengthSign = horzSpan ? -h.y : -h.x;
	/*
	/	In openGL the origin is in the lower left corner, not the upper left
	*/
	float lengthSign = horzSpan ? h.y : -h.x;
	//	both the vertical and the horizontal case use the same variables
	if(!horzSpan) lumaN = lumaW;
    if(!horzSpan) lumaS = lumaE;
    float gradientN = abs(lumaN - lumaM);
    float gradientS = abs(lumaS - lumaM);
    lumaN = (lumaN + lumaM) * 0.5f;
    lumaS = (lumaS + lumaM) * 0.5f;
	
	/*
	/	CHOOSE HIGHEST GRADIENT PIXEL PAIR
	/	
	/	Both cases ({N,M} and {S,M}) use the same variables,
	/	in order to handle them in parallel.
	*/
	bool pairN = gradientN >= gradientS;
	if(!pairN) lumaN = lumaS;
    if(!pairN) gradientN = gradientS;
    if(!pairN) lengthSign *= -1.0f;
	vec2 posN;
    posN.x = uvCoord.x + (horzSpan ? 0.0f : lengthSign * 0.5f);
    posN.y = uvCoord.y + (horzSpan ? lengthSign * 0.5f : 0.0f);
	
	/*
	/	CHOOSE SEARCH LIMITING VALUES
	*/
	gradientN *= FXAA_SEARCH_THRESHOLD;
	
	/*
	/	END OF EDGE SEARCH
	*/
	vec2 posP = posN;
    vec2 offNP = horzSpan ? vec2(h.x, 0.0f) : vec2(0.0f, h.y);
    float lumaEndN = lumaN;
    float lumaEndP = lumaN;
	bool doneN = false;
    bool doneP = false;
	if(FXAA_SEARCH_ACCELERATION == 1)
	{
		posN += offNP * vec2(-1.0, -1.0);
        posP += offNP * vec2( 1.0,  1.0);
	}
	if(FXAA_SEARCH_ACCELERATION == 2)
	{
		posN += offNP * vec2(-1.5, -1.5);
        posP += offNP * vec2( 1.5,  1.5);
        offNP *= vec2(2.0, 2.0);
	}
	if(FXAA_SEARCH_ACCELERATION == 3)
	{
		posN += offNP * vec2(-2.0, -2.0);
        posP += offNP * vec2( 2.0,  2.0);
        offNP *= vec2(3.0, 3.0);
	}
	if(FXAA_SEARCH_ACCELERATION == 4)
	{
		posN += offNP * vec2(-2.5, -2.5);
        posP += offNP * vec2( 2.5,  2.5);
        offNP *= vec2(4.0, 4.0);
	}
	for(int i = 0; i < FXAA_SEARCH_STEPS; i++)
	{
		if(FXAA_SEARCH_ACCELERATION == 1)
		{
			if(!doneN) lumaEndN = FxaaLuma(texture2D(inputImage, posN).xyz);
			if(!doneP) lumaEndP = FxaaLuma(texture2D(inputImage, posP).xyz);
		}
		else
		{
			if(!doneN) lumaEndN = FxaaLuma(textureGrad(inputImage, posN, offNP, offNP).xyz);
			if(!doneP) lumaEndP = FxaaLuma(textureGrad(inputImage, posP, offNP, offNP).xyz);
		}
		doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
		doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
		if(doneN && doneP) break;
		if(!doneN) posN -= offNP;
		if(!doneN) posP += offNP;
	}
	
	/*
	/	HANDLE IF CENTER IS ON POSITIVE OR NEGATIVE SIDE
	*/
	float dstN = horzSpan ? uvCoord.x - posN.x : uvCoord.y - posN.y;
    float dstP = horzSpan ? posP.x - uvCoord.x : posP.y - uvCoord.y;
	bool directionN = dstN < dstP;
	lumaEndN = directionN ? lumaEndN : lumaEndP;
	
	/*
	/	CHECK IF PIXEL IS IN SECTION OF SPAN WHICH GETS NO FILTERING
	*/
	if(((lumaM - lumaN) < 0.0) == ((lumaEndN - lumaN) < 0.0))
	{
		lengthSign = 0.0;
	}
	
	/*
	/	COMPUTE SUB-PIXEL OFFSET AND FILTER SPAN
	*/
	float spanLength = (dstP + dstN);
    dstN = directionN ? dstN : dstP;
	float subPixelOffset = (0.5 + (dstN * (-1.0/spanLength))) * lengthSign;
	
	vec3 rgbF = textureLod(inputImage, vec2(uvCoord.x + (horzSpan ? 0.0 : subPixelOffset),uvCoord.y + (horzSpan ? subPixelOffset : 0.0)),0.0).xyz;
	#if FXAA_SUBPIX == 0
		return rgbF;
	#else
		return mix(rgbF, rgbL, blendL);
	#endif
}

void main()
{
	fragColour = vec4(fxaa(),1.0f);
}