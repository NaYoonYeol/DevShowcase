#pragma once
#include "Engine_Define.h"

BEGIN(Engine)

class ENGINE_DLL CImageProcess_Manager final : public CSingleton<CImageProcess_Manager>
{
private:
	explicit CImageProcess_Manager();
	virtual ~CImageProcess_Manager() DEFAULT;


public:
	void			Initialize(IMAGE_PROCESS, const ComPtr<ID3D11Device>&, const ComPtr<ID3D11DeviceContext>&,
						const ComPtr<ID3D11Texture2D>&, const ComPtr<ID3D11RenderTargetView>&,
						const _int _iWidth, const _int _iHeight, const _int _iBloomLevels);

	void			Initialize(IMAGE_PROCESS, const ComPtr<ID3D11Device>&, const ComPtr<ID3D11DeviceContext>&,
						const ComPtr<ID3D11Texture2D>&, const ComPtr<ID3D11RenderTargetView>&,
						const _int _iWidth, const _int _iHeight, const _int _iBloomLevels, _float _fBloomStrength, _float _fExposure, _float _fGamma);

	void			Tick(_float _fTimeDelta);
	void			Excute_ImageProcess(IMAGE_PROCESS _eProcessType, _uint _iFilterPass = 2, function<HRESULT(shared_ptr<class CShader>)> = nullptr);

public:
#pragma region Postprocess
	// Tonemapping Settings
public:
	void			FadeInExposure(IMAGE_PROCESS, _float _fEndExposure, _float _fTimeScale);
	void			FadeInGamma(IMAGE_PROCESS, _float _fEndGamma, _float _fTimeScale);
	void			FadeOutExposure(IMAGE_PROCESS, _float _fEndExposure, _float _fTimeScale);
	void			FadeOutGamma(IMAGE_PROCESS, _float _fEndGamma, _float _fTimeScale);

	void			Set_Exposure(IMAGE_PROCESS, _float _fExposure);
	void			Set_Gamma(IMAGE_PROCESS, _float _fGamma);
	void			Set_BloomStrength(IMAGE_PROCESS, _float _fStrength);

	_float			Get_Exposure(IMAGE_PROCESS);
	_float			Get_Gamma(IMAGE_PROCESS);
	_float			Get_BloomStrength(IMAGE_PROCESS);
#pragma endregion

#pragma region LUT Filter
	void			Enable_LUTFilter(_bool _bEnable) { m_bLUTFilter = _bEnable; }
	_bool			Enable_LUTFilter() { return m_bLUTFilter; }
	void			Set_LUTIndex(_uint _iLUTIndex) { m_iLUTIndex = _iLUTIndex; }
	_uint			Get_LUTIndex() { return m_iLUTIndex; }

	void			MaskingLUT(_bool _bMasking, shared_ptr<class CTexture> = nullptr);
#pragma endregion

#pragma region Bloom

#pragma endregion

#pragma region FXAA
	void			Enable_FXAA(_bool);
	_bool			Enable_FXAA();
#pragma endregion

#pragma region DOF
	void			Enable_DOF(_bool);
	_bool			Enable_DOF();

	void			Set_DOF_DepthStart(_float _fDepthStart);
	void			Set_DOF_DepthRange(_float _fDepthRange);

	_float			Get_DOF_DepthStart();
	_float			Get_DOF_DepthRange();
#pragma endregion

public:
	shared_ptr<class CShader>				Get_PostShader(IMAGE_PROCESS);

private:
	shared_ptr<class CPostprocess>			m_pImageProcessor[IDX(IMAGE_PROCESS::MAX)];

private:
	_bool									m_bLUTFilter = { false };
	_uint									m_iLUTIndex = { 0 };

	friend CSingleton<CImageProcess_Manager>;
};

END