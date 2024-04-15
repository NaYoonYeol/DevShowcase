#include "EnginePCH.h"
#include "ImageProcess_Manager.h"

#include "Postprocess.h"
#include "Engine_Macro.h"

CImageProcess_Manager::CImageProcess_Manager()
{
	for (_uint i = 0; i < IDX(IMAGE_PROCESS::MAX); ++i)
	{
		m_pImageProcessor[i] = make_private_shared(CPostprocess);
	}
}

void CImageProcess_Manager::Initialize(IMAGE_PROCESS _eProcessType, const ComPtr<ID3D11Device>& _pDevice, const ComPtr<ID3D11DeviceContext>& _pContext,
	const ComPtr<ID3D11Texture2D>& _pTexture, const ComPtr<ID3D11RenderTargetView>& _pRenderTargetView, 
	const _int _iWidth, const _int _iHeight, const _int _iBloomLevels)
{
	m_pImageProcessor[IDX(_eProcessType)]->Initialize(_pDevice, _pContext, _pTexture, _pRenderTargetView, _iWidth, _iHeight, _iBloomLevels, _eProcessType);
}

void CImageProcess_Manager::Initialize(IMAGE_PROCESS _eProcessType, const ComPtr<ID3D11Device>& _pDevice, const ComPtr<ID3D11DeviceContext>& _pContext,
	const ComPtr<ID3D11Texture2D>& _pTexture, const ComPtr<ID3D11RenderTargetView>& _pRenderTargetView, 
	const _int _iWidth, const _int _iHeight, const _int _iBloomLevels, _float _fBloomStrength, _float _fExposure, _float _fGamma)
{
	Initialize(_eProcessType, _pDevice, _pContext, _pTexture, _pRenderTargetView, _iWidth, _iHeight, _iBloomLevels);

	Set_BloomStrength(_eProcessType, _fBloomStrength);
	Set_Exposure(_eProcessType, _fExposure);
	Set_Gamma(_eProcessType, _fGamma);
}

void CImageProcess_Manager::Tick(_float _fTimeDelta)
{
	for (_uint i = 0; i < IDX(IMAGE_PROCESS::MAX); ++i)
	{
		if (m_pImageProcessor[i])
			m_pImageProcessor[i]->Tick(_fTimeDelta);
	}
}

void CImageProcess_Manager::Excute_ImageProcess(IMAGE_PROCESS _eProcessType, _uint _iFilterPass, function<HRESULT(shared_ptr<CShader>)> _fpListner)
{
	if (_eProcessType == IMAGE_PROCESS::PROCESS_TONEMAPPING
		&& m_bLUTFilter)
	{
		m_pImageProcessor[IDX(_eProcessType)]->Render(_eProcessType, 5, m_iLUTIndex);
	}
	else
	{
		m_pImageProcessor[IDX(_eProcessType)]->Render(_eProcessType, _iFilterPass, _fpListner);
	}
}

void CImageProcess_Manager::FadeInExposure(IMAGE_PROCESS _eProcessType, _float _fEndExposure, _float _fTimeScale)
{
	m_pImageProcessor[IDX(_eProcessType)]->FadeInExposure(_fEndExposure, _fTimeScale);
}

void CImageProcess_Manager::FadeInGamma(IMAGE_PROCESS _eProcessType, _float _fEndGamma, _float _fTimeScale)
{
	m_pImageProcessor[IDX(_eProcessType)]->FadeInGamma(_fEndGamma, _fTimeScale);
}

void CImageProcess_Manager::FadeOutExposure(IMAGE_PROCESS _eProcessType, _float _fEndExposure, _float _fTimeScale)
{
	m_pImageProcessor[IDX(_eProcessType)]->FadeOutExposure(_fEndExposure, _fTimeScale);
}

void CImageProcess_Manager::FadeOutGamma(IMAGE_PROCESS _eProcessType, _float _fEndGamma, _float _fTimeScale)
{
	m_pImageProcessor[IDX(_eProcessType)]->FadeOutGamma(_fEndGamma, _fTimeScale);
}

void CImageProcess_Manager::Set_Exposure(IMAGE_PROCESS _eProcessType, _float _fExposure)
{
	m_pImageProcessor[IDX(_eProcessType)]->Set_Exposure(_fExposure);
}

void CImageProcess_Manager::Set_Gamma(IMAGE_PROCESS _eProcessType, _float _fGamma)
{
	m_pImageProcessor[IDX(_eProcessType)]->Set_Gamma(_fGamma);
}

void CImageProcess_Manager::Set_BloomStrength(IMAGE_PROCESS _eProcessType, _float _fStrength)
{
	m_pImageProcessor[IDX(_eProcessType)]->Set_BloomStrength(_fStrength);
}

_float CImageProcess_Manager::Get_Exposure(IMAGE_PROCESS _eProcessType)
{
	return m_pImageProcessor[IDX(_eProcessType)]->Get_Exposure();
}

_float CImageProcess_Manager::Get_Gamma(IMAGE_PROCESS _eProcessType)
{
	return m_pImageProcessor[IDX(_eProcessType)]->Get_Gamma();
}

_float CImageProcess_Manager::Get_BloomStrength(IMAGE_PROCESS _eProcessType)
{
	return m_pImageProcessor[IDX(_eProcessType)]->Get_BloomStrength();
}

shared_ptr<class CShader> CImageProcess_Manager::Get_PostShader(IMAGE_PROCESS _eProcessType)
{
	return m_pImageProcessor[IDX(_eProcessType)]->Get_Shader();
}

void CImageProcess_Manager::MaskingLUT(_bool _bMasking, shared_ptr<CTexture> _pMaskingTexture)
{
	m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->MaskingLUT(_bMasking, _pMaskingTexture);
}

void CImageProcess_Manager::Enable_FXAA(_bool _bEnable)
{
	m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->Enable_FXAA(_bEnable);
}

_bool CImageProcess_Manager::Enable_FXAA()
{
	return m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->Enable_FXAA();
}

void CImageProcess_Manager::Enable_DOF(_bool _bEnable)
{
	m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->Enable_DOF(_bEnable);
}

_bool CImageProcess_Manager::Enable_DOF()
{
	return m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->Enable_DOF();
}

void CImageProcess_Manager::Set_DOF_DepthStart(_float _fDepthStart)
{
	m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->Set_DepthStart(_fDepthStart);
}

void CImageProcess_Manager::Set_DOF_DepthRange(_float _fDepthRange)
{
	m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->Set_DepthRange(_fDepthRange);
}

_float CImageProcess_Manager::Get_DOF_DepthStart()
{
	return m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->Get_DepthStart();
}

_float CImageProcess_Manager::Get_DOF_DepthRange()
{
	return m_pImageProcessor[IDX(IMAGE_PROCESS::PROCESS_TONEMAPPING)]->Get_DepthRange();
}
