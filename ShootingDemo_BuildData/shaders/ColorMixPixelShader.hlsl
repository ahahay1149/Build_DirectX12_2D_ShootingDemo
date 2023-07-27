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


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    color.xyz = color.xyz * input.Col.xyz;  //RGB‚ğŠ|‚¯Z
    color.w *= input.Col.w;                 //A‚ÍŠ|‚¯Z

    //Alpha Test ‚±‚ê‚ğ‚â‚ç‚È‚¢‚Æ“§–¾F‚àDepth‚ğ‘‚«‚ñ‚Å‚µ‚Ü‚¤B
    if (color.w < 0.1f)
        discard;

    //HLSL‚Ìswitch•¶‚Í”»’è®‚ª‚OˆÈã‚Å‚ ‚é–‚ª•ÛØ‚³‚ê‚Ä‚¢‚È‚¢‚Æƒ_ƒ‚È‚Ì‚Å’ˆÓ
    switch (ColorMix)
    {
    case 0:  //Mul
        color.xyz *= MixColor.xyz;
        break;

    case 1: //add
        color.xyz += MixColor.xyz;
        break;

    case 2: //dec
        color.xyz -= MixColor.xyz;
        break;

    case 3: //set
        color.xyz = MixColor.xyz;
        break;
    }

    switch (AlphaMix)
    {
    case 0:  //Mul
        color.w *= MixColor.w;
        break;

    case 1: //add
        color.w += MixColor.w;
        break;

    case 2: //dec
        color.w -= MixColor.w;
        break;

    case 3: //set
        color.w = MixColor.w;
        break;
    }

    return color;
}
