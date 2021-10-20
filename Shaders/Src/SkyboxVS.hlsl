
struct Skybox
{
    matrix ViewProjection;
    uint SkyboxTexIndex;
};
    
ConstantBuffer<Skybox> SkyboxCB : register(b0);


struct VSOutput
{
    float3 TexCoord : TEXCOORD;
    float4 Position : SV_Position;
};


VSOutput main(in float3 position : POSITION)
{
    VSOutput output;
    output.Position = mul(float4(position, 0.0f), SkyboxCB.ViewProjection).xyww;
    output.TexCoord = position;
    
    return output;
}