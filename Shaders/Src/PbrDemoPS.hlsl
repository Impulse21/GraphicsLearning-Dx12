#include "BRDFFunctions.hlsli"

// -- Const buffers ---

struct DrawInfo
{
    matrix WorldMatrix;
    matrix ModelViewProjectMatrix;
    float3 CameraPosition;
};

ConstantBuffer<DrawInfo> DrawInfoCB : register(b0);

struct Material
{
    float3 Albedo;
    float Metallic;
    float Roughness;
    float Ao;

    uint AlbedoTexIndex;
    uint NormalTexIndex;
    uint MetallicTexIndex;
    uint RoughnessTexIndex;
};
    
ConstantBuffer<Material> MaterialCB : register(b1);

struct Enviroment
{
    float3 SunDirection;
    uint __PADDING;
    float3 SunColour;
    uint IrradianceMapTexIndex;
    uint PreFilteredEnvMapTexIndex;
    uint BrdfLUTTexIndex;
};

ConstantBuffer<Enviroment> EnviromentCB : register(b2);

Texture2D   Texture2DTable[]    : register(t0, Tex2DSpace);
TextureCube TextureCubeTable[]  : register(t0, TexCubeSpace);

SamplerState SamplerDefault : register(s0);
SamplerState SamplerBrdf : register(s1);

struct PSInput
{
    float3 NormalWS : NORMAL;
    float4 Colour : COLOUR;
    float2 TexCoord : TEXCOORD;
    float3 PositionWS : Position;
};
    
// Constant normal incidence Fresnel factor for all dielectrics.
static const float Fdielectric = 0.04f;
static const float MaxReflectionLod = 7.0f;
float4 main(PSInput input) : SV_Target
{
    // -- Collect Material Data ---
    float3 albedo = MaterialCB.Albedo;
    if (MaterialCB.AlbedoTexIndex != InvalidDescriptorIndex)
    {
        albedo = Texture2DTable[MaterialCB.AlbedoTexIndex].Sample(SamplerDefault, input.TexCoord).xyz;
    }
    
    float metallic = MaterialCB.Metallic;
    if (MaterialCB.MetallicTexIndex != InvalidDescriptorIndex)
    {
        metallic = Texture2DTable[MaterialCB.MetallicTexIndex].Sample(SamplerDefault, input.TexCoord).r;
    }
    
    float roughness = MaterialCB.Roughness;
    if (MaterialCB.RoughnessTexIndex != InvalidDescriptorIndex)
    {
        roughness = Texture2DTable[MaterialCB.RoughnessTexIndex].Sample(SamplerDefault, input.TexCoord).r;
    }
    
    float ao = MaterialCB.Ao;
    
    float3 normal = input.NormalWS;
    if (MaterialCB.NormalTexIndex != InvalidDescriptorIndex)
    {
        return Texture2DTable[MaterialCB.AlbedoTexIndex].Sample(SamplerDefault, input.TexCoord);
    }
    // -- End Material Collection ---
    
    // -- Lighting Model ---
    float3 N = normalize(normal);
    float3 V = normalize(DrawInfoCB.CameraPosition - input.PositionWS);
    float3 R = reflect(-V, N);
    
    // Linear Interpolate the value against the abledo as matallic
    // surfaces reflect their colour.
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    
    // Reflectance equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    {
        // -- Iterate over lights here
        // If this is a point light, calculate vector from light to World Pos
        float3 L = normalize(EnviromentCB.SunDirection);
        float3 H = normalize(V + L);
    
        // If point light, calculate attenuation here;
        float3 radiance = EnviromentCB.SunColour; // * attenuation;
    
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
    }
    // -- End light iteration
    
    
    // Improvised abmient lighting by using the Env Irradance map.
    float3 ambient = float(0.03).xxx * albedo * ao;
    if (EnviromentCB.IrradianceMapTexIndex != InvalidDescriptorIndex &&
        EnviromentCB.PreFilteredEnvMapTexIndex != InvalidDescriptorIndex &&
        EnviromentCB.BrdfLUTTexIndex != InvalidDescriptorIndex)
    {
        
        float3 F = FresnelSchlick(saturate(dot(N, V)), F0, roughness);
        // Improvised abmient lighting by using the Env Irradance map.
        float3 irradiance = TextureCubeTable[EnviromentCB.IrradianceMapTexIndex].Sample(SamplerDefault, N).rgb;
        
        float3 kSpecular = F;
        float3 kDiffuse = 1.0 - kSpecular;
        float3 diffuse = irradiance * albedo;
        
        // Sample both the BRDFLut and Pre-filtered map and combine them together as per the
        // split-sum approximation to get the IBL Specular part.
        float lodLevel = roughness * MaxReflectionLod;
        float3 prefilteredColour =
            TextureCubeTable[EnviromentCB.PreFilteredEnvMapTexIndex].SampleLevel(SamplerDefault, R, lodLevel).rgb;
        
        float2 brdfTexCoord = float2(saturate(dot(N, V)), roughness);
        
        float2 brdf = Texture2DTable[EnviromentCB.BrdfLUTTexIndex].Sample(SamplerBrdf, brdfTexCoord).rg;
        
        float3 specular = prefilteredColour * (F * brdf.x + brdf.y);   
        
        ambient = (kDiffuse * diffuse + specular) * ao;
    }
    
    float3 colour = ambient + Lo;
    
    // Correction for gamma?
    colour = colour / (colour + float3(1.0, 1.0, 1.0));
    colour = pow(colour, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(colour, 1.0f);
}