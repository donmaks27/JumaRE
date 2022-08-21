#define TEXTURE_BLOCK_SIZE 8

#define WIDTH_HEIGHT_EVEN 0
#define WIDTH_ODD_HEIGHT_EVEN 1
#define WIDTH_EVEN_HEIGHT_ODD 2
#define WIDTH_HEIGHT_ODD 3

struct ComputeShaderInput
{
    uint3 GroupID           : SV_GroupID;
    uint3 GroupThreadID     : SV_GroupThreadID;
    uint3 DispatchThreadID  : SV_DispatchThreadID;
    uint  GroupIndex        : SV_GroupIndex;
};

cbuffer GenerateMipsBuffer : register(b0)
{
    float2 uTexelSize;
    uint uMipCount;
    uint uSrcMipIndex;
    uint uSrcMipSizeType; // Width and height of the source texture are even or odd.
    uint uIsSRGB;
}

Texture2D<float4> uTexture : register(t0);
SamplerState uSampler : register(s0);

RWTexture2D<float4> uTextureOutput1 : register(u0);
RWTexture2D<float4> uTextureOutput2 : register(u1);
RWTexture2D<float4> uTextureOutput3 : register(u2);
RWTexture2D<float4> uTextureOutput4 : register(u3);

groupshared float sgCachedValueR[64];
groupshared float sgCachedValueG[64];
groupshared float sgCachedValueB[64];
groupshared float sgCachedValueA[64];
void CacheColor(const uint groupIndex, const float4 color)
{
    sgCachedValueR[groupIndex] = color.r;
    sgCachedValueG[groupIndex] = color.g;
    sgCachedValueB[groupIndex] = color.b;
    sgCachedValueA[groupIndex] = color.a;
}
float4 GetCachedColor(const uint groupIndex)
{
    return float4(sgCachedValueR[groupIndex], sgCachedValueG[groupIndex], sgCachedValueB[groupIndex], sgCachedValueA[groupIndex]);
}

float3 LinearColorToSRGB(const float3 color)
{
    //return color < 0.0031308 ? 12.92 * color : 1.055 * pow(abs(color), 1.0 / 2.4) - 0.055; // Correctly
    return color < 0.0031308 ? 12.92 * color : 1.13005 * sqrt(abs(color - 0.00228)) - 0.13448 * color + 0.005719; // Cheaper
}
float4 MakeResultColor(float4 color)
{
    return uIsSRGB != 0 ? float4(LinearColorToSRGB(color.rgb), color.a) : color;
}

[numthreads(TEXTURE_BLOCK_SIZE, TEXTURE_BLOCK_SIZE, 1)]
void main(const ComputeShaderInput inputData)
{
    float4 sampledPixel1 = float4(0.0, 0.0, 0.0, 0.0), sampledPixel2, sampledPixel3, sampledPixel4;
    switch (uSrcMipSizeType)
    {
    case WIDTH_HEIGHT_EVEN:
        {
            const float2 textureCoords = uTexelSize * (inputData.DispatchThreadID.xy + 0.5);
            sampledPixel1 = uTexture.SampleLevel(uSampler, textureCoords, uSrcMipIndex);
        }
        break;
    case WIDTH_ODD_HEIGHT_EVEN:
        {
            const float2 textureCoords1 = uTexelSize * (inputData.DispatchThreadID.xy + float2(0.25, 0.5));
            const float2 textureCoords2 = textureCoords1 + uTexelSize * float2(0.5, 0.0);
            sampledPixel1 = (uTexture.SampleLevel(uSampler, textureCoords1, uSrcMipIndex) + 
                            uTexture.SampleLevel(uSampler, textureCoords2, uSrcMipIndex)) * 0.5;
        }
        break;
    case WIDTH_EVEN_HEIGHT_ODD:
        {
            const float2 textureCoords1 = uTexelSize * (inputData.DispatchThreadID.xy + float2(0.5, 0.25));
            const float2 textureCoords2 = textureCoords1 + uTexelSize * float2(0.0, 0.5);
            sampledPixel1 = (uTexture.SampleLevel(uSampler, textureCoords1, uSrcMipIndex) + 
                            uTexture.SampleLevel(uSampler, textureCoords2, uSrcMipIndex)) * 0.5;
        }
        break;
    case WIDTH_HEIGHT_ODD:
        {
            const float2 textureCoords1 = uTexelSize * (inputData.DispatchThreadID.xy + float2(0.25, 0.25));
            const float2 textureCoords2 = textureCoords1 + uTexelSize * float2(0.5, 0.0);
            const float2 textureCoords3 = textureCoords1 + uTexelSize * float2(0.0, 0.5);
            const float2 textureCoords4 = textureCoords1 + uTexelSize * float2(0.5, 0.5);
            sampledPixel1 = (uTexture.SampleLevel(uSampler, textureCoords1, uSrcMipIndex) + 
                            uTexture.SampleLevel(uSampler, textureCoords2, uSrcMipIndex) + 
                            uTexture.SampleLevel(uSampler, textureCoords3, uSrcMipIndex) + 
                            uTexture.SampleLevel(uSampler, textureCoords4, uSrcMipIndex)) * 0.25;
        }
        break;
    default: return;
    }

    uTextureOutput1[inputData.DispatchThreadID.xy] = MakeResultColor(sampledPixel1);
    if (uMipCount <= 1)
    {
        return;
    }
    CacheColor(inputData.GroupIndex, sampledPixel1);
    GroupMemoryBarrierWithGroupSync();

    if ((inputData.GroupIndex & 0x9) == 0)
    {
        sampledPixel2 = GetCachedColor(inputData.GroupIndex + 1);
        sampledPixel3 = GetCachedColor(inputData.GroupIndex + 8);
        sampledPixel4 = GetCachedColor(inputData.GroupIndex + 9);
        sampledPixel1 = (sampledPixel1 + sampledPixel2 + sampledPixel3 + sampledPixel4) * 0.25;
        uTextureOutput2[inputData.DispatchThreadID.xy / 2] = MakeResultColor(sampledPixel1);
        CacheColor(inputData.GroupIndex, sampledPixel1);
    }
    if (uMipCount == 2)
    {
        return;
    }
    GroupMemoryBarrierWithGroupSync();

    if ((inputData.GroupIndex & 0x1B) == 0)
    {
        sampledPixel2 = GetCachedColor(inputData.GroupIndex + 2);
        sampledPixel3 = GetCachedColor(inputData.GroupIndex + 16);
        sampledPixel4 = GetCachedColor(inputData.GroupIndex + 18);
        sampledPixel1 = (sampledPixel1 + sampledPixel2 + sampledPixel3 + sampledPixel4) * 0.25;
        uTextureOutput3[inputData.DispatchThreadID.xy / 4] = MakeResultColor(sampledPixel1);
        CacheColor(inputData.GroupIndex, sampledPixel1);
    }
    if (uMipCount == 3)
    {
        return;
    }
    GroupMemoryBarrierWithGroupSync();

    if (inputData.GroupIndex == 0)
    {
        sampledPixel2 = GetCachedColor(inputData.GroupIndex + 4);
        sampledPixel3 = GetCachedColor(inputData.GroupIndex + 32);
        sampledPixel4 = GetCachedColor(inputData.GroupIndex + 36);
        sampledPixel1 = (sampledPixel1 + sampledPixel2 + sampledPixel3 + sampledPixel4) * 0.25;
        uTextureOutput4[inputData.DispatchThreadID.xy / 8] = MakeResultColor(sampledPixel1);
    }
}