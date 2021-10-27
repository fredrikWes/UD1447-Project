Texture2D textureColor : register(t0);
SamplerState samp : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return textureColor.Sample(samp, input.uvs);
}