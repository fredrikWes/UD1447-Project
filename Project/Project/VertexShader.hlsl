struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

cbuffer MATRICES : register(b0)
{
    float4x4 world;
    float4x4 viewPerspective;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.position = mul(mul(float4(input.position, 1.0f), world), viewPerspective);
    output.normal = normalize(mul(float4(input.normal, 0.0f), world));
    output.normal = float3(output.normal.z, output.normal.y, output.normal.x);
    output.uvs = input.uvs;
    
    return output;
}