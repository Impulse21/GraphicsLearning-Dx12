#define Tex2DSpace space100

Texture2D Texture2DTable[] : register(t0, Tex2DSpace);

SamplerState DefaultSampler : register(s0);

// -- Const buffers ---
struct MaterialInfo
{
    uint albedoTextureIndex;
    float3 _emtpty;
};
    
ConstantBuffer<MaterialInfo> MaterialInfoCB : register(b0);

struct PSInput
{
    float4 Colour : COLOUR;
    float2 TexCoord : TEXCOORD;
};
    
float4 main(PSInput input) : SV_Target
{
    if (MaterialInfoCB.albedoTextureIndex != -1)
    {
        return Texture2DTable[MaterialInfoCB.albedoTextureIndex].Sample(DefaultSampler, input.TexCoord);
    }
    else
    {
        return input.Colour;
    }
}