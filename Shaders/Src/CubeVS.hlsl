
struct DrawInfo
{
    matrix modelViewProjectMatrix;
};

ConstantBuffer<DrawInfo> DrawInfoCB : register(b0);

struct VSInput
{
    float3 Pos      : POSITION;
    float3 colour   : COLOUR;
};

struct VsOutput
{
    float4 Colour : COLOUR;
    float4 Position : SV_POSITION;
};

VsOutput main(VSInput input)
{
    VsOutput output;
 
    output.Position = mul(DrawInfoCB.modelViewProjectMatrix, float4(input.Pos, 1.0f));
    output.Colour = float4(input.colour, 1.0f);
 
    return output;
}