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
	*/
	vec4 rgbaM = texture2D(inputImage,pos);
	/*
	/	If the pixel at M has already been inpainted, exit early.
	*/
	if(rgbaM.a > 0.0f)
	{
		return rgbaM;
	}
	vec4 rgbaN = texture2D(inputImage,pos+vN);
	vec4 rgbaNE = texture2D(inputImage,pos+vNE);
	vec4 rgbaE = texture2D(inputImage,pos+vE);
	vec4 rgbaSE = texture2D(inputImage,pos+vSE);
	vec4 rgbaS = texture2D(inputImage,pos+vS);
	vec4 rgbaSW = texture2D(inputImage,pos+vSW);
	vec4 rgbaW = texture2D(inputImage,pos+vW);
	vec4 rgbaNW = texture2D(inputImage,pos+vNW);
	/*
	/	If all stencil pixels have opacity = 0, then exit early as this pixel
	/	will be inpainted in an upcoming iterations.
	*/
	if((rgbaN.a+rgbaNE.a+rgbaE.a+rgbaSE.a+rgbaS.a+rgbaSW.a+rgbaW.a+rgbaNW.a)<0.1)
	{
		return vec4(0.0);
	}
	
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
	/	Depening on the position inside the stencil a different discrete differantial scheme
	/	is used to avoid hitting pixels in the inpainting domain that haven't been set yet:
	/
	/	Standard texture coordinate system:
	/	
	/	t
	/	|
	/	|
	/	0,0----s
	/
	/	N  -> forward difference (f) in t, central differecence (c) in s
	/	NE -> f in t, f in s
	/	E  -> c in t, f in s
	/	SE -> backward difference (b) in t, f in s
	/	S  -> b in t, c in s
	/	SW -> b in t, b in s
	/	W  -> c in t, b in s
	/	NW -> f in t, b in s
	*/
	vec2 isoN = vec2( -(lumaNN-lumaN) , (lumaNE-lumaNW)*0.5f );
	vec2 isoNE = vec2( -(lumaNNE-lumaNE) , (lumaNEE-lumaNE) );
	vec2 isoE = vec2( -(lumaNE-lumaSE)*0.5f , (lumaEE-lumaE) );
	vec2 isoSE = vec2( -(lumaSE-lumaSSE) , (lumaSEE-lumaSE) );
	vec2 isoS = vec2( -(lumaS-lumaSS) , (lumaSE-lumaSW)*0.5f );
	vec2 isoSW = vec2( -(lumaSW-lumaSSW) , (lumaSW-lumaSWW) );
	vec2 isoW = vec2( -(lumaNW-lumaSW)*0.5f , (lumaW-lumaWW) );
	vec2 isoNW = vec2( -(lumaNNW-lumaNW) , (lumaNW-lumaNWW) );
	
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
	rgbF += ((rgbaN.rgb) * isoProjN * rgbaN.a);
	weightSum += (isoProjN * rgbaN.a);
	rgbF += ((rgbaNE.rgb) * isoProjNE * rgbaNE.a);
	weightSum += (isoProjNE * rgbaNE.a);
	rgbF += ((rgbaE.rgb) * isoProjE * rgbaE.a);
	weightSum += (isoProjE * rgbaE.a);
	rgbF += ((rgbaSE.rgb)	* isoProjSE * rgbaSE.a);
	weightSum += (isoProjSE * rgbaSE.a);
	rgbF += ((rgbaS.rgb) * isoProjS * rgbaS.a);
	weightSum += (isoProjS * rgbaS.a);
	rgbF += ((rgbaSW.rgb) * isoProjSW * rgbaSW.a);
	weightSum += (isoProjSW * rgbaSW.a);
	rgbF += ((rgbaW.rgb) * isoProjW * rgbaW.a);
	weightSum += (isoProjW * rgbaW.a);
	rgbF += ((rgbaNW.rgb) * isoProjNW * rgbaNW.a);
	weightSum += (isoProjNW * rgbaNW.a);
	
	rgbF = rgbF / weightSum;
	//if(rgbaM.a > 0.0)
	//{
	//	return vec4(0.0,1.0,0.0,1.0);
	//}
	
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