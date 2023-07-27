#include "Header.hlsli"

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    color.xyz = color.xyz * input.Col.xyz;
    color.w *= input.Col.w;
    return color;
}
