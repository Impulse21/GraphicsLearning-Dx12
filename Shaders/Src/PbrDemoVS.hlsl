
struct DrawInfo
{
    matrix modelViewProjectMatrix;
};

ConstantBuffer<DrawInfo> DrawInfoCB : register(b1);

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Colour   : COLOUR;
    float2 TexCoord : TEXCOORD;
};

struct VsOutput
{
    float3 Normal : NORMAL;
    float4 Colour : COLOUR;
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_POSITION;
};

VsOutput main(VSInput input)
{
    VsOutput output;
 
    output.Position = mul(DrawInfoCB.modelViewProjectMatrix, float4(input.Pos, 1.0f));
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    output.Colour = float4(input.Colour, 1.0f);
    
    return output;
}