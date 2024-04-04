#include "ComputeBuffer.h"
#include "Shader.h"

CComputeBuffer::CComputeBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent(pDevice, pContext)
{
}

CComputeBuffer::CComputeBuffer(const CComputeBuffer& rhs)
    : CComponent(rhs)
	, m_iNumInputData(rhs.m_iNumInputData)
    , m_iNumOutputData(rhs.m_iNumOutputData)
    , m_pInputData(rhs.m_pInputData)
{
    for (_uint i = 0; i < m_iNumInputData; ++i)
    {
        m_pInputBuffer[i] = rhs.m_pInputBuffer[i];
        m_pSRV[i] = rhs.m_pSRV[i];
        m_iInputStride[i] = rhs.m_iInputStride[i];
        m_iNumInputElements[i] = rhs.m_iNumInputElements[i];
        
        Safe_AddRef(m_pInputBuffer[i]);
        Safe_AddRef(m_pSRV[i]);
    }

    for (_uint i = 0; i < m_iNumOutputData; ++i)
    {
        m_pOutputBuffer[i] = rhs.m_pOutputBuffer[i];
        m_pUAV[i] = rhs.m_pUAV[i];
		m_pResultBuffer[i] = rhs.m_pResultBuffer[i];
		m_iOutputStride[i] = rhs.m_iOutputStride[i];
		m_iNumOutputElements[i] = rhs.m_iNumOutputElements[i];

		m_pResultInputBuffer[i] = rhs.m_pResultInputBuffer[i];
		m_pResultSRV[i] = rhs.m_pResultSRV[i];
        
        Safe_AddRef(m_pOutputBuffer[i]);
        Safe_AddRef(m_pResultBuffer[i]);
        Safe_AddRef(m_pUAV[i]);

        Safe_AddRef(m_pResultInputBuffer[i]);
        Safe_AddRef(m_pResultSRV[i]);
    }
}

HRESULT CComputeBuffer::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CComputeBuffer::Initialize(void* pArg)
{
	COMPUTESHADERDESC* Desc = (COMPUTESHADERDESC*)pArg;
    m_iNumInputData = Desc->iNumInputData;
    for (_uint i = 0; i < m_iNumInputData; ++i)
    {
        m_iInputStride[i] = Desc->iInputStride[i];
        m_iNumInputElements[i] = Desc->iNumInputElements[i];
    }

    m_iNumOutputData = Desc->iNumOutputData;
    for (_uint i = 0; i < m_iNumOutputData; ++i)
    {
        m_iOutputStride[i] = Desc->iOutputStride[i];
        m_iNumOutputElements[i] = Desc->iNumOutputElements[i];
        m_bIsStructuredBuffer[i] = Desc->bIsStructuredBuffer[i];
    }

	
	m_pInputData = Desc->pInputData;
    
    for (_uint i = 0; i < m_iNumInputData; ++i)
    {
        // Create Input Buffer
        {
            ID3D11Buffer* pBuffer = nullptr;

            D3D11_BUFFER_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

            desc.ByteWidth = m_iInputStride[i] * m_iNumInputElements[i];
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            D3D11_SUBRESOURCE_DATA subResource = { 0 };
            subResource.pSysMem = m_pInputData[i];

            if (FAILED(m_pDevice->CreateBuffer(&desc, &subResource, &pBuffer)))
                return E_FAIL;

            m_pInputBuffer[i] = (ID3D11Resource*)pBuffer;
        }
        // Create Shader Resource View
        {
            ID3D11Buffer* buffer = (ID3D11Buffer*)m_pInputBuffer[i];

            D3D11_BUFFER_DESC desc;
            buffer->GetDesc(&desc);

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
            srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
            srvDesc.BufferEx.NumElements = desc.ByteWidth / 4;

            if (FAILED(m_pDevice->CreateShaderResourceView(buffer, &srvDesc, &m_pSRV[i])))
                return E_FAIL;
        }
    }

    for (_uint i = 0; i < m_iNumOutputData; ++i)
    {
        // Create Output Buffer
        {
            ID3D11Buffer* pBuffer = NULL;

            D3D11_BUFFER_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

            desc.ByteWidth = m_iOutputStride[i] * m_iNumOutputElements[i];
            desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
            if (m_bIsStructuredBuffer[i])
            {
                desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
                desc.StructureByteStride = m_iOutputStride[i];
            }
            else
            {
                desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
            }

            if (FAILED(m_pDevice->CreateBuffer(&desc, NULL, &pBuffer)))
                return E_FAIL;

            m_pOutputBuffer[i] = (ID3D11Resource*)pBuffer;
        }
        // Create Unordered Access View
        {
            ID3D11Buffer* buffer = (ID3D11Buffer*)m_pOutputBuffer[i];

            D3D11_BUFFER_DESC desc;
            buffer->GetDesc(&desc);

            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
            if (m_bIsStructuredBuffer[i])
            {
                uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
                uavDesc.Buffer.NumElements = m_iNumOutputElements[i];
            }
            else
            {
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
                uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
                uavDesc.Buffer.NumElements = desc.ByteWidth / 4;
            }

            if (FAILED(m_pDevice->CreateUnorderedAccessView(buffer, &uavDesc, &m_pUAV[i])))
                return E_FAIL;
        }
        
        // Create Result Buffer
        {
            ID3D11Buffer* buffer;

            D3D11_BUFFER_DESC desc;
            
            ((ID3D11Buffer*)m_pOutputBuffer[i])->GetDesc(&desc);
            desc.Usage = D3D11_USAGE_STAGING;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.BindFlags = D3D11_USAGE_DEFAULT;
            desc.MiscFlags = 0;

            if (FAILED(m_pDevice->CreateBuffer(&desc, NULL, &buffer)))
                return E_FAIL;

            m_pResultBuffer[i] = (ID3D11Resource*)buffer;
        }

        // Create Result Input Buffer
        {
            if ((i == 0))
            {
                continue;
            }
            ID3D11Buffer* pBuffer = nullptr;

            D3D11_BUFFER_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

            desc.ByteWidth = m_iOutputStride[i] * m_iNumOutputElements[i];
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.CPUAccessFlags = 0;
            desc.StructureByteStride = m_iOutputStride[i];

            D3D11_SUBRESOURCE_DATA subResource = { 0 };
            subResource.pSysMem = nullptr;

            if (FAILED(m_pDevice->CreateBuffer(&desc, NULL, &pBuffer)))
                return E_FAIL;

            m_pResultInputBuffer[i] = (ID3D11Resource*)pBuffer;
        }
        // Create Shader Resource View
        {
            ID3D11Buffer* buffer = (ID3D11Buffer*)m_pResultInputBuffer[i];

            D3D11_BUFFER_DESC desc;
            buffer->GetDesc(&desc);

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
            srvDesc.BufferEx.NumElements = m_iNumOutputElements[i];

            if (FAILED(m_pDevice->CreateShaderResourceView(buffer, &srvDesc, &m_pResultSRV[i])))
                return E_FAIL;
        }
    }
    
    return S_OK;
}

HRESULT CComputeBuffer::Bind_ShaderResourceView(CShader* pShader, const char* pConstantName, const _uint iResourceIndex)
{
    return pShader->Bind_ShaderResourceView(pConstantName, m_pSRV[iResourceIndex]);
}

HRESULT CComputeBuffer::Bind_UnorderedAccessView(CShader* pShader, const char* pConstantName, const _uint iIndex)
{
    return pShader->Bind_UnorderedAccessView(pConstantName, m_pUAV[iIndex]);
}

HRESULT CComputeBuffer::Bind_Result_ShaderResourceView(CShader* pShader, const char* pConstantName, const _uint iIndex)
{
    return pShader->Bind_ShaderResourceView(pConstantName, m_pResultSRV[iIndex]);
}

HRESULT CComputeBuffer::Dispatch(class CShader* pShader, _uint iTechnique, _uint iPass, _uint iX, _uint iY, _uint iZ)
{
    return pShader->Dispatch(iTechnique, iPass, iX, iY, iZ);
}

void CComputeBuffer::StoreOutput(void* pData, const _uint iIndex)
{
    m_pContext->CopyResource(m_pResultBuffer[iIndex], m_pOutputBuffer[iIndex]);

    D3D11_MAPPED_SUBRESOURCE subResource;
    m_pContext->Map(m_pResultBuffer[iIndex], 0, D3D11_MAP_READ, 0, &subResource);
    {
        if (subResource.pData != nullptr)
            memcpy(pData, subResource.pData, m_iOutputStride[iIndex] * m_iNumOutputElements[iIndex]);
    }
    m_pContext->Unmap(m_pResultBuffer[iIndex], 0);
}

HRESULT CComputeBuffer::Update_ShaderResourceView(void* pData, const _uint iResourceIndex)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_pContext->Map(m_pInputBuffer[iResourceIndex], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    memcpy(mappedResource.pData, pData, m_iInputStride[iResourceIndex] * m_iNumInputElements[iResourceIndex]);

    m_pContext->Unmap(m_pInputBuffer[iResourceIndex], 0);
    
    return S_OK;
}

void CComputeBuffer::Copy_OutputToShaderResourceView(const _uint iOutputIndex, const _uint iSRVIndex)
{
    m_pContext->CopyResource(m_pResultInputBuffer[iSRVIndex], m_pOutputBuffer[iOutputIndex]);
}

CComputeBuffer* CComputeBuffer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CComputeBuffer* pInstance = new CComputeBuffer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Created : CComputeBuffer");
    }

    return pInstance;
}

CComponent* CComputeBuffer::Clone(void* pArg)
{
    CComputeBuffer* pInstance = new CComputeBuffer(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Cloned : CComputeBuffer");
    }

    return pInstance;
}

void CComputeBuffer::Free()
{
    __super::Free();

	for (_uint i = 0; i < m_iNumInputData; ++i)
	{
		Safe_Release(m_pInputBuffer[i]);
		Safe_Release(m_pSRV[i]);
	}

    for (_uint i = 0; i < m_iNumOutputData; ++i)
    {
        Safe_Release(m_pOutputBuffer[i]);
        Safe_Release(m_pUAV[i]);

        Safe_Release(m_pResultBuffer[i]);

        Safe_Release(m_pResultInputBuffer[i]);
        Safe_Release(m_pResultSRV[i]);
    }
}
