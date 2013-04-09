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
	vec3 rgbN = texture2D(inputImage,pos+vec2(0.0f,h.y)).xyz;
	vec3 rgbW = texture2D(inputImage,pos+vec2(-h.x,0.0f)).xyz;
	vec3 rgbE = texture2D(inputImage,pos+vec2(h.x,0.0f)).xyz;
	vec3 rgbS = texture2D(inputImage,pos+vec2(0.0f,-h.y)).xyz;
	
	vec3 guideNx = calcGuidanceVectorX( (pos+pos+vec2(0.0f,h.y))*0.5f, h );
	vec3 guideNy = calcGuidanceVectorY( (pos+pos+vec2(0.0f,h.y))*0.5f, h );
	vec3 guideWx = calcGuidanceVectorX( (pos+pos+vec2(-h.x,0.0f))*0.5f, h );
	vec3 guideWy = calcGuidanceVectorY( (pos+pos+vec2(-h.x,0.0f))*0.5f, h );
	vec3 guideEx = calcGuidanceVectorX( (pos+pos+vec2(h.x,0.0f))*0.5f, h );
	vec3 guideEy = calcGuidanceVectorY( (pos+pos+vec2(h.x,0.0f))*0.5f, h );
	vec3 guideSx = calcGuidanceVectorX( (pos+pos+vec2(0.0f,-h.y))*0.5f, h );
	vec3 guideSy = calcGuidanceVectorY( (pos+pos+vec2(0.0f,-h.y))*0.5f, h );
	
	vec3 vN = vec3( dot(vec2(guideNx.r,guideNy.r), vec2(0.0f,h.y)),
					dot(vec2(guideNx.g,guideNy.g), vec2(0.0f,h.y)),
					dot(vec2(guideNx.b,guideNy.b), vec2(0.0f,h.y)));
	vec3 vW = vec3( dot(vec2(guideWx.r,guideWy.r), vec2(-h.x,0.0f)),
					dot(vec2(guideWx.g,guideWy.g), vec2(-h.x,0.0f)),
					dot(vec2(guideWx.b,guideWy.b), vec2(-h.x,0.0f)));				
	vec3 vE = vec3( dot(vec2(guideEx.r,guideEy.r), vec2(h.x,0.0f)),
					dot(vec2(guideEx.g,guideEy.g), vec2(h.x,0.0f)),
					dot(vec2(guideEx.b,guideEy.b), vec2(h.x,0.0f)));
	vec3 vS = vec3( dot(vec2(guideSx.r,guideSy.r), vec2(0.0f,-h.y)),
					dot(vec2(guideSx.g,guideSy.g), vec2(0.0f,-h.y)),
					dot(vec2(guideSx.b,guideSy.b), vec2(0.0f,-h.y)));
	
	vec3 rgbF = (rgbN + rgbW + rgbE + rgbS - vN - vW - vE - vS) * 0.25;
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
	if(uvCoord.x > lowerBound && uvCoord.x < upperBound && uvCoord.y > lowerBound && uvCoord.y < upperBound)
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