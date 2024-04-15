#pragma once

#include "Engine_Define.h"
#include "System.h"

#include "ImageProcess_Manager.h"

BEGIN(Engine)

class CPostprocess final : public ISystem
{
private:
	explicit CPostprocess() DEFAULT;
	virtual ~CPostprocess() DEFAULT;

public:
	void					Initialize(const ComPtr<ID3D11Device>&, const ComPtr<ID3D11DeviceContext>&,
								const ComPtr<ID3D11Texture2D>&, const ComPtr<ID3D11RenderTargetView>&,
								const _int _iWidth, const _int _iHeight, const _int _iBloomLevels, const IMAGE_PROCESS _eType);
	void					Tick(_float _fTimeDelta);
	HRESULT					Render() { return E_FAIL; }
	HRESULT					Render(IMAGE_PROCESS, _uint _iFilterPass, function<HRESULT(shared_ptr<class CShader>)> = nullptr);
	HRESULT					Render(IMAGE_PROCESS, _uint _iFilterPass);
	HRESULT					Render(IMAGE_PROCESS, _uint _iFilterPass, _uint _iLUTIndex);

public:
	void					FadeInExposure(_float _fEndExposure, _float _fTimeScale);
	void					FadeInGamma(_float _fEndGamma, _float _fTimeScale);
	void					FadeOutExposure(_float _fEndExposure, _float _fTimeScale);
	void					FadeOutGamma(_float _fEndGamma, _float _fTimeScale);

public:
	void					Set_Exposure(_float _fExposure);
	void					Set_Gamma(_float _fGamma);
	void					Set_BloomStrength(_float _fStrength);

	_float					Get_Exposure();
	_float					Get_Gamma();
	_float					Get_BloomStrength();

public:
	void					Enable_FXAA(_bool);
	_bool					Enable_FXAA() { return m_bEnableFXAAFilter; }

public:
	void					Enable_DOF(_bool);
	_bool					Enable_DOF() { return m_bEnableDOFFilter; }

	void					Set_DepthStart(_float);
	void					Set_DepthRange(_float);

	_float					Get_DepthStart();
	_float					Get_DepthRange();

public:
	void					MaskingLUT(_bool _bMasking, shared_ptr<class CTexture>);

public:
	shared_ptr<class CShader>	Get_Shader() { return m_pPostprocessShader; }

private:
	void					RenderImageFilter(std::shared_ptr<class CImageFilter> _pImageFilter, _uint _iPassIndex, function<HRESULT(shared_ptr<class CShader>)> = nullptr);

	void					Create_VIBuffer();
	void					Create_ImageBuffer(_uint _iWidth, _uint _iHeight, _Out_ ComPtr<ID3D11ShaderResourceView>&, _Out_ ComPtr<ID3D11RenderTargetView>&);

private:
	void					Redirect_FilterTargets();

private:
	ComPtr<ID3D11Texture2D>							m_pBackBuffer;
	ComPtr<ID3D11RenderTargetView>					m_pBackbufferRenderTargetView;

	ComPtr<ID3D11DepthStencilView>					m_pDepthStencilView;

private:
	ComPtr<ID3D11Texture2D>							m_pFloatBuffer;
	ComPtr<ID3D11ShaderResourceView>				m_pFloatShaderResourceView;
	ComPtr<ID3D11RenderTargetView>					m_pFloatRenderTargetView;

private:
	_uint											m_iWidth = { 0 };
	_uint											m_iHeight = { 0 };

	ComPtr<ID3D11Buffer>							m_pVertexBuffer;
	ComPtr<ID3D11Buffer>							m_pIndexBuffer;

private:
	std::shared_ptr<class CShader>					m_pPostprocessShader;

	std::vector<ComPtr<ID3D11ShaderResourceView>>	m_vecBloomShaderResourceView;
	std::vector<ComPtr<ID3D11RenderTargetView>>		m_vecBloomRenderTargetView;

	ComPtr<ID3D11ShaderResourceView>				m_pBufferShaderResourceView;
	ComPtr<ID3D11RenderTargetView>					m_pBufferRenderTargetView;

private:
	shared_ptr<class CImageFilter>					m_pBloomLevel1Filter;
	ComPtr<ID3D11ShaderResourceView>				m_pBloomLevel1ShaderResourceView;
	ComPtr<ID3D11RenderTargetView>					m_pBloomLevel1RenderTargetView;

private:
	ComPtr<ID3D11ShaderResourceView>				m_pFXAAShaderResourceView;
	ComPtr<ID3D11RenderTargetView>					m_pFXAARenderTargetView;

private:
	std::shared_ptr<class CImageFilter>				m_pCombineFilter;
	vector<std::shared_ptr<class CImageFilter>>		m_vecBloomDownFilters;
	vector<std::shared_ptr<class CImageFilter>>		m_vecBloomUpFilters;

	shared_ptr<class CImageFilter>					m_pFXAAFilter;
	shared_ptr<class CImageFilter>					m_pDOFFilter;

private:
	_float											m_fBloomStrength = { 0.01f };
	_float											m_fExposure = { 3.f };
	_float											m_fGamma = { 1.3f };

private:
	_bool											m_bFadeInExposure = { false };
	std::pair<_float, _float>						m_FadeInExposurePair;
	_bool											m_bFadeInGamma = { false };
	std::pair<_float, _float>						m_FadeInGammaPair;

	_bool											m_bFadeOutExposure = { false };
	std::pair<_float, _float>						m_FadeOutExposurePair;
	_bool											m_bFadeOutGamma = { false };
	std::pair<_float, _float>						m_FadeOutGammaPair;

private:
	_bool											m_bEnableFXAAFilter = { false };

private:
	_bool											m_bEnableDOFFilter = { false };
	_float											m_fDepthStart = { 15.f };
	_float											m_fDepthRange = { 20.f };

private:
	IMAGE_PROCESS									m_eImageProcessType;

private:
	ComPtr<ID3D11Device>							m_pDevice;
	ComPtr<ID3D11DeviceContext>						m_pContext;

	friend CImageProcess_Manager;
};

END