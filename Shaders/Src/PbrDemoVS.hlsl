

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
    uint DrawFlags;
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
};

struct VsOutput
{
    float3 NormalWS     : NORMAL;
    float4 Colour       : COLOUR;
    float2 TexCoord     : TEXCOORD;
    float3 PositionWS : Position;
    float3 TangentWS : TANGENT;
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
    
    output.TangentWS = mul(input.Tangent.xyz, (float3x3) worldMatrix);
    
    return output;
}