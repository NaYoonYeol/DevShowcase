
#include "Shader_Defines.hlsli"

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix			g_InvTranspose;
matrix          g_LightViewMatrix;
matrix          g_LightProjMatrix;

TextureCube     g_ReflectionCapture;

Texture2D g_OccTexture;
float2 g_vTextureSize;

float3 g_eyePos;
float3 g_eyeLook;

float g_fFar = 500.f;
Texture2D g_ShadowDepth;

// Data used in hull shader
float g_dynamicTesselationAmount = 1.0f;
float g_staticTesselationOffset = 0.0f;

struct VS_VERTEX_IN
{
    float3 vPosition;
    float3 vNormal;
    float2 vTexcoord;
};

struct VS_IN
{
	float3		vPosition : POSITION;
    float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
    
    float       fPatchLT : INST_PATCH_LT;
};

struct VS_OUTPUT
{
    float4 vWorldPosition : VS0;
    float3 vWorldNormal : VS1;
    float2 vTexcoord : VS2;
};

struct HS_CONSTANTOUTPUT
{
    float edgeTessFactors[3] : SV_TessFactor;
    float insideTessFactor : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
    float4 vWorldPosition : HSO;
    float3 vWorldNormal : HS1;
    float2 vTexcoord : HS2;
};

struct DS_OUTPUT
{
    float4 clipSpacePosition : SV_Position;
    float3 vWorldNormal : NORMAL;
    centroid float2 vTexcoord : TEXCOORD0;
    float3 worldspacePositionDisplaced : TEXCOORD2;
    float3 worldspacePositionUndisplaced : TEXCOORD3;
    
    float4 vProjPos : Position;
};

struct PS_IN
{
    float4 vProjPosition : SV_POSITION;
    float3 vWorldPosition : POSITION;
    float3 vWorldNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_IN_NORMAL
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEEXCOORD;
    uint iPrimID : SV_PrimitiveID;
};

struct GS_IN
{
    float4 vPosWorld : SV_POSITION;
    float3 vNormalWorld : NORMAL;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
    float4 vDepth : SV_TARGET1;
};

// Animate Surface Datas
// For Compute Shader
ByteAddressBuffer g_CurrentSurface : register(t0);
ByteAddressBuffer g_PreviousSurface : register(t1);
ByteAddressBuffer g_DampingCoefficient : register(t2);

// For Vertex Shader
StructuredBuffer<VS_VERTEX_IN> g_SurfaceVertexTable : register(t3);

RWByteAddressBuffer g_AnimatedSurface : register(u0);

// Calculate Vertices Datas
RWStructuredBuffer<VS_VERTEX_IN> g_SurfaceVertices : register(u1);
RWStructuredBuffer<float3> g_SurfacePosition : register(u2);

uint g_iSimulateBufferWidth;
uint g_iQuadTreeGridWidth;
uint g_iPixelsPerThread;

VS_OUTPUT VS_MAIN(VS_IN In)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0;
    
    uint iRow_World = In.fPatchLT / g_iQuadTreeGridWidth;
    uint iCol_World = In.fPatchLT % g_iQuadTreeGridWidth;
    int iRow = In.vTexcoord.x / 8;
    int iCol = In.vTexcoord.x % 8;
    uint iVertexTableIndex = (iRow_World + iRow) * g_iSimulateBufferWidth + (iCol_World + iCol);
    
    VS_VERTEX_IN vVertexPosition = g_SurfaceVertexTable[iVertexTableIndex];
    
    float4 vPosition = float4(vVertexPosition.vPosition, 1.0);
    vPosition = mul(vPosition, g_WorldMatrix);
	
    Out.vWorldPosition = vPosition;
	
    float4 normal = float4(vVertexPosition.vNormal, 0.0);
    Out.vWorldNormal = mul(normal, g_WorldMatrix).xyz;
    Out.vWorldNormal = normalize(Out.vWorldNormal);
    
    Out.vTexcoord = vVertexPosition.vTexcoord;

	return Out;
}

float GetEdgeTessellationFactor(float4 vertex1, float4 vertex2)
{
    float3 edgeCenter = 0.5 * (vertex1.xyz + vertex2.xyz);
    float edgeLength = length(vertex1.xyz - vertex2.xyz);
    float distanceToEdge = length(g_eyePos - edgeCenter);
    return g_staticTesselationOffset + g_dynamicTesselationAmount * edgeLength / distanceToEdge;
}

HS_CONSTANTOUTPUT HS_Constant(InputPatch<VS_OUTPUT, 3> I)
{
    HS_CONSTANTOUTPUT Output;
    Output.edgeTessFactors[0] = GetEdgeTessellationFactor(I[1].vWorldPosition, I[2].vWorldPosition);
    Output.edgeTessFactors[1] = GetEdgeTessellationFactor(I[2].vWorldPosition, I[0].vWorldPosition);
    Output.edgeTessFactors[2] = GetEdgeTessellationFactor(I[0].vWorldPosition, I[1].vWorldPosition);
    Output.insideTessFactor = (Output.edgeTessFactors[0] + Output.edgeTessFactors[1] + Output.edgeTessFactors[2]) / 3.0;
    return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HS_Constant")]
[outputcontrolpoints(3)]
HS_OUTPUT HS_MAIN(InputPatch<VS_OUTPUT, 3> I, uint uCPID : SV_OutputControlPointID)
{
    HS_OUTPUT Output;
    Output.vWorldPosition = float4(I[uCPID].vWorldPosition);
    Output.vWorldNormal = float3(I[uCPID].vWorldNormal);
    Output.vTexcoord = float2(I[uCPID].vTexcoord);
    return Output;
}

float g_localWavesSimulationDomainWorldspaceSize;
float2 g_localWavesSimulationDomainWorldspaceCenter;

Texture2D g_textureFlowingWater;
Texture2D g_FoamTexture;

float2 g_windWave;

DS_OUTPUT GetDisplacedVertexAfterTessellation(HS_OUTPUT In0, HS_OUTPUT In1, HS_OUTPUT In2, float3 BarycentricCoords)
{
    precise float3 tessellatedWorldspacePosition = In0.vWorldPosition.xyz * BarycentricCoords.x + In1.vWorldPosition.xyz * BarycentricCoords.y + In2.vWorldPosition.xyz * BarycentricCoords.z;
    float3 worldspacePositionUndisplaced = tessellatedWorldspacePosition;
	
    precise float3 tessellatedWorldspaceNormal = In0.vWorldNormal.xyz * BarycentricCoords.x + In1.vWorldNormal.xyz * BarycentricCoords.y + In2.vWorldNormal.xyz * BarycentricCoords.z;
    
    precise float2 tessellatedTexcoord = In0.vTexcoord.xy * BarycentricCoords.x + In1.vTexcoord.xy * BarycentricCoords.y + In2.vTexcoord.xy * BarycentricCoords.z;
    tessellatedTexcoord += g_windWave;
    
    float4 vFormIntensity = g_textureFlowingWater.SampleLevel(BilinearSampler, tessellatedTexcoord, 0.0);
    
    float3 vDisplacement = float3(0.0, (vFormIntensity.r), 0.0);
    float3 worldspacePositionDisplaced = worldspacePositionUndisplaced/* + vDisplacement*/;
	
	// Output
    DS_OUTPUT Output;
    Output.vTexcoord = tessellatedTexcoord;
    Output.vWorldNormal = normalize(tessellatedWorldspaceNormal);
    Output.worldspacePositionDisplaced = worldspacePositionDisplaced;
    Output.worldspacePositionUndisplaced = worldspacePositionUndisplaced;
	
    return Output;
}

[domain("tri")]
DS_OUTPUT DS_MAIN(HS_CONSTANTOUTPUT HSConstantData, const OutputPatch<HS_OUTPUT, 3> I, float3 vBarycentricCoords : SV_DomainLocation)
{
    DS_OUTPUT Output = GetDisplacedVertexAfterTessellation(I[0], I[1], I[2], vBarycentricCoords);

    matrix matViewProj = mul(g_ViewMatrix, g_ProjMatrix);
    Output.clipSpacePosition = mul(float4(Output.worldspacePositionDisplaced, 1.0), matViewProj);
    Output.vProjPos = mul(float4(Output.worldspacePositionDisplaced, 1.0), matViewProj);
    
    return Output;
}

float3 Fresnel(float3 fresnelR0, float3 normal, float3 toEye)
{
    float normalDotView = saturate(dot(normal, toEye));

    float f0 = 1.0f - normalDotView;

    return fresnelR0 + (1.0f - fresnelR0) * pow(f0, 5.0);
}

vector g_vLightDir = vector(1.f, -1.f, 1.f, 0.f);

PS_OUT PS_MAIN(DS_OUTPUT In)
{
    float3 g_WaterDeepColor = { 0.0, 0.2, 0.4 };
    float3 g_WaterScatterColor = { 0.0, 0.7, 0.6 };
    float4 g_WaterColorIntensity = { 0.02, 0.02, 0.01, 0.2 };
    float3 g_FoamColor = { 0.9, 0.9, 0.9 };
    float3 g_FoamUnderwaterColor = { 0.6, 0.6, 0.6 };
   
    float2 vUV;
    vUV.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
    vUV.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;
    
    uint2 pixelCoord = uint2(vUV * g_vTextureSize);
    vector vOccKeyColor = g_OccTexture.Sample(PointSampler, vUV);
    if (vOccKeyColor.x == 1.f)
        discard;
    
    float3 vToEye = normalize(g_eyePos - In.worldspacePositionUndisplaced);
    float3 R = reflect(-vToEye, In.vWorldNormal);
    
	PS_OUT		Out = (PS_OUT)0;
    
    vector vDiffuse = vector(g_WaterDeepColor, 1.f);
    
    vector vFoam = g_FoamTexture.Sample(LinearSampler, In.vTexcoord);
    
    vector vReflect = reflect(normalize(g_vLightDir), normalize(float4(R, 0)));
    vector vLook = vector(g_eyeLook, 0);
    vector vUnderLayer = vector(0.0, 0.15, 0.35, 1.f) * pow(saturate(dot(normalize(vLook), normalize(float4(R, 0)))), 30.0f);
    
    //vector vFoamColor = ((vector(g_FoamColor, 1) * vFoam.g) + (vector(g_FoamUnderwaterColor, 1) * vFoam.b)) * vFoam.a;
    vector vFoamColor = (vector(g_FoamUnderwaterColor, 1) * vFoam.g);
    vFoamColor *=  pow(saturate(dot(float4(0, 1, 0, 0), normalize(float4(R, 0)))), 1.0f);

    vector vSpecular = g_ReflectionCapture.Sample(LinearSampler, R);
    vSpecular *= pow((vSpecular.r + vSpecular.g + vSpecular.b) / 3.0, 2.f);
    
    float fresnel = Fresnel(float3(0.02f, 0.02f, 0.02f), R, vToEye);
    vSpecular *= fresnel;
    
    Out.vColor = vDiffuse + vUnderLayer + vFoamColor + vSpecular;
    Out.vColor.a = 1.f;
    
    // Shadow Mapping
    vector vWorldPosition = float4(In.worldspacePositionUndisplaced, 1.f);
    vector vLightSpace = mul(vWorldPosition, g_LightViewMatrix);
    vector vUVPos = mul(vLightSpace, g_LightProjMatrix);
    
    float2 vNewUV;
    vNewUV.x = (vUVPos.x / vUVPos.w) * 0.5f + 0.5f;
    vNewUV.y = (vUVPos.y / vUVPos.w) * -0.5f + 0.5f;
    vNewUV.x = clamp(vNewUV.x, 0.0, 1.0);
    vNewUV.y = clamp(vNewUV.y, 0.0, 1.0);
    
    vector vShadowDepthInfo = g_ShadowDepth.Sample(LinearSampler, vNewUV);
    float2 vShadowDepthTextureSize = g_vTextureSize * 8.f;
    float fShadowFactor = SampleShadowFactorFromShaderMap(g_ShadowDepth, vShadowDepthTextureSize, vNewUV, 3);
    
    Out.vColor *= (fShadowFactor > 0.5f) && (vLightSpace.z - 0.1f > vShadowDepthInfo.r * g_fFar) ?
        pow(vector(1.5 - fShadowFactor, 1.5 - fShadowFactor, 1.5 - fShadowFactor, 1.f), 2) : vector(1.f, 1.f, 1.f, 1.f);

	return Out;
}

//=========================================================================================
// Draw Normals

GS_IN VS_NORMAL_MAIN(VS_IN In)
{
    GS_IN Out = (GS_IN) 0;
    
    uint iRow_World = In.fPatchLT / g_iQuadTreeGridWidth;
    uint iCol_World = In.fPatchLT % g_iQuadTreeGridWidth;
    int iRow = In.vTexcoord.x / 8;
    int iCol = In.vTexcoord.x % 8;
    uint iVertexTableIndex = (iRow_World + iRow) * g_iSimulateBufferWidth + (iCol_World + iCol);
    
    VS_VERTEX_IN vVertexPosition = g_SurfaceVertexTable[iVertexTableIndex];

    float4 pos = float4(vVertexPosition.vPosition, 1.0f);
    Out.vPosWorld = mul(pos, g_WorldMatrix);

    float4 normal = float4(vVertexPosition.vNormal, 0.0f);
    Out.vNormalWorld = normalize(mul(normal, g_WorldMatrix));

    return Out;
}

[maxvertexcount(2)]
void GS_NORMAL_MAIN(point GS_IN input[1], uint primID : SV_PrimitiveID,
                              inout LineStream<PS_IN_NORMAL> outputStream)
{
    PS_IN_NORMAL output;
    
    output.vPosition = input[0].vPosWorld;
    output.vPosition = mul(output.vPosition, g_ViewMatrix);
    output.vPosition = mul(output.vPosition, g_ProjMatrix);
    output.vTexcoord = float2(0.0, 0.0);
    output.iPrimID = primID;
    
    outputStream.Append(output);

    output.vPosition = input[0].vPosWorld + float4(input[0].vNormalWorld, 0.0) * 1;
    output.vPosition = mul(output.vPosition, g_ViewMatrix);
    output.vPosition = mul(output.vPosition, g_ProjMatrix);
    output.vTexcoord = float2(0.0, 1.0);
    output.iPrimID = primID;
    
    outputStream.Append(output);
}

PS_OUT PS_NORMAL_MAIN(PS_IN_NORMAL In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float t = In.vTexcoord.y;
    Out.vColor = float4(float3(1.0, 1.0, 0.0) * (1.0 - t) + float3(1.0, 0.0, 0.0) * t, 1.0f);
    
    return Out;
}

//=========================================================================================
// Compute Shader

struct CS_IN
{
    uint3 iDispatchThreadID : SV_DispatchThreadID;
};

float g_fGridCellWidth;

// 시간 경과에 따른 파동의 전파를 계산하기 위해 2D 파동 방정식에서 사용되는 계수.
float g_fWavePropagationCoefficient;
// 파동 방정식의 수치 근사에서 중심 셀에 가중치를 적용하는 데 사용되는 요소.
// 다음 시간 스텝에 대한 현재 셀의 기여도에 영향을 미침.
float g_fCenterWeightingFactor;

float g_TempDampingCoefficient = 1.f;

[numthreads(29, 29, 1)]
void CS_WATER_ANIMATION(CS_IN In)
{
    uint3 iDispatchThreadID = asuint(In.iDispatchThreadID);
    
    uint threadIndexX = iDispatchThreadID.x;
    uint threadIndexY = iDispatchThreadID.y;
    
    uint globalIndex = threadIndexX + threadIndexY * g_iSimulateBufferWidth;

    uint startX = threadIndexX * g_iPixelsPerThread;
    uint startY = threadIndexY * g_iPixelsPerThread;
    
    // 격자의 가장자리 부분을 제외한 나머지 부분에서 물결의 상태를 계산
    if (threadIndexX == 0 || threadIndexY == 0 || threadIndexX == (g_iSimulateBufferWidth - 1) || threadIndexY == (g_iSimulateBufferWidth - 1))
    {
        return;
    }
    
    // (i-1, j), (i+1, j), (i, j-1), (i, j+1) 4 방향의 상태를 이용하여 다음 상태 계산
    //wavePropagationCoefficient * (m_vecPrevSurfaceHeight[(i - 1) * m_iGridSize + j]
    float fSurfaceHeightA = asfloat(g_PreviousSurface.Load(((threadIndexX - 1) * 4) + (threadIndexY * g_iSimulateBufferWidth * 4)));
    // m_vecPrevSurfaceHeight[(i + 1) * m_iGridSize + j]
    float fSurfaceHeightB = asfloat(g_PreviousSurface.Load(((threadIndexX + 1) * 4) + (threadIndexY * g_iSimulateBufferWidth * 4)));
    // m_vecPrevSurfaceHeight[i * m_iGridSize + (j - 1)]
    float fSurfaceHeightC = asfloat(g_PreviousSurface.Load(((threadIndexX) * 4) + ((threadIndexY - 1) * g_iSimulateBufferWidth * 4)));
    // m_vecPrevSurfaceHeight[i * m_iGridSize + (j + 1)]
    float fSurfaceHeightD = asfloat(g_PreviousSurface.Load(((threadIndexX) * 4) + ((threadIndexY + 1) * g_iSimulateBufferWidth * 4)));
            
    // m_vecPrevSurfaceHeight[i * m_iGridSize + j]
    float fPrevSurfaceHeight = asfloat(g_PreviousSurface.Load(((threadIndexX) * 4) + ((threadIndexY) * g_iSimulateBufferWidth * 4)));
    // m_vecCurrentSurfaceHeight[i * m_iGridSize + j]
    float fCurrentSurfaceHeight = asfloat(g_CurrentSurface.Load(((threadIndexX) * 4) + ((threadIndexY) * g_iSimulateBufferWidth * 4)));
            
    float fSurfaceHeight = (fSurfaceHeightA + fSurfaceHeightB + fSurfaceHeightC + fSurfaceHeightD) * g_fWavePropagationCoefficient
            + g_fCenterWeightingFactor * fPrevSurfaceHeight - fCurrentSurfaceHeight;
            
    // 계산된 물결 상태값 m_ppCurrentSurfaceHeight에 각 포인트의 댐핑 계수를 적용하여 
	// 최종 물결 상태값을 계산
    fSurfaceHeight *= g_TempDampingCoefficient;
            
    uint iOutAddress = globalIndex * 4;
    g_AnimatedSurface.Store(iOutAddress, asuint(fSurfaceHeight));
}

[numthreads(29, 29, 1)]
void CS_WATER_VERTEX_CALCULATION(CS_IN In)
{
    uint3 iDispatchThreadID = asuint(In.iDispatchThreadID);
    
    int threadIndexX = iDispatchThreadID.x;
    int threadIndexY = iDispatchThreadID.y;
    
    uint globalIndex = threadIndexX + threadIndexY * g_iSimulateBufferWidth;
    
    float fSurfaceHeight = asfloat(g_AnimatedSurface.Load(globalIndex * 4));
    g_SurfaceVertices[globalIndex].vPosition.y = fSurfaceHeight;
    
    // 하이트맵에서 노멀 계산하기
    // https://stackoverflow.com/questions/49640250/calculate-normals-from-heightmap
    
    int leftIndex = max(threadIndexX - 1, 0);
    int rightIndex = min(threadIndexX + 1, g_iSimulateBufferWidth - 1);
    int aboveIndex = max(threadIndexY - 1, 0);
    int belowIndex = min(threadIndexY + 1, g_iSimulateBufferWidth - 1);

    float leftHeight = asfloat(g_AnimatedSurface.Load((globalIndex - 1) * 4));
    float rightHeight = asfloat(g_AnimatedSurface.Load((globalIndex + 1) * 4));
    float aboveHeight = asfloat(g_AnimatedSurface.Load((threadIndexX + aboveIndex * g_iSimulateBufferWidth) * 4));
    float belowHeight = asfloat(g_AnimatedSurface.Load((threadIndexX + belowIndex * g_iSimulateBufferWidth) * 4));

    float dx = (rightHeight - leftHeight) * 2;
    float dz = (aboveHeight - belowHeight) * 2;

    g_SurfaceVertices[globalIndex].vNormal.x = dx;
    g_SurfaceVertices[globalIndex].vNormal.y = 4;
    g_SurfaceVertices[globalIndex].vNormal.z = -dz;
    
    g_SurfaceVertices[globalIndex].vNormal = normalize(g_SurfaceVertices[globalIndex].vNormal);
    
    if (threadIndexY <= g_iQuadTreeGridWidth && threadIndexX <= g_iQuadTreeGridWidth)
    {
        uint iRelativeIndex = threadIndexX + threadIndexY * g_iQuadTreeGridWidth;
        g_SurfacePosition[iRelativeIndex] = g_SurfaceVertices[globalIndex].vPosition;
    }
}

[numthreads(29, 29, 1)]
void CS_WATER_VERTEX_INITIALIZE(CS_IN In)
{
    float fGridEntireWidth = g_fGridCellWidth * (g_iSimulateBufferWidth - 1);
    
    uint3 iDispatchThreadID = asuint(In.iDispatchThreadID);
    
    uint iThreadIndexX = iDispatchThreadID.x;
    uint iThreadIndexY = iDispatchThreadID.y;
    
    VS_VERTEX_IN Vertex;
    
    // Vertex Position 정의
	// Grid의 중앙이 (0, 0, 0)
    Vertex.vPosition.x = (iThreadIndexY * g_fGridCellWidth) - (fGridEntireWidth * 0.5);
    Vertex.vPosition.z = (iThreadIndexX * g_fGridCellWidth) - (fGridEntireWidth * 0.5);
    Vertex.vPosition.y = 0.f;
    
    // Normal
    // Surface의 가장자리에 위치한 Vertex의 법선 벡터의 크기는 1로 설정
    if (iThreadIndexX == 0 || iThreadIndexX == (g_iSimulateBufferWidth - 1) || iThreadIndexY == 0 || iThreadIndexY == (g_iSimulateBufferWidth - 1))
    {
        Vertex.vNormal = float3(0.0, 1.0, 0.0);
    }
    else
    {
        Vertex.vNormal = float3(0.0, 2 * g_fGridCellWidth, 0.0);
    }
    
    //_float2(j / (m_iNumVerticesX - 1.f), i / (m_iNumVerticesZ - 1.f));
    Vertex.vTexcoord = float2(iThreadIndexX / (g_iSimulateBufferWidth - 1.f) * 30.f, iThreadIndexY / (g_iSimulateBufferWidth - 1.f) * 30.f);
    
    g_SurfaceVertices[iThreadIndexX + iThreadIndexY * g_iSimulateBufferWidth] = Vertex;
    
    if (iThreadIndexY <= g_iQuadTreeGridWidth && iThreadIndexX <= g_iQuadTreeGridWidth)
    {
        uint iRelativeIndex = iThreadIndexX + iThreadIndexY * g_iQuadTreeGridWidth;
        g_SurfacePosition[iRelativeIndex] = Vertex.vPosition;
    }
}

technique11		DefaultTechnique
{
	pass SURFACE
	{
        SetRasterizerState(RS_Default);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
        HullShader = compile hs_5_0 HS_MAIN();
        DomainShader = compile ds_5_0 DS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN();

        ComputeShader = NULL;
    }

    pass SURFACE_WIREFRAME
    {
        SetRasterizerState(RS_Wireframe);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetDepthStencilState(DSS_Default, 0);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = compile hs_5_0 HS_MAIN();
        DomainShader = compile ds_5_0 DS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();

        ComputeShader = NULL;
    }

    pass SURFACE_NORMAL
    {
        SetRasterizerState(RS_Default);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetDepthStencilState(DSS_Default, 0);

        VertexShader = compile vs_5_0 VS_NORMAL_MAIN();
        GeometryShader = compile gs_5_0 GS_NORMAL_MAIN();
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_NORMAL_MAIN();

        ComputeShader = NULL;
    }
}

technique11     ComputeTechnique
{
    pass WATER_ANIMATION
    {
        VertexShader = NULL;
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = NULL;

        SetComputeShader(CompileShader(cs_5_0, CS_WATER_ANIMATION()));
    }

    pass WATER_VERTEX_CALCULATION
    {
        VertexShader = NULL;
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = NULL;

        SetComputeShader(CompileShader(cs_5_0, CS_WATER_VERTEX_CALCULATION()));
    }

    pass WATER_VERTEX_INITIALIZE
    {
        VertexShader = NULL;
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = NULL;

        SetComputeShader(CompileShader(cs_5_0, CS_WATER_VERTEX_INITIALIZE()));
    }
}