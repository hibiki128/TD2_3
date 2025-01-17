#include"FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    float value = dot(output.color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
    
    // セピア調の色を薄くするために調整
    float3 sepia = value * float3(1.0f, 74.0f / 107.0f, 43.0f / 107.0f);
    
    // 薄くするために色をさらに薄くする (0.5f は調整値)
    output.color.rgb = lerp(sepia, output.color.rgb, 0.7f);

    return output;
}
