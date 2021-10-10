#ifndef __BINDLESS_EXAMPLE_HLSLI__
#define __BINDLESS_EXAMPLE_HLSLI__


#define Tex2DSpace space100
#define BufferSpace space101


struct GeometryData
{
    uint IndexBufferIndex;
    uint IndexOffset;
    uint VertexBufferIndex;
    uint VertexOffset;
    uint AlbedoTextureIndex;
};
    
ConstantBuffer<GeometryData> GeometryDataCB : register(b0);

#endif