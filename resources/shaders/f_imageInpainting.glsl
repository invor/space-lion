/*
---------------------------------------------------------------------------------------------------
File: f_imageInpainting.glsl
Author: Michael Becher
Date of (presumingly) last edit: 17.04.2013

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
/	Helper function for calculating luma using red and green channel with magic numbers.
/	The origin of the magic numbers is NVIDIA's whitepaper on FXAA by Timithy Lottes.
*/
float luma(vec3 rgb)
{
	return (rgb.y * (0.587f/0.299f) + rgb.x);
}

vec4 imageInpainting(vec2 pos, vec2 h)
{
	/*
	/	Vectors to the neighbour texels
	*/
	vec2 vN = vec2(0.0f,h.y);
	vec2 vNE = vec2(h.x,h.y);
	vec2 vE = vec2(h.x,0.0f);
	vec2 vSE = vec2(h.x,-h.y);
	vec2 vS = vec2(0.0f,-h.y);
	vec2 vSW = vec2(-h.x,-h.y);
	vec2 vW = vec2(-h.x,0.0f);
	vec2 vNW = vec2(-h.x,h.y);
	
	/*
	/	Obtain rgba values of a 3x3 stencil
	/
	/	NW | N | NE
	/	-----------
	/	W  | M |  E
	/	-----------
	/	SW | S | SE
	/
	/	Position M inside the stencil equals the position of the current fragment.
	/
	/	If the pixel at M has already been inpainted, exit early.
	*/
	vec4 rgbaM = texture2D(inputImage,pos);
	if(rgbaM.a > 0.0f) return rgbaM;
	vec4 rgbaN = texture2D(inputImage,pos+vN);
	vec4 rgbaNE = texture2D(inputImage,pos+vNE);
	vec4 rgbaE = texture2D(inputImage,pos+vE);
	vec4 rgbaSE = texture2D(inputImage,pos+vSE);
	vec4 rgbaS = texture2D(inputImage,pos+vS);
	vec4 rgbaSW = texture2D(inputImage,pos+vSW);
	vec4 rgbaW = texture2D(inputImage,pos+vW);
	vec4 rgbaNW = texture2D(inputImage,pos+vNW);
	/*
	/	If all stencil pixels have opacity = 0, then exit early as the current pixel
	/	will be inpainted in an upcoming iterations.
	*/
	if((rgbaN.a+rgbaNE.a+rgbaE.a+rgbaSE.a+rgbaS.a+rgbaSW.a+rgbaW.a+rgbaNW.a)<0.1) return vec4(0.0);
	/*
	/	Also, we now have enough information to find out a bit more about our stencil's
	/	position within the inpainting region. Specifically we want to know if the
	/	pixels north,south,west and east of M have valid values (since we assume
	/	inpainting regions to be rectangular, this four will give us all the information
	/	we need). This will help with more accurate finite differentials later on.
	*/
	bool validN = ((rgbaN.a > 0.0f) ? true : false);
	bool validE = ((rgbaE.a > 0.0f) ? true : false);
	bool validS = ((rgbaS.a > 0.0f) ? true : false);
	bool validW = ((rgbaW.a > 0.0f) ? true : false);
	
	/*
	/	Obtain some more values around the stencil.
	/	These are used later to calculate gradients/isophotes.
	*/
	vec4 rgbaNN = texture2D(inputImage,pos+vN+vN);
	vec4 rgbaNNE = texture2D(inputImage,pos+vN+vN+vE);
	vec4 rgbaNEE = texture2D(inputImage,pos+vN+vE+vE);
	vec4 rgbaEE = texture2D(inputImage,pos+vE+vE);
	vec4 rgbaSEE = texture2D(inputImage,pos+vS+vE+vE);
	vec4 rgbaSSE = texture2D(inputImage,pos+vS+vS+vE);
	vec4 rgbaSS = texture2D(inputImage,pos+vS+vS);
	vec4 rgbaSSW = texture2D(inputImage,pos+vS+vS+vW);
	vec4 rgbaSWW = texture2D(inputImage,pos+vS+vW+vW);
	vec4 rgbaWW = texture2D(inputImage,pos+vW+vW);
	vec4 rgbaNWW = texture2D(inputImage,pos+vN+vW+vW);
	vec4 rgbaNNW = texture2D(inputImage,pos+vN+vN+vW);
	
	/*
	/	Obtain luminance values from the rgb values
	*/
	float lumaM = luma(rgbaM.xyz);
	float lumaN = luma(rgbaN.xyz);
	float lumaNE = luma(rgbaNE.xyz);
	float lumaE = luma(rgbaE.xyz);
	float lumaSE = luma(rgbaSE.xyz);
	float lumaS = luma(rgbaS.xyz);
	float lumaSW = luma(rgbaSW.xyz);
	float lumaW = luma(rgbaW.xyz);
	float lumaNW = luma(rgbaNW.xyz);
	
	float lumaNN = luma(rgbaNN.xyz);
	float lumaNNE = luma(rgbaNNE.xyz);
	float lumaNEE = luma(rgbaNEE.xyz);
	float lumaEE = luma(rgbaEE.xyz);
	float lumaSEE = luma(rgbaSEE.xyz);
	float lumaSSE = luma(rgbaSSE.xyz);
	float lumaSS = luma(rgbaSS.xyz);
	float lumaSSW = luma(rgbaSSW.xyz);
	float lumaSWW = luma(rgbaSWW.xyz);
	float lumaWW = luma(rgbaWW.xyz);
	float lumaNWW = luma(rgbaNWW.xyz);
	float lumaNNW = luma(rgbaNNW.xyz);
	
	/*
	/	Calculate the isophote directions (orthogonal to the gradient).
	/	Depening on the position inside the stencil, as well as the stencil position inside
	/	the inpainting region, a different discrete differantial scheme is used to avoid
	/	hitting pixels in the inpainting domain that haven't been set yet:
	/
	/	Standard texture coordinate system:
	/	
	/	t
	/	|
	/	|
	/	0,0----s
	/
	/	The four direct neighbours can stick to one scheme all the time, since this will
	/	either use valid pixels, or they themselves have no valid values and therefore
	/	no impact on the calculations for M.
	/	N  -> forward difference (f) in t, central differecence (c) in s
	/	E  -> c in t, f in s
	/	S  -> b in t, c in s
	/	W  -> c in t, b in s
	/
	/	The corner pixels of the stencil need more consideration. A central difference
	/	approximation is prefered but not always possible due to missing image information.
	/	General idea: Check relevant pixels (N,E,S,W) and use central difference if possible,
	/	else use forward or backward difference respectivly.
	*/
	vec2 isoN = vec2( -((rgbaM.a>0.0f) ? (lumaNN-lumaM)*0.5f : (lumaNN-lumaN)) ,
						((rgbaNE.a>0.0f) ? ((rgbaNW.a>0.0f) ? (lumaNE-lumaNW)*0.5f : (lumaNE-lumaN) ) : (lumaN-lumaNW)) );
	vec2 isoNE = vec2( -(validE ? (lumaNNE-lumaE)*0.5f: (lumaNNE-lumaNE)) ,
						(validN ? (lumaNEE-lumaN)*0.5f : (lumaNEE-lumaNE)) );
	vec2 isoE = vec2( ((rgbaNE.a>0.0f) ? ((rgbaSE.a>0.0f) ? -(lumaNE-lumaSE)*0.5f : -(lumaNE-lumaE) ) : -(lumaE-lumaSE)) ,
						((rgbaM.a>0.0f) ? (lumaEE-lumaM)*0.5f : (lumaEE-lumaE)) );
	vec2 isoSE = vec2( -(validE ? (lumaE-lumaSSE)*0.5f : (lumaSE-lumaSSE)) ,
						(validS ? (lumaSEE-lumaS)*0.5f : (lumaSEE-lumaSE)) );
	vec2 isoS = vec2( -((rgbaM.a>0.0f) ? (lumaM-lumaSS)*0.5f : (lumaS-lumaSS)) ,
						((rgbaSE.a>0.0f) ? ((rgbaSW.a>0.0f) ? (lumaSE-lumaSW)*0.5f : (lumaSE-lumaS) ) : (lumaS-lumaSW)) );
	vec2 isoSW = vec2( -(validW ? (lumaW-lumaSSW)*0.5f : (lumaSW-lumaSSW)) ,
						(validS ? (lumaS-lumaSWW)*0.5f : (lumaSW-lumaSWW)) );
	vec2 isoW = vec2( ((rgbaNW.a>0.0f) ? ((rgbaSW.a>0.0f) ? -(lumaNW-lumaSW)*0.5f : -(lumaNW-lumaW) ) : -(lumaW-lumaSW)) ,
						((rgbaM.a>0.0f) ? (lumaM-lumaWW)*0.5f : (lumaW-lumaWW)) );
	vec2 isoNW = vec2( -(validW ? (lumaNNW-lumaW)*0.5f : (lumaNNW-lumaNW)) ,
						(validN ? (lumaN-lumaNWW)*0.5f : (lumaNW-lumaNWW)) );
	
	/*
	/	Check if the isophotes are pointing towards our center pixel M by projecting
	/	the isophote direction onto the vectors from M towards respective stencil pixel.
	/	
	/	These values will serve as weights of the stencil pixels and are expected to be
	/	between 0 and 1.
	*/
	float isoProjN = abs( dot( normalize(isoN), normalize(vN) ) );
	float isoProjNE = abs( dot( normalize(isoNE), normalize(vNE) ) );
	float isoProjE = abs( dot( normalize(isoE), normalize(vE) ) );
	float isoProjSE = abs( dot( normalize(isoSE), normalize(vSE) ) );
	float isoProjS = abs( dot( normalize(isoS), normalize(vS) ) );
	float isoProjSW = abs( dot( normalize(isoSW), normalize(vSW) ) );
	float isoProjW = abs( dot( normalize(isoW), normalize(vW) ) );
	float isoProjNW = abs( dot( normalize(isoNW), normalize(vNW) ) );
	
	/*
	/	Sum up all stencil pixels but M and weight them together.
	*/
	float weightSum = 0.0f;
	vec3 rgbF = vec3(0.0f);
	
	/*
	/	Pixels inside the inpainting domain that have not been inpainted yet are expected
	/	to have an opacity of 0, while others are expected to have an opacity of 1.
	/	Only pixels outside the domain, or already inpainted ones, are taken into consideration
	/	here. Therefore we multiply by the alpha channel before adding them our final pixel
	/	color rgbF.
	*/
	float weightN = (isoProjN * rgbaN.a)/(length(vN));
	rgbF += ((rgbaN.rgb) * weightN);
	weightSum += weightN;
	float weightNE = (isoProjNE * rgbaNE.a)/(length(vNE));
	rgbF += ((rgbaNE.rgb) * weightNE);
	weightSum += weightNE;
	float weightE = (isoProjE * rgbaE.a)/(length(vE));
	rgbF += ((rgbaE.rgb) * weightE);
	weightSum += weightE;
	float weightSE = (isoProjSE * rgbaSE.a)/(length(vSE));
	rgbF += ((rgbaSE.rgb) * weightSE);
	weightSum += weightSE;
	float weigthS = (isoProjS * rgbaS.a)/(length(vS));
	rgbF += ((rgbaS.rgb) * weigthS);
	weightSum += weigthS;
	float weightSW = (isoProjSW * rgbaSW.a)/(length(vSW));
	rgbF += ((rgbaSW.rgb) * weightSW);
	weightSum += weightSW;
	float weightW = (isoProjW * rgbaW.a)/(length(vW));
	rgbF += ((rgbaW.rgb) * weightW);
	weightSum += weightW;
	float weightNW = (isoProjNW * rgbaNW.a)/(length(vNW));
	rgbF += ((rgbaNW.rgb) * weightNW);
	weightSum += weightNW;

	/*
	/	Alternative weighting scheme,...doesn't work too well though.
	*/
	//float weightN = max(((length(isoN)<0.1) ? 0.25 : 0.0),((isoProjN>0.9f) ? 1.0 : 0.0)) * rgbaN.a;
	//rgbF += ((rgbaN.rgb) * weightN);
	//weightSum += max(0.0,weightN);
	//float weightE =  max(((length(isoE)<0.1) ? 0.25 : 0.0),((isoProjE>0.9f) ? 1.0 : 0.0)) * rgbaE.a;
	//rgbF += ((rgbaE.rgb) * weightE);
	//weightSum += max(0.0,weightE);
	//float weightS = max(((length(isoS)<0.1) ? 0.25 : 0.0),((isoProjS>0.9f) ? 1.0 : 0.0)) * rgbaS.a;
	//rgbF += ((rgbaS.rgb) * weightS);
	//weightSum += max(0.0,weightS);
	//float weightW = max(((length(isoW)<0.1) ? 0.25 : 0.0),((isoProjW>0.9f) ? 1.0 : 0.0)) * rgbaW.a;
	//rgbF += ((rgbaW.rgb) * weightW);
	//weightSum += max(0.0,weightW);
	//
	//float weightSW = max(((length(isoSW)<0.1) ? 0.0 : 0.0),((isoProjSW>0.9f) ? 0.5 : 0.0)) * rgbaSW.a;
	//rgbF += ((rgbaSW.rgb) * weightSW);
	//weightSum += weightSW;
	//float weightSE = max(((length(isoSE)<0.1) ? 0.0 : 0.0),((isoProjSE>0.9f) ? 0.5 : 0.0)) * rgbaSE.a;
	//rgbF += ((rgbaSE.rgb) * weightSE);
	//weightSum += weightSE;
	//float weightNE = max(((length(isoNE)<0.1) ? 0.0 : 0.0),((isoProjNE>0.9f) ? 0.5 : 0.0)) * rgbaNE.a;
	//rgbF += ((rgbaNE.rgb) * weightNE);
	//weightSum += weightNE;
	//float weightNW = max(((length(isoNW)<0.1) ? 0.0	: 0.0),((isoProjNW>0.9f) ? 0.5 : 0.0)) * rgbaNW.a;
	//rgbF += ((rgbaNW.rgb) * weightNW);
	//weightSum += weightNW;
	
	rgbF = (rgbF / weightSum);
	
	return vec4(rgbF,1.0f);
}

void main()
{
	/*
	/	Check if the fragment is inside the inpainting domain.
	*/
	if(texture2D(mask,uvCoord).x < 0.5f)
	{
		/*
		/	Calculate the distance between two neighbour pixels in texture space.
		*/
		vec2 h = vec2(1.0f/imgDim.x, 1.0f/imgDim.y);
		/*
		/	Get the rgba value from the inpainting function.
		*/
		fragColour = vec4(imageInpainting(uvCoord, h));
	}
	else
	{	
		fragColour = vec4(texture2D(inputImage,uvCoord).xyz,1.0);
	}
}