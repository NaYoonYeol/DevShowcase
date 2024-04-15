#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CRenderer;
class CRigidbody;
class CRigidbodyState;
class CComputeBuffer;
class CVIBuffer_Surface;

class CQuadTree;
END

BEGIN(Client)

class COcean : public CGameObject
{
private:
	COcean(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	COcean(const COcean& rhs);
	virtual ~COcean() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();

public:
	void StormEnable(_bool bStorm) { m_bStorm = bStorm; }

public:
	void Animate_Ship(CRigidbody* pRigidbodyCom, const GEOMETRYDESC& GeometryDesc);
	_bool Compute_WaterHeightA(_float fX, _float fZ, _float& fWaterHeight);

private:
	void Interpolation_Water(const float fX, const float fZ, float& fWaterHeight, XMFLOAT3& vWaterNormal);

private:
	CShader* m_pShaderCom = { nullptr };
	CRenderer* m_pRendererCom = { nullptr };
	CComputeBuffer* m_pComputeBufferCom = { nullptr };
	CRigidbodyState* m_pRigidbodyStateCom = { nullptr };
	CVIBuffer_Surface* m_pVIBufferCom = { nullptr };

private:
	CQuadTree* m_pQuadTree = { nullptr };

private:
	/* Surface Simulation Variable */
	_uint m_iSimulateGridWidth = { 522 };
	_uint m_iQuadTreeGridWidth = { 512 + 1 };
	_uint m_iPixelsPerThread = { 18 }; // 한 Thread에서 처리할 Pixel의 수의 제곱근
	_float m_fWaveSpaceOrigin = { 2.f }; // 파동의 속도
	_float m_fGridCellWidth = 8 * float(0.1 * m_fWaveSpaceOrigin * sqrt(2)); // 그리드 셀 너비
	_float m_fGridEntireWidth = m_fGridCellWidth * (m_iSimulateGridWidth - 1); // 그리드 셀 전체 너비
	
	/* Surface Simulation Datas */
	std::vector<float> m_vecPrevSurfaceHeight; // 이전 수면 높이
	std::vector<float> m_vecCurrentSurfaceHeight; // 현재 수면 높이

	std::vector<float> m_vecDampingCoefficient; // 감쇠 계수

	/* Animated Surface Vertices */
	std::vector<SURFACE> m_vecAnimatedSurfaceVertices;
	std::vector<_float3> m_vecAnimatedSurfacePosition;

private: /* 렌더링할 Patch의 Grid Index들 */
	std::vector<INSTSURFACE> m_vecPatch;

private:
	_float m_fWaveTimeStep = { 0.025f }; // 파동 시간 간격

	/* Tessellation Parmater */
	_float m_fDynamicTesselationAmount = { 10.f };
	_float m_fStaticTessellationOffset = { 3.f };

	/* Wind Simulation Parameter */
	_float m_fDampingCoefficient = { 0.9999f };

	_float m_fLocalWavesSimulationDomainWorldspaceSize = { 0.f };
	_float2 m_fLocalWavesSimulationDomainWorldspaceCenter = { 0.f, 0.f };

private: /* Buoyancy Physics */
	_float m_fWaterDensity = { 1000.f };
	_float m_fGravity = { 9.8f };

	_float m_fDragForce = { -10000.f };
	_float m_fVelocityDampingCoefficient = { 0.5f };

private:
	_float2 m_vWindWave = { 0.f, 0.f };

private:
	typedef struct tagComputeOutput_SurfaceHeight
	{
		_float fReturnValue;
	}COMPUTEOUTPUT_SURFACEHEIGHT;

private:
	CTexture* m_pIBLTexture = { nullptr };
	CTexture* m_pGradientsTexture = { nullptr };
	CTexture* m_pMomentsTexture = { nullptr };

	CTexture* m_pFlowingWaterTexture = { nullptr };

	CTexture* m_pFoamTexture = { nullptr };
	
private:
	HRESULT Add_Components();

private:
	void InitSurface();
	void AnimateWater();

	void TuneDampingCoefficient(_float fTimeDelta);

private:
	_float m_fAmplificationAccTime = { 0.f };
	_float m_fAmplificationInterval = { 50.f };

	_float m_fAmplificationLimitAccTime = { 0.f };
	_float m_fAmplificationLimitInterval = { 1.f };

	_bool m_bStorm = { false };

public:
	static COcean* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)	override;
	virtual void Free() override;
};

END