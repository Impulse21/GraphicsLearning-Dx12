
#define INSTANCED_FLAG 0x01

struct DrawInfo
{
    uint InstanceIndex;
    
    float3 Albedo;
    float Metallic;
    float Roughness;
    float Ao;

    uint AlbedoTexIndex;
    uint NormalTexIndex;
    uint MetallicTexIndex;
    uint RoughnessTexIndex;
    uint AoTexIndex;
};

ConstantBuffer<DrawInfo> DrawInfoCB : register(b0);

struct SceneInfo
{
    matrix ViewProjection;
    float3 CameraPosition;
    float3 SunDirection;
    float3 SunColour;
    uint IrradianceMapTexIndex;
    uint PreFilteredEnvMapTexIndex;
    uint BrdfLUTTexIndex;
};

ConstantBuffer<SceneInfo> SceneInfoCB : register(b1);

struct InstanceInfo
{
    matrix Transform;
};

StructuredBuffer<InstanceInfo> InstanceSB : register(t0);

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Colour   : COLOUR;
    float2 TexCoord : TEXCOORD;
    float4 Tangent : TANGENT;
    float4 BiTangent : BITANGENT;
};

struct VsOutput
{
    float3 NormalWS     : NORMAL;
    float4 Colour       : COLOUR;
    float2 TexCoord     : TEXCOORD;
    float3 PositionWS   : Position;
    float3x3 TBN          : TBN;
    float4 Position : SV_POSITION;
};

VsOutput main(VSInput input)
{
    VsOutput output;
    
    matrix worldMatrix = InstanceSB[DrawInfoCB.InstanceIndex].Transform;
    
    float4 modelPosition = float4(input.Pos, 1.0f);
    output.PositionWS = mul(modelPosition, worldMatrix).xyz;
    output.Position = mul(float4(output.PositionWS, 1.0f), SceneInfoCB.ViewProjection);
    output.NormalWS = mul(input.Normal, (float3x3)worldMatrix);
    output.TexCoord = input.TexCoord;
    output.Colour = float4(input.Colour, 1.0f);
    
    float3 TangentWS = mul(input.Tangent.xyz, (float3x3) worldMatrix);
    float3 BiTangentWS = mul(input.BiTangent.xyz, (float3x3) worldMatrix);
    
    // Calculate TBN Matrix
    output.TBN = float3x3(normalize(TangentWS), normalize(BiTangentWS), normalize(output.NormalWS));
    return output;
}