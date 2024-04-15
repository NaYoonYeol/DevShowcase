#pragma once
#include "Base.h"

BEGIN(Engine)

constexpr _uint MINIMUM_PATCH = 7;

class ENGINE_DLL CQuadTree final : public CBase
{
private:
	enum			CORNER_TYPE { CORNER_TL, CORNER_TR, CORNER_BL, CORNER_BR };

	enum			QUAD_CULLED {
		FRUSTUM_OUT = 0,
		FRUSTUM_PARTIALLY_IN = 1,
		FRUSTUM_COMPLETELY_IN = 2,
		FRUSTUM_UNKNOWN = -1
	};
	
private:
	CQuadTree(_uint iNumGridSizeX, _uint iNumGridSizeY);
	CQuadTree(const CQuadTree& rhs);
	
	virtual ~CQuadTree() = default;
	
public:
	HRESULT Build_QuadTree(const std::vector<_float3>& vecVertices);
	void Generate_Patch(std::vector<INSTSURFACE>& vecPatch, const std::vector<_float3>& vecVertices);
	
private:
	void Frustom_Culling(const std::vector<_float3>& vecVertices, _uint iDepth);
	QUAD_CULLED Check_Frustom(const std::vector<_float3>& vecVertices);
	void Generate_Patch_Recursive(std::vector<INSTSURFACE>& vecPatch);
	
private:
	_bool Subdivide();
	_bool Set_Corners(_int iCornerTL, _int iCornerTR, _int iCornerBL, _int iCornerBR);
	
private:
	CQuadTree* Add_Child(_int iCornerTL, _int iCornerTR, _int iCornerBL, _int iCornerBR);

private:
	CQuadTree* m_pChilds[4];
	
	_uint m_iCenter = { 0 };
	_uint m_iCorners[4];

	_bool m_bCulled = { false };
	_float m_fRadius = { 0.f };

public:
	static CQuadTree* Create(_uint iNumGridSizeX, _uint iNumGridSizeY);
	static void Destroy_QuadTree(CQuadTree* pRoot);
	virtual void Free() override;
};

END