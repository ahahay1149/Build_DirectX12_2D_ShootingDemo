#include "Header.hlsli"

cbuffer cbMixColor : register(b0)
{
    float4 MixColor;
}

cbuffer cbMixMode : register(b1)
{
    uint ColorMix;
    uint AlphaMix;
}


float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    color.xyz = color.xyz * input.Col.xyz;
    color.w *= input.Col.w;

    if (color.w < 0.1f)
        discard;

    switch (ColorMix)
    {
    case 0:  
        color.xyz *= MixColor.xyz;
        break;

    case 1: 
        color.xyz += MixColor.xyz;
        break;

    case 2: 
        color.xyz -= MixColor.xyz;
        break;

    case 3: 
        color.xyz = MixColor.xyz;
        break;
    }

    switch (AlphaMix)
    {
    case 0:  
        color.w *= MixColor.w;
        break;

    case 1: 
        color.w += MixColor.w;
        break;

    case 2: 
        color.w -= MixColor.w;
        break;

    case 3: 
        color.w = MixColor.w;
        break;
    }

    return color;
}
