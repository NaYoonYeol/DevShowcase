#pragma once
#include "Component.h"

BEGIN(Engine)

#define MAX_SHADER_RESOURCE_VIEW 8
#define MAX_UNORDERED_ACCESS_VIEW 8

typedef ENGINE_DLL struct tagComputeShaderDesc
{
	_uint iNumInputData;
	_uint iInputStride[MAX_SHADER_RESOURCE_VIEW];
	_uint iNumInputElements[MAX_SHADER_RESOURCE_VIEW];
	void** pInputData;
	
	_uint iNumOutputData;
	_uint iOutputStride[MAX_UNORDERED_ACCESS_VIEW];
	_uint iNumOutputElements[MAX_UNORDERED_ACCESS_VIEW];
	_bool bIsStructuredBuffer[MAX_UNORDERED_ACCESS_VIEW];
}COMPUTESHADERDESC;

class ENGINE_DLL CComputeBuffer final : public CComponent
{
private:
	CComputeBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CComputeBuffer(const CComputeBuffer& rhs);
	virtual ~CComputeBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	HRESULT Bind_ShaderResourceView(class CShader* pShader, const char* pConstantName, const _uint iResourceIndex);
	HRESULT Bind_UnorderedAccessView(class CShader* pShader, const char* pConstantName, const _uint iIndex);

	HRESULT Bind_Result_ShaderResourceView(class CShader* pShader, const char* pConstantName, const _uint iIndex);
	
	HRESULT Dispatch(class CShader* pShader, _uint iTechnique, _uint iPass, _uint iX, _uint iY, _uint iZ);

	void StoreOutput(void* pData, const _uint iIndex);
	void Copy_OutputToShaderResourceView(const _uint iOutputIndex, const _uint iSRVIndex);

public:
	HRESULT Update_ShaderResourceView(void* pData, const _uint iResourceIndex);
	
private:
	ID3D11Resource* m_pInputBuffer[MAX_SHADER_RESOURCE_VIEW] = { nullptr, };
	ID3D11ShaderResourceView* m_pSRV[MAX_SHADER_RESOURCE_VIEW] = { nullptr, };
	
	ID3D11Resource* m_pOutputBuffer[MAX_UNORDERED_ACCESS_VIEW] = {nullptr, };
	ID3D11UnorderedAccessView* m_pUAV[MAX_UNORDERED_ACCESS_VIEW] = {nullptr, };

	// UAV 결과를 복사하기 위한 버퍼
	ID3D11Resource* m_pResultBuffer[MAX_UNORDERED_ACCESS_VIEW] = {nullptr, };
	
	// 결과를 Vertex Shader로 넘기기 위한 SRV
	ID3D11Resource* m_pResultInputBuffer[MAX_UNORDERED_ACCESS_VIEW] = { nullptr, };
	ID3D11ShaderResourceView* m_pResultSRV[MAX_UNORDERED_ACCESS_VIEW] = { nullptr, };
	
private:
	_uint m_iNumInputData = { 0 };
	_uint m_iInputStride[MAX_SHADER_RESOURCE_VIEW];
	_uint m_iNumInputElements[MAX_SHADER_RESOURCE_VIEW];

	void** m_pInputData = { nullptr };
	
private:
	_uint m_iNumOutputData = { 0 };
	_uint m_iOutputStride[MAX_UNORDERED_ACCESS_VIEW];
	_uint m_iNumOutputElements[MAX_UNORDERED_ACCESS_VIEW];
	_bool m_bIsStructuredBuffer[MAX_UNORDERED_ACCESS_VIEW];

public:
	static CComputeBuffer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END