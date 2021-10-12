#include "BindlessExample.hlsli"

// Linear clamp sampler.
SamplerState LinearClampSampler : register(s0);
Texture2D Texture2DTable[] : register(t0, Tex2DSpace);

// -- Const buffers ---
struct DrawInfo
{
    matrix mvp;
    uint textureIndex;
    float3 _padding;
};
    
ConstantBuffer<DrawInfo> DrawInfoCB : register(b0);

struct PSInput
{
    float4 Colour : COLOR;
    float2 TexCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_Target
{
    float4 outColour = input.Colour * Texture2DTable[DrawInfoCB.textureIndex].SampleLevel(LinearClampSampler, input.TexCoord, 0);
    return outColour;
}
