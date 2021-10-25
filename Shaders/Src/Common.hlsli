#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

float3 ComputeBinormal(float3 normal, float3 tangent, float sign)
{
    float3 binormal = cross(normalize(normal), normalize(tangent));
    binormal *= sign;
    
    return binormal;
}
#endif