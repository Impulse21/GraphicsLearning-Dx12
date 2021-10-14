
struct DrawInfo
{
    matrix WorldMatrix;
    matrix ModelViewProjectMatrix;
    float3 CameraPosition;
};

ConstantBuffer<DrawInfo> DrawInfoCB : register(b0);

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
    float3 WorldPosition : Position;
    float4 Position : SV_POSITION;
};

VsOutput main(VSInput input)
{
    VsOutput output;
 
    float4 modelPosition = float4(input.Pos, 1.0f);
    output.WorldPosition = mul(DrawInfoCB.WorldMatrix, modelPosition);
    output.Position = mul(DrawInfoCB.ModelViewProjectMatrix, modelPosition);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    output.Colour = float4(input.Colour, 1.0f);
    
    return output;
}