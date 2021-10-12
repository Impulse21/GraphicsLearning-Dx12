
struct DrawInfo
{
    matrix mvp;
    uint textureIndex;
    float3 _padding;
};
    
ConstantBuffer<DrawInfo> DrawInfoCB : register(b0);

struct VSInput
{
    float3 Position : POSITION;
    float3 Colour : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct VSOutput
{
    float4 Colour : COLOR;
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_Position;
};


VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = mul(DrawInfoCB.mvp, float4(input.Position.xy, 0.f, 1.f));
    output.Colour = float4(input.Colour, 1.0f);
    output.TexCoord = input.TexCoord;
    
    return output;
}