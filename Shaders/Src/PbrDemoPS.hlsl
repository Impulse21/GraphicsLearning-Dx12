#include "BRDFFunctions.hlsli"

// -- Const buffers ---

#define DRAW_FLAG_ALBEDO        0x01
#define DRAW_FLAG_NORMAL        0x02
#define DRAW_FLAG_ROUGHNESS     0x04
#define DRAW_FLAG_METALLIC      0x08
#define DRAW_FLAG_AO            0x10
#define DRAW_FLAG_TANGENT       0x20
#define DRAW_FLAG_BITANGENT     0x40


struct DrawInfo
{
    uint InstanceIndex;
    
    float3 Albedo;
    float Metallic;
    float Roughness;
    float Ao;

    uint AlbedoTexIndex;
    uint NormalTexIndex;
    uint MetallicTexIndex;
    uint RoughnessTexIndex;
    uint AoTexIndex;
    uint DrawFlags;
};

ConstantBuffer<DrawInfo> DrawInfoCB : register(b0);

struct SceneInfo
{
    matrix ViewProjection;
    float3 CameraPosition;
    float3 SunDirection;
    float3 SunColour;
    uint IrradianceMapTexIndex;
    uint PreFilteredEnvMapTexIndex;
    uint BrdfLUTTexIndex;
};

ConstantBuffer<SceneInfo> SceneInfoCB : register(b1);

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
    float3 TangentWS : TANGENT;
};
    
// Constant normal incidence Fresnel factor for all dielectrics.
static const float Fdielectric = 0.04f;
static const float MaxReflectionLod = 7.0f;
float4 main(PSInput input) : SV_Target
{
    // -- Collect Material Data ---
    float3 albedo = DrawInfoCB.Albedo;
    if (DrawInfoCB.AlbedoTexIndex != InvalidDescriptorIndex)
    {
        albedo = Texture2DTable[DrawInfoCB.AlbedoTexIndex].Sample(SamplerDefault, input.TexCoord).xyz;
    }
    
    if (DrawInfoCB.DrawFlags & DRAW_FLAG_ALBEDO)
    {
        return float4(albedo, 1.0f);
    }
    
    float metallic = DrawInfoCB.Metallic;
    if (DrawInfoCB.MetallicTexIndex != InvalidDescriptorIndex)
    {
        metallic = Texture2DTable[DrawInfoCB.MetallicTexIndex].Sample(SamplerDefault, input.TexCoord).r;
    }
    
    if (DrawInfoCB.DrawFlags & DRAW_FLAG_METALLIC)
    {
        return float4(metallic, metallic, metallic, 1.0f);
    }
    
    float roughness = DrawInfoCB.Roughness;
    if (DrawInfoCB.RoughnessTexIndex != InvalidDescriptorIndex)
    {
        roughness = Texture2DTable[DrawInfoCB.RoughnessTexIndex].Sample(SamplerDefault, input.TexCoord).r;
    }
    
    if (DrawInfoCB.DrawFlags & DRAW_FLAG_ROUGHNESS)
    {
        return float4(roughness, roughness, roughness, 1.0f);
    }
    
    float ao = DrawInfoCB.Ao;
    if (DrawInfoCB.AoTexIndex != InvalidDescriptorIndex)
    {
        ao = Texture2DTable[DrawInfoCB.AoTexIndex].Sample(SamplerDefault, input.TexCoord).r;
    }
    
    if (DrawInfoCB.DrawFlags & DRAW_FLAG_AO)
    {
        return float4(ao, ao, ao, 1.0f);
    }
    
    float3 tangent = normalize(input.TangentWS.xyz);
    float3 normal = normalize(input.NormalWS);
    float3 biTangent = cross(normal, tangent);
    if (DrawInfoCB.NormalTexIndex != InvalidDescriptorIndex)
    {
        float3x3 tbn = float3x3(tangent, biTangent, normal);
        normal = Texture2DTable[DrawInfoCB.AlbedoTexIndex].Sample(SamplerDefault, input.TexCoord).rgb * 2.0 - 1.0;;
        normal = normalize(mul(normal, tbn));
    }
    
    if (DrawInfoCB.DrawFlags & DRAW_FLAG_NORMAL)
    {
        return float4(normal,1.0f);
    }
    
    if (DrawInfoCB.DrawFlags & DRAW_FLAG_TANGENT)
    {
        return float4(tangent, 1.0f);
    }
    
    if (DrawInfoCB.DrawFlags & DRAW_FLAG_BITANGENT)
    {
        return float4(biTangent, 1.0f);
    }
    // -- End Material Collection ---
    
    // -- Lighting Model ---
    float3 N = normalize(normal);
    float3 V = normalize(SceneInfoCB.CameraPosition - input.PositionWS);
    float3 R = reflect(-V, N);
    
    // Linear Interpolate the value against the abledo as matallic
    // surfaces reflect their colour.
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    
    // Reflectance equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    {
        // -- Iterate over lights here
        // If this is a point light, calculate vector from light to World Pos
        float3 L = normalize(SceneInfoCB.SunDirection);
        float3 H = normalize(V + L);
    
        // If point light, calculate attenuation here;
        float3 radiance = SceneInfoCB.SunColour; // * attenuation;
    
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
    if (SceneInfoCB.IrradianceMapTexIndex != InvalidDescriptorIndex &&
        SceneInfoCB.PreFilteredEnvMapTexIndex != InvalidDescriptorIndex &&
        SceneInfoCB.BrdfLUTTexIndex != InvalidDescriptorIndex)
    {
        
        float3 F = FresnelSchlick(saturate(dot(N, V)), F0, roughness);
        // Improvised abmient lighting by using the Env Irradance map.
        float3 irradiance = TextureCubeTable[SceneInfoCB.IrradianceMapTexIndex].Sample(SamplerDefault, N).rgb;
        
        float3 kSpecular = F;
        float3 kDiffuse = 1.0 - kSpecular;
        float3 diffuse = irradiance * albedo;
        
        // Sample both the BRDFLut and Pre-filtered map and combine them together as per the
        // split-sum approximation to get the IBL Specular part.
        float lodLevel = roughness * MaxReflectionLod;
        float3 prefilteredColour =
            TextureCubeTable[SceneInfoCB.PreFilteredEnvMapTexIndex].SampleLevel(SamplerDefault, R, lodLevel).rgb;
        
        float2 brdfTexCoord = float2(saturate(dot(N, V)), roughness);
        
        float2 brdf = Texture2DTable[SceneInfoCB.BrdfLUTTexIndex].Sample(SamplerBrdf, brdfTexCoord).rg;
        
        float3 specular = prefilteredColour * (F * brdf.x + brdf.y);   
        
        ambient = (kDiffuse * diffuse + specular) * ao;
    }
        
    float3 colour = ambient + Lo;
    
    // Correction for gamma?
    colour = colour / (colour + float3(1.0, 1.0, 1.0));
    colour = pow(colour, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(colour, 1.0f);
}