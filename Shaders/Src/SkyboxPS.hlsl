#include "Defines.hlsli"

// Linear clamp sampler.
SamplerState LinearClampSampler : register(s0);
TextureCube TextureCubeTable[] : register(t0, TexCubeSpace);

// -- Const buffers ---

struct Skybox
{
    matrix ViewProjection;
    uint SkyboxTexIndex;
};

// TODO: Could optimize by providing the tex index as an input
ConstantBuffer<Skybox> SkyboxCB : register(b0);

struct PSInput
{
    float3 TexCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_Target
{
    float3 envColour = TextureCubeTable[SkyboxCB.SkyboxTexIndex].Sample(LinearClampSampler, input.TexCoord).rgb;
    // HDR tonemap and gamma correct
    envColour = envColour / (envColour + float3(1.0, 1.0, 1.0));
    envColour = pow(envColour, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(envColour, 1.0f);
}
