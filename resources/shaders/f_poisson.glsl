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

//	Normalized coordinates [0,1]x[0,1] of the fragment
in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	if(uvCoord.x > lowerBound && uvCoord.x < upperBound && uvCoord.y > lowerBound && uvCoord.y < upperBound)
	{
		vec2 h = vec2(1.0f/imgDim.x, 1.0f/imgDim.y);
		vec3 rgbN = texture2D(inputImage,uvCoord+vec2(0.0f,h.y)).xyz;
		vec3 rgbW = texture2D(inputImage,uvCoord+vec2(-h.x,0.0f)).xyz;
		vec3 rgbE = texture2D(inputImage,uvCoord+vec2(h.x,0.0f)).xyz;
		vec3 rgbS = texture2D(inputImage,uvCoord+vec2(0.0f,-h.y)).xyz;
		
		vec3 rgbNW = texture2D(inputImage,uvCoord+vec2(-h.x,h.y)).xyz;
		vec3 rgbNE = texture2D(inputImage,uvCoord+vec2(h.x,h.y)).xyz;
		vec3 rgbSW = texture2D(inputImage,uvCoord+vec2(-h.x,-h.y)).xyz;
		vec3 rgbSE = texture2D(inputImage,uvCoord+vec2(h.x,-h.y)).xyz;
		
		vec3 rgbL = (rgbN + rgbW + rgbE +rgbS) * vec3(1.0f/4.0f);
		
		fragColour = vec4(rgbL,1.0);
	}
	else
	{
		fragColour = vec4(texture2D(inputImage,uvCoord).xyz,1.0);
	}
}