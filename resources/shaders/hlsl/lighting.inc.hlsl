#define PI 3.1415926535
#define INV_PI 0.318309886183

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