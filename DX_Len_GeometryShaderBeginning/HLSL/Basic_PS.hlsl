#include "Basic.hlsli"

// 像素着色器(3D)
float4 PS(VertexPosHWNormalTex pIn) : SV_Target
{ 
    // 若不使用纹理，则使用默认白色
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float2 projectTexCoord = float2(0.0f, 0.0f);
    float lightDepthValue = 1.0f;

    if (gTextureUsed)
    {
        texColor = gDiffuseMap.Sample(gSamWrap, pIn.Tex);
        // 提前进行裁剪，对不符合要求的像素可以避免后续运算
        clip(texColor.a - 0.1f);
    }

    // 阴影部分
    [flatten]
    if(gShadowEnabled)
    {
        projectTexCoord.x = (pIn.lightViewPositionH.x / pIn.lightViewPositionH.w) / 2.0f + 0.5f;
        projectTexCoord.y = (-pIn.lightViewPositionH.y / pIn.lightViewPositionH.w) / 2.0f + 0.5f;

        if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
        {
            float depth = gShadowMap.Sample(gSamPonitBorder, float2(projectTexCoord)).r;

            lightDepthValue = pIn.lightViewPositionH.z / pIn.lightViewPositionH.w;

            if ((lightDepthValue - gBias) > depth)
                {
                    texColor.rgb *= 0.24;   
                    return texColor;
                }
        }
    }
    
    // 标准化法向量
    pIn.NormalW = normalize(pIn.NormalW);

    // 求出顶点指向眼睛的向量，以及顶点与眼睛的距离
    float3 toEyeW = normalize(gEyePosW - pIn.PosW);
    float distToEye = distance(gEyePosW, pIn.PosW);

    // 初始化为0 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int i;

    [unroll]
    for (i = 0; i < gNumDirLight; ++i)
    {
        ComputeDirectionalLight(gMaterial, gDirLight[i], pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
        
    [unroll]
    for (i = 0; i < gNumPointLight; ++i)
    {
        ComputePointLight(gMaterial, gPointLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }

    [unroll]
    for (i = 0; i < gNumSpotLight; ++i)
    {
        ComputeSpotLight(gMaterial, gSpotLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
  
    float4 litColor = texColor * (ambient + diffuse) + spec;
    
    // 雾效部分
    [flatten]
    if (gFogEnabled)
    {
        // 限定在0.0f到1.0f范围
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);
        // 根据雾色和光照颜色进行线性插值
        litColor = lerp(litColor, gFogColor, fogLerp);
    }

    litColor.a = texColor.a * gMaterial.Diffuse.a;
    return litColor;
}
