void PixelShaderPolygonGlass(
    in float4 inPosition : SV_POSITION,
    in float2 inTexCoord : TEXCOORD0,
    in float4 inRefractionPosition : TEXCOORD1,
    out float4 outColor : SV_Target

)
//-----------------------------------------------------------------------------
// Globals
SamplerState SampleType : register(s0);

Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D refractionTexture : register(t2);

cbuffer GlassBuffer
{
    float refractionScale;
    float3 padding;
}

//-----------------------------------------------------------------------------
// Pixel Shader (C-style explicit input/output)
void PixelShaderPolygonGlass(
    in float4 inPosition : SV_POSITION,
    in float2 inTexCoord : TEXCOORD0,
    in float4 inRefractionPosition : TEXCOORD1,

    out float4 outColor : SV_Target
)
{
    float2 refractTexCoord;
    float4 normalMap;
    float3 normal;
    float4 refractionColor;
    float4 textureColor;

    // Calculate projected refraction texture coordinates
    refractTexCoord.x = inRefractionPosition.x / inRefractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -inRefractionPosition.y / inRefractionPosition.w / 2.0f + 0.5f;

    // Sample the normal from the normal map texture
    normalMap = normalTexture.Sample(SampleType, inTexCoord);

    // Expand normal from (0,1) to (-1,+1)
    normal = normalMap.xyz * 2.0f - 1.0f;

    // Offset the sampling position by the normal for refraction
    refractTexCoord = refractTexCoord + (normal.xy * refractionScale);

    // Sample the refraction and color textures
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);
    textureColor = colorTexture.Sample(SampleType, inTexCoord);

    // Combine for final output
    outColor = lerp(refractionColor, textureColor, 0.5f);
}
