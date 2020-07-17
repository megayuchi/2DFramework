#pragma once

#include <d3d11.h>
#include "math.inl"

class CRenderTexture;

HRESULT CompileShaderFromFile(char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, DWORD ShaderParams, SYSTEMTIME* pOutLastWriteTime);

void ConvertSceenCoord2DTo3D(VECTOR3* pv3OutPoint, const VECTOR2* pv2Point, float rcp_width, float rcp_height);
void SetDefaultReasterizeValue(D3D11_RASTERIZER_DESC* pOutDesc);
void SetDefaultBlendValue(D3D11_BLEND_DESC* pOutDesc);
void SetBlendValueColorWriteDisable(D3D11_BLEND_DESC* pOutDesc);
void SetDefaultSamplerValue(D3D11_SAMPLER_DESC* pOutDesc);
void SetDefaultDepthStencilValue(D3D11_DEPTH_STENCIL_DESC* pOutDesc);
void SetHFieldAlphaTexMatrix(MATRIX4* pMat, VECTOR3* pv3Rect);
void SetHFieldTileTexMatrix(MATRIX4* pMatQuad, VECTOR3* pv3Rect, float fFaceSize, DWORD dwWidthHeight);
void OutputD3DErrorMsg(HRESULT hr, WCHAR* wchMsg);


