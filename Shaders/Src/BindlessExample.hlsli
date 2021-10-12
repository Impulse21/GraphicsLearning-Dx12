
#include "Defines.hlsli"

#ifndef __BINDLESS_EXAMPLE_HLSLI__
#define __BINDLESS_EXAMPLE_HLSLI__



struct GeometryData
{
    uint IndexOffset;
    uint VertexBufferIndex;
    uint VertexOffset;
    uint AlbedoTextureIndex;
};
    
ConstantBuffer<GeometryData> GeometryDataCB : register(b0);

#endif