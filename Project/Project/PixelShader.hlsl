Texture2D textureColor : register(t0);
SamplerState samp : register(s0);

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

float4 main(PS_INPUT input) : SV_TARGET
{
    const float ambient = 1.2f;
    const float3 colorSample = textureColor.Sample(samp, input.uvs).xyz;
    return float4(max(0, dot(-lightDirection, input.normal)) * colorSample * ambient, 1.0f);
}