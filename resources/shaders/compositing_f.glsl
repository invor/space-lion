#version 430

uniform sampler2D lighting_tx2D;
uniform sampler2D atmosphere_tx2D;
//uniform sampler2D ocean_tx2D;
//uniform sampler2D volume_tx2D;

uniform float exposure;

in vec2 uvCoord;

layout (location = 0) out vec4 fragColour;


float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 Uncharted2Tonemap(vec3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main()
{
    vec4 geometry = texture(lighting_tx2D,uvCoord);
    vec4 atmosphere = texture(atmosphere_tx2D,uvCoord);
    //vec4 ocean = texture(ocean_tx2D,uvCoord);
    //vec4 volumetrics = texture(volume_tx2D,uvCoord);

    vec4 rgb_depth_linear = vec4(0.0,0.0,0.0,10000.0);
    
    rgb_depth_linear = atmosphere;

    if( geometry.a < rgb_depth_linear.a  && geometry.a > 0.001 )
        rgb_depth_linear = geometry;

    /* Adjust exposure */
    rgb_depth_linear.rgb *= exposure;
    //rgb_depth_linear.rgb *= 0.18/5000.0; // default value for mapping avg luminance of ~4000cd/m^2 to 0.18 intensity


    //rgb_depth_linear.rgb = ocean.rgb * ocean.a + (1.0-ocean.a) * rgb_depth_linear.rgb;

    //rgb_depth_linear.rgb = rgb_depth_linear.rgb * (1.0 - volumetrics.a) + volumetrics.rgb * volumetrics.a;

    //  float fog_value = 1.0 - exp( -(rgb_depth_linear.a) * 0.0004);
    //  if(rgb_depth_linear.a > 10000.0)
    //      fog_value = 0.0;
	//  fog_value = clamp(fog_value,0.0,1.0);
	//  rgb_depth_linear.rgb = mix(rgb_depth_linear.rgb,vec3(0.3,0.3,0.4), fog_value );

    //  Temporary tone mapping
    //rgb_depth_linear.rgb = rgb_depth_linear.rgb/(1.0+rgb_depth_linear.rgb);
    float ExposureBias = 2.0f;
    vec3 curr = Uncharted2Tonemap(ExposureBias*rgb_depth_linear.rgb);
    vec3 whiteScale = 1.0f/Uncharted2Tonemap(vec3(W));
    rgb_depth_linear.rgb = rgb_depth_linear.rgb*whiteScale;

    //	Temporary gamma correction
	vec3 rgb = pow( rgb_depth_linear.rgb, vec3(1.0/2.2) );

    //rgb = vec3(1.0,0.0,0.0);

    fragColour = vec4(rgb, 1.0);

    //fragColour = vec4( texture(volume_tx2D,uvCoord).rgb,1.0);
}