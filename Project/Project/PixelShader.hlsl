struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(input.normal, 1.0f);
}