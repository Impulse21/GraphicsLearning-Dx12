#include "BindlessExample.hlsli"

struct DrawInfo
{
    matrix modelViewProjectMatrix;
};

ConstantBuffer<DrawInfo> DrawInfoCB : register(b1);
ByteAddressBuffer BufferTable[] : register(t0, BufferSpace);

struct Vertex
{
    float3 Pos      : POSITION;
    float3 Colour   : COLOUR;
    float2 TexCoord : TEXCOORD;

};

struct VsOutput
{
    float4 Colour : COLOUR;
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_POSITION;
};

VsOutput main(in uint vertexID : SV_VertexID)
{
    VsOutput output;
    
    ByteAddressBuffer vertexBuffer = BufferTable[GeometryDataCB.VertexBufferIndex];
    Vertex vert = BufferTable[GeometryDataCB.VertexBufferIndex].Load<Vertex>((GeometryDataCB.VertexOffset + vertexID) * sizeof(Vertex));
    
    output.Position = mul(DrawInfoCB.modelViewProjectMatrix, float4(vert.Pos, 1.0f));
    output.TexCoord = vert.TexCoord;
    output.Colour = float4(vert.Colour, 1.0f);
    
    return output;
}