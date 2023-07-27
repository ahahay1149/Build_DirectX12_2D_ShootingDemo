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

//==========UV Anime
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
//==========UV Anime End

//--------------------------------------------------------------------------------------
// Sprite Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(SPRITE_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;

    output.Pos = input.Pos;

    //====ToDo 1: UV反映
    //ここが重要に
    output.Tex.x = SpriteRect.x + input.Tex.x * SpriteRect.z;
    output.Tex.y = SpriteRect.y + input.Tex.y * SpriteRect.w;
    //====ToDo 1:ここまで

    //====ToDo 2: サイズ反映
    //スプライトのサイズ変更 MVPの前に。
    output.Pos.x = input.Pos.x * SpriteSize.x;
    output.Pos.y = input.Pos.y * SpriteSize.y;
    //====ToDo 2:ここまで

    //MVP Matrix
    output.Pos = mul(output.Pos, Model);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    //====ToDo 3: 頂点カラーはとりあえず白
    output.Col = float4(1.0f, 1.0f, 1.0f, 1.0f);    //とりあえず真っ白
    //====ToDo 3:ここまで


    return output;
}