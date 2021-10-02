
struct VSInput
{
    float2 Pos : POSITION;
    float4 colour : COLOUR;
};
struct VsOutput
{
    float4 Colour : COLOUR;
    float4 Position : SV_POSITION;
};

VsOutput main(VSInput input)
{
    VsOutput output;
 
    output.Position = float4(input.Pos, 0.0f, 1.0f);
    output.Colour = input.colour;
 
    return output;
}