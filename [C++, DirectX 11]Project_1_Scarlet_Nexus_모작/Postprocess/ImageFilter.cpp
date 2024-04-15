#include "EnginePCH.h"
#include "ImageFilter.h"

#include "Shader.h"
#include "Texture.h"

void CImageFilter::Initialize(const ComPtr<ID3D11Device>& _pDevice, const ComPtr<ID3D11DeviceContext>& _pContext,
	std::shared_ptr<class CShader> _pPixelShader, _int _iWidth, _int _iHeight)
{
	m_pDevice = _pDevice;
	m_pContext = _pContext;

	m_pPixelShader = _pPixelShader;

	ZeroMemory(&m_tViewport, sizeof(D3D11_VIEWPORT));
	m_tViewport.TopLeftX = 0;
	m_tViewport.TopLeftY = 0;
	m_tViewport.Width = static_cast<FLOAT>(_iWidth);
	m_tViewport.Height = static_cast<FLOAT>(_iHeight);
	m_tViewport.MinDepth = 0.0f;
	m_tViewport.MaxDepth = 1.f;

	m_vSampleStep.x = 1.f / _iWidth;
	m_vSampleStep.y = 1.f / _iHeight;
}

void CImageFilter::Render()
{
	assert(m_vecShaderResources.size() > 0);
	assert(m_vecRenderTargets.size() > 0);

	ComPtr<ID3D11ShaderResourceView> pDumpSRV[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {
		nullptr
	};
	m_pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pDumpSRV->GetAddressOf());

	m_pContext->RSSetViewports(1, &m_tViewport);
	m_pContext->OMSetRenderTargets(UINT(m_vecRenderTargets.size()),
		m_vecRenderTargets.data()->GetAddressOf(), nullptr);

	m_pPixelShader->Bind_RawValue("g_SampleStep", &m_vSampleStep.x, sizeof(_float2));

	m_pPixelShader->Bind_ShaderResourceViews("g_texture0", m_vecShaderResources[0].GetAddressOf());
	if (m_vecShaderResources.size() > 1)
	{
		m_pPixelShader->Bind_ShaderResourceViews("g_texture1", m_vecShaderResources[1].GetAddressOf());
	}
	
	if (m_bExecuteLUT)
	{
		m_vecLUTFilters[m_iLUTFilterIndex].second->Bind_ShaderResourceView(m_pPixelShader, aiTextureType_DIFFUSE, "g_LUTTexture");

		m_pPixelShader->Bind_RawValue("g_bMaskingLUT", &m_bMaskingLUT, sizeof(_bool));
		if (m_bMaskingLUT)
		{
			m_pPixelShader->Bind_ShaderResourceView(SHADER_TEXMASK, m_pMaskingTexture->Get_ShaderResourceView());
		}
	}
}

void CImageFilter::Clear(_float4 _vClearColor)
{
	for (auto pRTV : m_vecRenderTargets)
		m_pContext->ClearRenderTargetView(pRTV.Get(), &_vClearColor.x);
}

void CImageFilter::SetShaderResources(const std::vector<ComPtr<ID3D11ShaderResourceView>>& _vecResources)
{
	m_vecShaderResources.clear();
	for (const auto pResource : _vecResources) {
		m_vecShaderResources.push_back(pResource);
	}
}

void CImageFilter::SetRenderTargets(const std::vector<ComPtr<ID3D11RenderTargetView>>& _vecTargets)
{
	m_vecRenderTargets.clear();
	for (const auto pTarget : _vecTargets) {
		m_vecRenderTargets.push_back(pTarget);
	}
}

void CImageFilter::Bind_RawValue(const char* _pConstantName, const void* _pData, _uint _iByteSize)
{
	m_pPixelShader->Bind_RawValue(_pConstantName, _pData, _iByteSize);
}

void CImageFilter::Bind_LUTTexture(_uint _iLUTFilterIndex)
{
	m_bExecuteLUT = true;
	m_iLUTFilterIndex = _iLUTFilterIndex;
}

void CImageFilter::MaskingLUT(_bool _bMasking, shared_ptr<class CTexture> _pMaskingTexture)
{
	m_bMaskingLUT = _bMasking;
	if (m_bMaskingLUT)
	{
		m_pMaskingTexture = _pMaskingTexture;
	}
	else
	{
		m_pMaskingTexture = _pMaskingTexture;
	}
}

void CImageFilter::Append_LUTTexture(const _wchar* _pLUTTag, shared_ptr<class CTexture> _pLUTTexture)
{
	m_vecLUTFilters.push_back({ _pLUTTag, _pLUTTexture });
}

shared_ptr<CImageFilter> CImageFilter::Create()
{
	shared_ptr<CImageFilter> pInstance = make_private_shared(CImageFilter);
	return pInstance;
}
