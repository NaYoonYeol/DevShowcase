#include "EnginePCH.h"
#include "Postprocess.h"

#include "GeometryGenerator.h"
#include "Shader.h"
#include "Texture.h"

#include "ImageFilter.h"
#include "RenderTarget_Manager.h"
#include "PipeLine.h"
#include "Camera.h"

void CPostprocess::Initialize(const ComPtr<ID3D11Device>& _pDevice, const ComPtr<ID3D11DeviceContext>& _pContext, 
	const ComPtr<ID3D11Texture2D>& _pBackBuffer, const ComPtr<ID3D11RenderTargetView>& _pBackBufferRenderTargetView,
	const _int _iWidth, const _int _iHeight, const _int _iBloomLevels, const IMAGE_PROCESS _eType)
{
	m_pDevice = _pDevice;
	m_pContext = _pContext;

	m_pBackBuffer = _pBackBuffer;
	m_pBackbufferRenderTargetView = _pBackBufferRenderTargetView;

	m_eImageProcessType = _eType;

	D3D11_TEXTURE2D_DESC tFloatTextureDesc;
	_pBackBuffer->GetDesc(&tFloatTextureDesc);
	tFloatTextureDesc.MipLevels = tFloatTextureDesc.ArraySize = 1;
	tFloatTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tFloatTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	tFloatTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	tFloatTextureDesc.MiscFlags = 0;
	tFloatTextureDesc.CPUAccessFlags = 0;
	tFloatTextureDesc.SampleDesc.Count = 1;
	tFloatTextureDesc.SampleDesc.Quality = 0;

	if (FAILED(_pDevice->CreateTexture2D(&tFloatTextureDesc, nullptr, m_pFloatBuffer.GetAddressOf())))
		assert(false && L"CreateTexture2D Failed");

	if (FAILED(_pDevice->CreateShaderResourceView(m_pFloatBuffer.Get(), nullptr, m_pFloatShaderResourceView.GetAddressOf())))
		assert(false && L"CreateShaderResourceView Failed");

	if (FAILED(_pDevice->CreateRenderTargetView(m_pFloatBuffer.Get(), nullptr, m_pFloatRenderTargetView.GetAddressOf())))
		assert(false && L"CreateRenderTargetView Failed");

	m_iWidth = _iWidth;
	m_iHeight = _iHeight;

	Create_VIBuffer();

	m_pPostprocessShader = CShader::Create(_pDevice, _pContext, TEXT("Bin/Resources/Shader/PosTex_Postprocess.hlsl"), VTXPOSTEX::tElements, VTXPOSTEX::iNumElement);

	m_vecBloomShaderResourceView.resize(_iBloomLevels);
	m_vecBloomRenderTargetView.resize(_iBloomLevels);
	for (_int i = 0; i < _iBloomLevels; ++i)
	{
		_int iDiv = _int(pow(2, i));
		Create_ImageBuffer(_iWidth / iDiv, _iHeight / iDiv, m_vecBloomShaderResourceView[i], m_vecBloomRenderTargetView[i]);
	}
	Create_ImageBuffer(_iWidth, _iHeight, m_pFXAAShaderResourceView, m_pFXAARenderTargetView);
	Create_ImageBuffer(_iWidth, _iHeight, m_pBufferShaderResourceView, m_pBufferRenderTargetView);
	Create_ImageBuffer(_iWidth, _iHeight, m_pBloomLevel1ShaderResourceView, m_pBloomLevel1RenderTargetView);

	m_vecBloomDownFilters.resize(_iBloomLevels - 1);
	for (_int i = 0; i < _iBloomLevels - 1; i++) {
		_int div = _int(pow(2, i + 1));
		m_vecBloomDownFilters[i] = CImageFilter::Create();
		m_vecBloomDownFilters[i]->Initialize(
			_pDevice, _pContext, m_pPostprocessShader, _iWidth / div, _iHeight / div);
		if (i == 0) {
			m_vecBloomDownFilters[i]->SetShaderResources({ m_pFloatShaderResourceView });
		}
		else {
			m_vecBloomDownFilters[i]->SetShaderResources({ m_vecBloomShaderResourceView[i] });
		}

		m_vecBloomDownFilters[i]->SetRenderTargets({ m_vecBloomRenderTargetView[i + 1] });
	}

	m_vecBloomUpFilters.resize(_iBloomLevels - 1, nullptr);
	for (_int i = 0; i < _iBloomLevels - 1; i++) {
		_int level = _iBloomLevels - 2 - i; // 4, 2, 1
		_int div = _int(pow(2, level));
		m_vecBloomUpFilters[i] = CImageFilter::Create();
		m_vecBloomUpFilters[i]->Initialize(_pDevice, _pContext, m_pPostprocessShader,
			_iWidth / div, _iHeight / div);
		m_vecBloomUpFilters[i]->SetShaderResources({ m_vecBloomShaderResourceView[level + 1] });
		m_vecBloomUpFilters[i]->SetRenderTargets({ m_vecBloomRenderTargetView[level] });
	}

	m_pCombineFilter = CImageFilter::Create();
	m_pCombineFilter->Initialize(_pDevice, _pContext, m_pPostprocessShader, _iWidth,
		_iHeight);
	m_pCombineFilter->SetShaderResources({ m_pFloatShaderResourceView, m_vecBloomShaderResourceView[0] });
	m_pCombineFilter->SetRenderTargets({ _pBackBufferRenderTargetView });

	m_pCombineFilter->Bind_RawValue("g_fBloomStrength", &m_fBloomStrength, sizeof(float));
	m_pCombineFilter->Bind_RawValue("g_Exposure", &m_fExposure, sizeof(float));
	m_pCombineFilter->Bind_RawValue("g_Gamma", &m_fGamma, sizeof(float));

	if (m_eImageProcessType == IMAGE_PROCESS::PROCESS_TONEMAPPING)
	{
		m_pCombineFilter->Append_LUTTexture(TEXT("g_LUT_Default"),
			CTexture::Create(m_pDevice, m_pContext, TEXT("Bin/Resources/Texture/LUT/LUT_Default.png")));
		m_pCombineFilter->Append_LUTTexture(TEXT("g_LUT_Tunnel"),
			CTexture::Create(m_pDevice, m_pContext, TEXT("Bin/Resources/Texture/LUT/LUT_tunnel_01.png")));
		m_pCombineFilter->Append_LUTTexture(TEXT("g_LUT_01"),
			CTexture::Create(m_pDevice, m_pContext, TEXT("Bin/Resources/Texture/LUT/LUT_01.png")));
		m_pCombineFilter->Append_LUTTexture(TEXT("BlueHue"),
			CTexture::Create(m_pDevice, m_pContext, TEXT("Bin/Resources/Texture/LUT/BlueHue.png")));
		m_pCombineFilter->Append_LUTTexture(TEXT("RaptorHunt"),
			CTexture::Create(m_pDevice, m_pContext, TEXT("Bin/Resources/Texture/LUT/RaptorHunt.png")));

		m_pCombineFilter->Append_LUTTexture(TEXT("T_SunnyNoisePowerCurve"),
			CTexture::Create(m_pDevice, m_pContext, TEXT("Bin/Resources/Texture/LUT/T_SunnyNoisePowerCurve.png")));
		m_pCombineFilter->Append_LUTTexture(TEXT("T_DefaultColorGrading"),
			CTexture::Create(m_pDevice, m_pContext, TEXT("Bin/Resources/Texture/LUT/T_DefaultColorGrading.png")));

		m_pDOFFilter = CImageFilter::Create();
		m_pDOFFilter->Initialize(_pDevice, _pContext, m_pPostprocessShader, _iWidth, _iHeight);
	}

	m_pFXAAFilter = CImageFilter::Create();
	m_pFXAAFilter->Initialize(_pDevice, _pContext, m_pPostprocessShader, _iWidth, _iHeight);
	
	if (m_eImageProcessType == IMAGE_PROCESS::PROCESS_TONEMAPPING)
	{
		m_pBloomLevel1Filter = CImageFilter::Create();
		m_pBloomLevel1Filter->Initialize(_pDevice, _pContext, m_pPostprocessShader, _iWidth, _iHeight);
		m_pBloomLevel1Filter->SetShaderResources({ m_vecBloomShaderResourceView[1] });
		m_pBloomLevel1Filter->SetRenderTargets({ m_pBloomLevel1RenderTargetView });
	}
}

void CPostprocess::Tick(_float _fTimeDelta)
{
	if (m_bFadeInExposure)
	{
		_float fEndExposure = m_FadeInExposurePair.first;
		_float fTimeScale = m_FadeInExposurePair.second;

		if (m_fExposure < fEndExposure)
		{
			m_fExposure += _fTimeDelta * fTimeScale;
		}
		else
		{
			m_fExposure = fEndExposure;
			m_bFadeInExposure = false;
		}

		m_pCombineFilter->Bind_RawValue("g_Exposure", &m_fExposure, sizeof(float));
	}

	if (m_bFadeInGamma)
	{
		_float fEndGamma = m_FadeInGammaPair.first;
		_float fTimeScale = m_FadeInGammaPair.second;

		if (m_fGamma < fEndGamma)
		{
			m_fGamma += _fTimeDelta * fTimeScale;
		}
		else
		{
			m_fGamma = fEndGamma;
			m_bFadeInGamma = false;
		}

		m_pCombineFilter->Bind_RawValue("g_Gamma", &m_fGamma, sizeof(float));
	}

	if (m_bFadeOutExposure)
	{
		_float fEndExposure = m_FadeOutExposurePair.first;
		_float fTimeScale = m_FadeOutExposurePair.second;

		if (m_fExposure > fEndExposure)
		{
			m_fExposure -= _fTimeDelta * fTimeScale;
		}
		else

		{
			m_fExposure = fEndExposure;
			m_bFadeOutExposure = false;
		}
		
		m_pCombineFilter->Bind_RawValue("g_Exposure", &m_fExposure, sizeof(float));
	}

	if (m_bFadeOutGamma)
	{
		_float fEndGamma = m_FadeOutGammaPair.first;
		_float fTimeScale = m_FadeOutGammaPair.second;

		if (m_fGamma > fEndGamma)
		{
			m_fGamma -= _fTimeDelta * fTimeScale;
		}
		else
		{
			m_fGamma = fEndGamma;
			m_bFadeOutGamma = false;
		}

		m_pCombineFilter->Bind_RawValue("g_Gamma", &m_fGamma, sizeof(float));
	}
}

HRESULT CPostprocess::Render(IMAGE_PROCESS _eProcessType, _uint _iFilterPass, function<HRESULT(shared_ptr<CShader>)> _fpListener)
{
	ComPtr<ID3D11RenderTargetView> pBackbufferRenderTargetView;
	ComPtr<ID3D11DepthStencilView> pDepthStencilView;

	m_pContext->OMGetRenderTargets(1, pBackbufferRenderTargetView.GetAddressOf(), pDepthStencilView.GetAddressOf());
	m_pContext->CopyResource(m_pFloatBuffer.Get(), m_pBackBuffer.Get());

	UINT stride = sizeof(VTXPOSTEX);
	UINT offset = 0;

	m_pContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pContext->IASetIndexBuffer(m_pIndexBuffer.Get(),
		DXGI_FORMAT_R16_UINT,
		0);

	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int i = 0; i < m_vecBloomDownFilters.size(); i++)
	{
		RenderImageFilter(m_vecBloomDownFilters[i], 0);

		if (m_eImageProcessType == IMAGE_PROCESS::PROCESS_TONEMAPPING && i == 0)
		{
			RenderImageFilter(m_pBloomLevel1Filter, 1);
		}
	}

	for (int i = 0; i < m_vecBloomUpFilters.size(); i++)
	{
		RenderImageFilter(m_vecBloomUpFilters[i], 1);
	}

	m_pCombineFilter->Bind_RawValue("g_fBloomStrength", &m_fBloomStrength, sizeof(float));
	m_pCombineFilter->Bind_RawValue("g_Exposure", &m_fExposure, sizeof(float));
	m_pCombineFilter->Bind_RawValue("g_Gamma", &m_fGamma, sizeof(float));
	
	switch (_eProcessType)
	{
	case IMAGE_PROCESS::PROCESS_TONEMAPPING:
		RenderImageFilter(m_pCombineFilter, _iFilterPass, _fpListener);

		if (m_bEnableFXAAFilter)
		{
			RenderImageFilter(m_pFXAAFilter, 6);
		}

		if (m_bEnableDOFFilter)
		{
			RenderImageFilter(m_pDOFFilter, 7, [&](shared_ptr<CShader> _pPostProcess)->HRESULT
				{
					auto pRenderTarget_Manager = CRenderTarget_Manager::Get_Instance();
					if (FAILED(pRenderTarget_Manager->Bind_RenderTarget(RENDERTARGET_DEPTH, _pPostProcess, SHADER_TEXTARGET_DEPTH)))
					{
						MSG_RETURN(E_FAIL, "CPostprocess::Render", "Failed to Bind_RenderTarget: RENDERTARGET_DEPTH");
					}

					auto pPipeLine = CPipeLine::Get_Instance();
					const CCamera::CAMERA_DESC& tCamDesc = pPipeLine->Get_Camera()->Get_Desc();

					if (FAILED(_pPostProcess->Bind_Float("g_fNearPlane", tCamDesc.fNear)))
					{
						MSG_RETURN(E_FAIL, "CPostprocess::Render", "Failed to Bind_Float: g_fNearPlane");
					}
					if (FAILED(_pPostProcess->Bind_Float("g_fFarPlane", tCamDesc.fFar)))
					{
						MSG_RETURN(E_FAIL, "CPostprocess::Render", "Failed to Bind_Float: g_fFarPlane");
					}

					if (FAILED(_pPostProcess->Bind_Float("g_fDepthStart", m_fDepthStart)))
					{
						MSG_RETURN(E_FAIL, "CPostprocess::Render", "Failed to Bind_Float: g_fDepthStart");
					}
					if (FAILED(_pPostProcess->Bind_Float("g_fDepthRange", m_fDepthRange)))
					{
						MSG_RETURN(E_FAIL, "CPostprocess::Render", "Failed to Bind_Float: g_fDepthRange");
					}

					return S_OK;
				}
			);
		}

		break;
	case IMAGE_PROCESS::PROCESS_BLOOM:
	case IMAGE_PROCESS::PROCESS_BLOOM_STRENGTH:
	case IMAGE_PROCESS::PROCESS_NEON:
		RenderImageFilter(m_pCombineFilter, iFilterPass, _fpListener);
		break;
	}
	
	m_pContext->OMSetRenderTargets(1, pBackbufferRenderTargetView.GetAddressOf(), pDepthStencilView.Get());

	return S_OK;
}

HRESULT CPostprocess::Render(IMAGE_PROCESS _eProcessType, _uint _iFilterPass, _uint _iLUTIndex)
{
	m_pCombineFilter->Bind_LUTTexture(_iLUTIndex);

	return Render(_eProcessType, _iFilterPass, nullptr);
}

void CPostprocess::FadeInExposure(_float _fEndExposure, _float _fTimeScale)
{
	m_bFadeInExposure = true;

	m_FadeInExposurePair.first = _fEndExposure;
	m_FadeInExposurePair.second = _fTimeScale;
}

void CPostprocess::FadeInGamma(_float _fEndGamma, _float _fTimeScale)
{
	m_bFadeInGamma = true;

	m_FadeInGammaPair.first = _fEndGamma;
	m_FadeInGammaPair.second = _fTimeScale;
}

void CPostprocess::FadeOutExposure(_float _fEndExposure, _float _fTimeScale)
{
	m_bFadeOutExposure = true;

	m_FadeOutExposurePair.first = _fEndExposure;
	m_FadeOutExposurePair.second = _fTimeScale;
}

void CPostprocess::FadeOutGamma(_float _fEndGamma, _float _fTimeScale)
{
	m_bFadeOutGamma = true;

	m_FadeOutGammaPair.first = _fEndGamma;
	m_FadeOutGammaPair.second = _fTimeScale;
}

void CPostprocess::Set_Exposure(_float _fExposure)
{
	m_fExposure = _fExposure;

	m_pCombineFilter->Bind_RawValue("g_Exposure", &m_fExposure, sizeof(float));
}

void CPostprocess::Set_Gamma(_float _fGamma)
{
	m_fGamma = _fGamma;

	m_pCombineFilter->Bind_RawValue("g_Gamma", &m_fGamma, sizeof(float));
}

void CPostprocess::Set_BloomStrength(_float _fStrength)
{
	m_fBloomStrength = _fStrength;

	m_pCombineFilter->Bind_RawValue("g_fBloomStrength", &m_fBloomStrength, sizeof(float));
}

_float CPostprocess::Get_Exposure()
{
	return m_fExposure;
}

_float CPostprocess::Get_Gamma()
{
	return m_fGamma;
}

_float CPostprocess::Get_BloomStrength()
{
	return m_fBloomStrength;
}

void CPostprocess::Enable_FXAA(_bool _bEnable)
{
	m_bEnableFXAAFilter = _bEnable;

	Redirect_FilterTargets();
}

void CPostprocess::Enable_DOF(_bool _bEnable)
{
	m_bEnableDOFFilter = _bEnable;

	Redirect_FilterTargets();
}

void CPostprocess::Set_DepthStart(_float _fDepthStart)
{
	m_fDepthStart = _fDepthStart;
}

void CPostprocess::Set_DepthRange(_float _fDepthRange)
{
	m_fDepthRange = _fDepthRange;
}

_float CPostprocess::Get_DepthStart()
{
	return m_fDepthStart;
}

_float CPostprocess::Get_DepthRange()
{
	return m_fDepthRange;
}

void CPostprocess::MaskingLUT(_bool _bMasking, shared_ptr<class CTexture> _pMaskingTexture)
{
	m_pCombineFilter->MaskingLUT(_bMasking, _pMaskingTexture);
}

void CPostprocess::RenderImageFilter(std::shared_ptr<class CImageFilter> _pImageFilter, _uint _iPassIndex, function<HRESULT(shared_ptr<CShader>)> _fpListener)
{
	_pImageFilter->Render();

	if (_fpListener)
	{
		_fpListener(m_pPostprocessShader);
	}

	m_pPostprocessShader->BeginPass(_iPassIndex);
	m_pContext->DrawIndexed(6, 0, 0);
}

void CPostprocess::Create_VIBuffer()
{
	auto [vecVertices, vecIndices] = CGeometryGenerator::Create_Rect();

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof bufferDesc);

	bufferDesc.ByteWidth = _uint(sizeof(VTXPOSTEX) * vecVertices.size());
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(VTXPOSTEX);

	D3D11_SUBRESOURCE_DATA subResourceData;
	ZeroMemory(&subResourceData, sizeof subResourceData);
	subResourceData.pSysMem = vecVertices.data();

	if (FAILED(m_pDevice->CreateBuffer(&bufferDesc, &subResourceData, m_pVertexBuffer.GetAddressOf())))
		assert(false && "Failed to create vertex buffer");

	bufferDesc.ByteWidth = _uint(sizeof(_ushort) * vecIndices.size());
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(_ushort);

	ZeroMemory(&subResourceData, sizeof subResourceData);
	subResourceData.pSysMem = vecIndices.data();

	if (FAILED(m_pDevice->CreateBuffer(&bufferDesc, &subResourceData, m_pIndexBuffer.GetAddressOf())))
		assert(false && "Failed to create index buffer");
}

void CPostprocess::Create_ImageBuffer(_uint _iWidth, _uint _iHeight, ComPtr<ID3D11ShaderResourceView>& _pShaderResourceView, ComPtr<ID3D11RenderTargetView>& _pRenderTargetView)
{
	ComPtr<ID3D11Texture2D> pTextureBuffer;

	D3D11_TEXTURE2D_DESC tTextureDesc;
	ZeroMemory(&tTextureDesc, sizeof(tTextureDesc));
	tTextureDesc.Width = _iWidth;
	tTextureDesc.Height = _iHeight;
	tTextureDesc.MipLevels = tTextureDesc.ArraySize = 1;
	tTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	tTextureDesc.SampleDesc.Count = 1;
	tTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	tTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tTextureDesc.MiscFlags = 0;
	tTextureDesc.CPUAccessFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&tTextureDesc, nullptr, pTextureBuffer.GetAddressOf())))
		assert(false && "Failed To Create Texture2D");
	if (FAILED(m_pDevice->CreateRenderTargetView(pTextureBuffer.Get(), nullptr, _pRenderTargetView.GetAddressOf())))
		assert(false && "Failed To Create RenderTargetView");
	if (FAILED(m_pDevice->CreateShaderResourceView(pTextureBuffer.Get(), nullptr, _pShaderResourceView.GetAddressOf())))
		assert(false && "Failed To Create ShaderResourceView");
}

void CPostprocess::Redirect_FilterTargets()
{
	if (m_bEnableFXAAFilter && m_bEnableDOFFilter)
	{
		m_pCombineFilter->SetRenderTargets({ m_pBufferRenderTargetView });

		m_pFXAAFilter->SetShaderResources({m_pBufferShaderResourceView});
		m_pFXAAFilter->SetRenderTargets({ m_pFXAARenderTargetView });

		m_pDOFFilter->SetShaderResources({ m_pFXAAShaderResourceView, m_pBloomLevel1ShaderResourceView });
		m_pDOFFilter->SetRenderTargets({ m_pBackbufferRenderTargetView });
	}
	else if (false == m_bEnableFXAAFilter && m_bEnableDOFFilter)
	{
		m_pCombineFilter->SetRenderTargets({ m_pBufferRenderTargetView });

		m_pDOFFilter->SetShaderResources({ m_pBufferShaderResourceView, m_pBloomLevel1ShaderResourceView });
		m_pDOFFilter->SetRenderTargets({ m_pBackbufferRenderTargetView });
	}
	else if (m_bEnableFXAAFilter && false == m_bEnableDOFFilter)
	{
		m_pCombineFilter->SetRenderTargets({ m_pBufferRenderTargetView });

		m_pFXAAFilter->SetShaderResources({ m_pBufferShaderResourceView });
		m_pFXAAFilter->SetRenderTargets({ m_pBackbufferRenderTargetView });
	}
	else
	{
		m_pCombineFilter->SetRenderTargets({ m_pBackbufferRenderTargetView });
	}
}
