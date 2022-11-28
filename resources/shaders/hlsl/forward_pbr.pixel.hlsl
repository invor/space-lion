// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos     : SV_POSITION;
	float3 normal  : NORMAL;
	float3 camPos  : TEXCOORD0; //feels so hacky
	float3 worldPos: TEXCOORD1; //feels so hacky
};

// A constant buffer that stores the model transform.
cbuffer ModelConstantBuffer : register(b0)
{
	float4x4 model;

	float4x4 normalMatrix;
	float4   albedo_colour;
	float4   specular_colour;
	float    roughness;
	float3   padding10;
	float4   padding11;

	float4x4 padding2;
};

cbuffer ViewProjectionConstantBuffer : register(b1)
{
	float4x4 viewProjection;
	float4x4 viewInv;

	float4   sunlight_direction_illuminance;
};

SamplerState CubmapSamplerState;
TextureCube IrradianceMap;

#define PI 3.1415926535
#define INV_PI 0.318309886183

float3 Uncharted2Tonemap(float3 x)
{
	float3 A = float3(0.15, 0.15, 0.15);
	float3 B = float3(0.50, 0.50, 0.50);
	float3 C = float3(0.10, 0.10, 0.10);
	float3 D = float3(0.20, 0.20, 0.20);
	float3 E = float3(0.02, 0.02, 0.02);
	float3 F = float3(0.30, 0.30, 0.30);

	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 FresnelSchlick(float3 specular_col, float3 E, float3 H)
{
	return specular_col + (1.0f - specular_col) * pow(1.0f - saturate(dot(E, H)), 5);
}

float3 blinnPhongShading(
	float3 diffuse_col,
	float3 specular_col,
	float specular_power,
	float3 N,
	float3 H,
	float3 L)
{
	float NdotL = clamp(dot(N, L), 0.0f, 1.0f);

	float3 diffuse = (diffuse_col / PI) * NdotL;
	float3 specular = FresnelSchlick(specular_col, L, H) * ((specular_power + 2.0f) / 8.0f) * pow(saturate(clamp(dot(N, H), 0.0f, 1.0f)), specular_power) * NdotL;

	return (diffuse + specular);
}

// The pixel shader passes through the color data. The color data from 
// is interpolated and assigned to a pixel at the rasterization step.
min16float4 Main(PixelShaderInput input) : SV_TARGET
{
	float3 surface_albedo = albedo_colour;
	float3 surface_specular_color = specular_colour;
	float  surface_roughness = roughness;
	float3 surface_normal = input.normal;
	float3 light_direction = normalize(float3(1.0f, 1.0f, 1.0f));
	float3 viewer_direction = normalize(input.camPos - input.worldPos);
	float3 light_colour = float3(100000.0f, 100000.0f, 100000.0f); //given in Lumen

	float3 reflection_direction = reflect(-viewer_direction, surface_normal);

	float3 halfway = normalize(light_direction + viewer_direction);
	// map PBR roughness values to reasonable blinn phong specular power
	float spec_pow = 1.0f / (pow(surface_roughness, 2.0f) + 0.001);


	float3 color_rgb_linear = light_colour * blinnPhongShading(
												surface_albedo,
												surface_specular_color,
												spec_pow,
												surface_normal,
												halfway,
												light_direction);

	//	// fake ambient glossly reflection
	//	float3 ambient_specular = IrradianceMap.Sample(CubmapSamplerState, /*surface_normal*/reflection_direction).rgb;
	//	color_rgb_linear += surface_specular_color * ambient_specular * 5000.0f;
	//	
	//	// fake ambient diffuse lighting
	//	float3 ambient_diffuse = IrradianceMap.Sample(CubmapSamplerState, surface_normal).rgb;
	//	color_rgb_linear += surface_albedo * ambient_diffuse * 5000.0f;

	color_rgb_linear *= 0.18 / 4000.0; // default value for mapping avg luminance of ~4000cd/m^2 to 0.18 intensity

	// Temporary tone mapping
	float3 W = float3(11.2, 11.2, 11.2);
	float ExposureBias = 2.0f;
	float3 curr = Uncharted2Tonemap(ExposureBias * color_rgb_linear);
	float3 whiteScale = float3(1.0f, 1.0f, 1.0f) / Uncharted2Tonemap(W);
	color_rgb_linear = curr * whiteScale;

	//color_rgb_linear = color_rgb_linear / (float3(1.0f,1.0f,1.0f) + color_rgb_linear);

	//	Temporary gamma correction
	float3 color_rgb = pow(color_rgb_linear, (1.0 / 2.2));

	return min16float4(color_rgb, albedo_colour.a);//input.color.rgb;, 1.0f);
	//return min16float4(input.normal, 1.0);//input.color.rgb;, 1.0f);
}
