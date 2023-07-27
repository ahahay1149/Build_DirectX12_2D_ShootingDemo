#include "Header.hlsli"

cbuffer cbViewMatrix : register(b0)
{
    matrix View;
};

cbuffer cbProjectionMatrix : register(b1)
{
    matrix Projection;
};

cbuffer cbModelMatrix : register(b2)
{
    matrix Model;
};

cbuffer cbSprite : register(b3)
{
    float4 SpriteRect;
}

cbuffer cbSpSize : register(b4)
{
    float2 SpriteSize;
}

struct SPRITE_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

PS_INPUT main(SPRITE_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;

    output.Pos = input.Pos;

    output.Tex.x = SpriteRect.x + input.Tex.x * SpriteRect.z;
    output.Tex.y = SpriteRect.y + input.Tex.y * SpriteRect.w;
    output.Pos.x = input.Pos.x * SpriteSize.x;
    output.Pos.y = input.Pos.y * SpriteSize.y;
    output.Pos = mul(output.Pos, Model);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    output.Col = float4(1.0f, 1.0f, 1.0f, 1.0f);    

    return output;
}