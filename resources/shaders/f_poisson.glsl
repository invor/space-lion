#version 330

uniform sampler2D previousFrame;
uniform sampler2D inputImage;
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

out vec4 fragColour;


float luma(vec3 rgb)
{
	return rgb.y * (0.587f/0.299f) + rgb.x;
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

void main()
{
	if((uvCoord.x > lowerBound.x) && (uvCoord.x < upperBound.x) && (uvCoord.y > lowerBound.y) && (uvCoord.y < upperBound.y))
	{
		vec2 h = vec2(1.0f/imgDim.x, 1.0f/imgDim.y);
		
		fragColour = vec4(poissonImageEditing(uvCoord, h),1.0);
		//fragColour = vec4(seamlessCloning(uvCoord, h),1.0);
	}
	else
	{
		fragColour = vec4(texture2D(inputImage,uvCoord).xyz,1.0);
	}
}