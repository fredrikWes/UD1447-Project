struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

cbuffer Light : register(b0)
{
    float3 lightDirection;
}

cbuffer Color : register(b1)
{
    float3 color;
}

float4 main(PS_INPUT input) : SV_TARGET
{ 
    const float ambient = 1.2f;
    return float4(max(0, dot(-lightDirection, input.normal)) * color * ambient, 1.0f);
}