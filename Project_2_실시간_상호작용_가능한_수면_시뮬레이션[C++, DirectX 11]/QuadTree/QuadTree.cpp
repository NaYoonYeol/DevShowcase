#include "QuadTree.h"
#include "Frustom.h"
#include "Frustom.h"

CQuadTree::CQuadTree(_uint iNumGridSizeX, _uint iNumGridSizeY)
	: m_fRadius(0.f)
	, m_bCulled(false)
{
	for (_uint i = 0; i < 4; ++i)
	{
		m_pChilds[i] = nullptr;
	}

	m_iCorners[CORNER_TL] = 0;
	m_iCorners[CORNER_TR] = iNumGridSizeX - 1;
	m_iCorners[CORNER_BL] = iNumGridSizeX * (iNumGridSizeY - 1);
	m_iCorners[CORNER_BR] = iNumGridSizeX * iNumGridSizeY - 1;
	m_iCenter = (m_iCorners[CORNER_TL] + m_iCorners[CORNER_TR] + m_iCorners[CORNER_BL] + m_iCorners[CORNER_BR]) / 4;
}

CQuadTree::CQuadTree(const CQuadTree& rhs)
{
	for (_uint i = 0; i < 4; ++i)
	{
		m_pChilds[i] = nullptr;
		m_iCorners[i] = 0;
	}
}

HRESULT CQuadTree::Build_QuadTree(const std::vector<_float3>& vecVertices)
{
	if (Subdivide())
	{
		// 해당 노드의 경계구의 반지름을 계산.
		_vector vVertex = XMLoadFloat3(&vecVertices[m_iCorners[CORNER_TL]])
			- XMLoadFloat3(&vecVertices[m_iCorners[CORNER_BR]]);
		m_fRadius = XMVectorGetX(XMVector3Length(vVertex)) * 0.5f;

		m_pChilds[CORNER_TL]->Build_QuadTree(vecVertices);
		m_pChilds[CORNER_TR]->Build_QuadTree(vecVertices);
		m_pChilds[CORNER_BL]->Build_QuadTree(vecVertices);
		m_pChilds[CORNER_BR]->Build_QuadTree(vecVertices);
	}
	
	return S_OK;
}

void CQuadTree::Generate_Patch(std::vector<INSTSURFACE>& vecPatch, const std::vector<_float3>& vecVertices)
{
	Frustom_Culling(vecVertices, 0);
	Generate_Patch_Recursive(vecPatch);
}

void CQuadTree::Frustom_Culling(const std::vector<_float3>& vecVertices, _uint iDepth)
{
	QUAD_CULLED eCulled = Check_Frustom(vecVertices);
	switch (eCulled)
	{
	case Engine::CQuadTree::FRUSTUM_OUT:
		m_bCulled = true;
		return;
	case Engine::CQuadTree::FRUSTUM_PARTIALLY_IN:
		m_bCulled = false;
		break;
	case Engine::CQuadTree::FRUSTUM_COMPLETELY_IN:
		m_bCulled = false;
		return;
	case Engine::CQuadTree::FRUSTUM_UNKNOWN:
		break;
	}

	for (_uint i = 0; i < 4; ++i)
	{
		if (nullptr != m_pChilds[i])
			m_pChilds[i]->Frustom_Culling(vecVertices, iDepth + 1);
	}
}

CQuadTree::QUAD_CULLED CQuadTree::Check_Frustom(const std::vector<_float3>& vecVertices)
{
	_vector vCenter = XMLoadFloat3(&vecVertices[m_iCenter]);
	vCenter = XMVectorSetW(vCenter, 1.f);
	_bool bIsInSphere = CFrustom::Get_Instance()->Culling_Bounding_Sphere(vCenter, m_fRadius);
	if (bIsInSphere == false)
		return FRUSTUM_OUT;

	_bool bIsInFrustom = false;
	for (_uint i = 0; i < 4; ++i)
	{
		_vector vCorner = XMLoadFloat3(&vecVertices[m_iCorners[i]]);
		vCorner = XMVectorSetW(vCorner, 1.f);
		bIsInFrustom = CFrustom::Get_Instance()->Culling_Point(vCorner);
		if (bIsInFrustom == false)
			return FRUSTUM_PARTIALLY_IN;
	}

	return FRUSTUM_COMPLETELY_IN;
}

void CQuadTree::Generate_Patch_Recursive(std::vector<INSTSURFACE>& vecPatch)
{
	if (m_bCulled)
	{
		m_bCulled = false;
		return;
	}
	
	// 최소 크기의 Patch일 때
	if ((m_iCorners[CORNER_TR] - m_iCorners[CORNER_TL]) <= MINIMUM_PATCH)
	{
		vecPatch.emplace_back(INSTSURFACE{ _float(m_iCorners[CORNER_TL]) });
	}
	else // 자식 Patch가 더 있을 때
	{
		for (_uint i = 0; i < 4; ++i)
		{
			m_pChilds[i]->Generate_Patch_Recursive(vecPatch);
		}
	}
}

_bool CQuadTree::Subdivide()
{
	_int iTopEdgeCenter = (m_iCorners[CORNER_TL] + m_iCorners[CORNER_TR]) / 2;
	_int iBottomEdgeCenter = (m_iCorners[CORNER_BL] + m_iCorners[CORNER_BR]) / 2;
	_int iLeftEdgeCenter = (m_iCorners[CORNER_TL] + m_iCorners[CORNER_BL]) / 2;
	_int iRightEdgeCenter = (m_iCorners[CORNER_TR] + m_iCorners[CORNER_BR]) / 2;
	_int iCenter = (m_iCorners[CORNER_TL] + m_iCorners[CORNER_TR] + m_iCorners[CORNER_BL] + m_iCorners[CORNER_BR]) / 4;
	
	// 더 이상 분할이 불가능할 때 종료
	if ((m_iCorners[CORNER_TR] - m_iCorners[CORNER_TL]) <= MINIMUM_PATCH)
		return false;

	// 자식 노드 생성
	m_pChilds[CORNER_TL] = Add_Child(m_iCorners[CORNER_TL], iTopEdgeCenter, iLeftEdgeCenter, iCenter);
	m_pChilds[CORNER_TR] = Add_Child(iTopEdgeCenter, m_iCorners[CORNER_TR], iCenter, iRightEdgeCenter);
	m_pChilds[CORNER_BL] = Add_Child(iLeftEdgeCenter, iCenter, m_iCorners[CORNER_BL], iBottomEdgeCenter);
	m_pChilds[CORNER_BR] = Add_Child(iCenter, iRightEdgeCenter, iBottomEdgeCenter, m_iCorners[CORNER_BR]);
	
	return true;
}

_bool CQuadTree::Set_Corners(_int iCornerTL, _int iCornerTR, _int iCornerBL, _int iCornerBR)
{
	m_iCorners[CORNER_TL] = iCornerTL;
	m_iCorners[CORNER_TR] = iCornerTR;
	m_iCorners[CORNER_BL] = iCornerBL;
	m_iCorners[CORNER_BR] = iCornerBR;
	m_iCenter = (m_iCorners[CORNER_TL] + m_iCorners[CORNER_TR] + m_iCorners[CORNER_BL] + m_iCorners[CORNER_BR]) / 4;
	
	return true;
}

CQuadTree* CQuadTree::Add_Child(_int iCornerTL, _int iCornerTR, _int iCornerBL, _int iCornerBR)
{
	CQuadTree* pChild = new CQuadTree(*this);
	pChild->Set_Corners(iCornerTL, iCornerTR, iCornerBL, iCornerBR);
	
	return pChild;
}

CQuadTree* CQuadTree::Create(_uint iNumGridSizeX, _uint iNumGridSizeY)
{
	CQuadTree* pQuadTree = new CQuadTree(iNumGridSizeX, iNumGridSizeY);
	
	return pQuadTree;
}

void CQuadTree::Destroy_QuadTree(CQuadTree* pRoot)
{
	if (nullptr == pRoot)
		return;

	for (_uint i = 0; i < 4; ++i)
	{
		Destroy_QuadTree(pRoot->m_pChilds[i]);
	}

	Safe_Release(pRoot);
}

void CQuadTree::Free()
{
}