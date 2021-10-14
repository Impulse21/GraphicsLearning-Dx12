#include "BRDFFunctions.hlsli"


// -- Const buffers ---

struct DrawInfo
{
    matrix WorldMatrix;
    matrix ModelViewProjectMatrix;
    float3 CameraPosition;
    
    float3 SunDirection;
    float3 SunColour;
};

ConstantBuffer<DrawInfo> DrawInfoCB : register(b0);

struct Material
{
    float3 Albedo;
    float Metallic;
    float Roughness;
    float Ao;
};
    
ConstantBuffer<Material> MaterialCB : register(b1);

struct PSInput
{
    float3 NormalWS : NORMAL;
    float4 Colour : COLOUR;
    float2 TexCoord : TEXCOORD;
    float3 PositionWS : Position;
};
    
// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

float4 main(PSInput input) : SV_Target
{
    // -- Collect Material Data ---
    float3 albedo = MaterialCB.Albedo;
    float metallic = MaterialCB.Metallic;
    float roughness = MaterialCB.Roughness;
    float ao = MaterialCB.Ao;
    // -- End Material Collection ---
    
    float3 N = normalize(input.NormalWS);
    float3 V = normalize(DrawInfoCB.CameraPosition - input.PositionWS);
        
    // Linear Interpolate the value against the abledo as matallic
    // surfaces reflect their colour.
    float3 F0 = lerp(0.04, albedo, metallic);
    
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    // -- Iterate over lights here
    // If this is a point light, calculate vector from light to World Pos
    float3 L = normalize(DrawInfoCB.SunDirection);
    float3 H = normalize(V + L);
    
    // If point light, calculate attenuation here;
    float3 radiance = DrawInfoCB.SunColour; // * attenuation;
    
    // Calculate Normal Distribution Term
    float NDF = DistributionGGX(N, H, roughness);
    
    // Calculate Geometry Term
    float G = GeometrySmith(N, V, L, roughness);

    // Calculate Fersnel Term
    float3 F = FresnelSchlick(saturate(dot(H, V)), F0);
    
    // Now calculate Cook-Torrance BRDF
    float3 numerator = NDF * G * F;
    
    // NOTE: we add 0.0001 to the denomiator to prevent a divide by zero in the case any dot product ends up zero
    float denominator = 4.0 * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001;

    float3 specular = numerator / denominator;
    
    // Now we can calculate the light's constribution to the reflectance equation. Since Fersnel Value directly corresponds to
    // Ks, we ca use F to denote the specular contribution of any light that hits the surface.
    // we can now deduce what the diffuse contribution is as 1.0 = KSpecular + kDiffuse;
    float3 kSpecular = F;
    float3 KDiffuse = float3(1.0, 1.0, 1.0) - kSpecular;
    KDiffuse *= 1.0f - metallic;
    
    float NdotL = saturate(dot(N, L));
    Lo += (KDiffuse * (albedo / PI) + specular) * radiance * NdotL;
    // -- End light iteration
    
    
    // Improvised abmient lighting
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    
    float3 colour = ambient + Lo;
    // Correction for gamma?
    colour = colour / (colour + float3(1.0, 1.0, 1.0));
    colour = pow(colour, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(colour, 1.0f);
}