#pragma once

#include "Engine_Define.h"

BEGIN(Engine)

class CImageFilter final
{
private:
	explicit CImageFilter() DEFAULT;
	virtual ~CImageFilter() DEFAULT;

public:
	void					Initialize(const ComPtr<ID3D11Device>&, const ComPtr<ID3D11DeviceContext>&,
								std::shared_ptr<class CShader> _pPixelShader, _int _iWidth, _int _iHeight);

	void					Render();
	void					Clear(_float4 _vClearColor);

	void					SetShaderResources(const std::vector<ComPtr<ID3D11ShaderResourceView>>& _vecResources);
	void					SetRenderTargets(const std::vector<ComPtr<ID3D11RenderTargetView>>& _vecTargets);

public:
	void					Bind_RawValue(const char* _pConstantName, const void* _pData, _uint _iByteSize);

public:
	void					Append_LUTTexture(const _wchar* _pLUTTag, shared_ptr<class CTexture>);
	void					Bind_LUTTexture(_uint _iLUTFilterIndex);
	void					MaskingLUT(_bool _bMasking, shared_ptr<class CTexture>);

private:
	ComPtr<ID3D11Device>									m_pDevice;
	ComPtr<ID3D11DeviceContext>								m_pContext;

	std::shared_ptr<class CShader>							m_pPixelShader;

	D3D11_VIEWPORT											m_tViewport = {};
	_float2													m_vSampleStep = {};

private:
	std::vector<pair<wstring, shared_ptr<class CTexture>>>	m_vecLUTFilters;
	shared_ptr<class CTexture>								m_pMaskingTexture;
	_bool													m_bExecuteLUT = { false };
	_bool													m_bMaskingLUT = { false };
	_uint													m_iLUTFilterIndex = { 0 };

private:
	std::vector<ComPtr<ID3D11ShaderResourceView>>			m_vecShaderResources;
	std::vector<ComPtr<ID3D11RenderTargetView>>				m_vecRenderTargets;

public:
	static shared_ptr<CImageFilter> Create();
};

END