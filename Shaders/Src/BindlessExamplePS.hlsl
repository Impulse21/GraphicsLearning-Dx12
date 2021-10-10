#include "BindlessExample.hlsli"


Texture2D Texture2DTable[] : register(t0, Tex2DSpace);

SamplerState DefaultSampler : register(s0);

// -- Const buffers ---
struct PSInput
{
    float4 Colour : COLOUR;
    float2 TexCoord : TEXCOORD;
};
    
float4 main(PSInput input) : SV_Target
{
    if (GeometryDataCB.AlbedoTextureIndex != -1)
    {
        return Texture2DTable[GeometryDataCB.AlbedoTextureIndex].Sample(DefaultSampler, input.TexCoord);
    }
    else
    {
        return input.Colour;
    }
}