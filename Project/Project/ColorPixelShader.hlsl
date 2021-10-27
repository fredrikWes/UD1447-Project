Texture2D textureColor : register(t0);
SamplerState samp : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

cbuffer Color : register(b1)
{
    float3 color;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(color, 1.0f);
}