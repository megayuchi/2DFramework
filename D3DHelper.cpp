#include "pch.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "D3DHelper.h"


BOOL CreateShaderCodeFromFile(BYTE** ppOutCodeBuffer, DWORD* pdwOutCodeSize, SYSTEMTIME* pOutLastWriteTime, char* szFileName)
{
	BOOL	bResult = FALSE;

	DWORD	dwOpenFlag = OPEN_EXISTING;
	DWORD	dwAccessMode = GENERIC_READ;
	DWORD	dwShare = 0;

	WCHAR	wchTxt[128] = { 0 };

	HANDLE	hFile = CreateFileA(szFileName, dwAccessMode, dwShare, NULL, dwOpenFlag, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		swprintf_s(wchTxt, L"Shader File Not Found : %S\n", szFileName);
		OutputDebugStringW(wchTxt);
		goto lb_return;
	}

	DWORD	dwFileSize = GetFileSize(hFile, NULL);
	if (dwFileSize > 1024 * 1024)
	{
		swprintf_s(wchTxt, L"Invalid Shader File : %S\n", szFileName);
		OutputDebugStringW(wchTxt);
		goto lb_close_return;
	}
	DWORD	dwCodeSize = dwFileSize + 1;

	BYTE*	pCodeBuffer = new BYTE[dwCodeSize];
	memset(pCodeBuffer, 0, dwCodeSize);

	DWORD	dwReadBytes = 0;
	if (!ReadFile(hFile, pCodeBuffer, dwFileSize, &dwReadBytes, NULL))
	{
		swprintf_s(wchTxt, L"Failed to Read File : %S\n", szFileName);
		OutputDebugStringW(wchTxt);
		goto lb_close_return;
	}
	FILETIME	createTime, lastAccessTime, lastWriteTime;

	GetFileTime(hFile, &createTime, &lastAccessTime, &lastWriteTime);

	SYSTEMTIME	sysLastWriteTime;
	FileTimeToSystemTime(&lastWriteTime, &sysLastWriteTime);


	*ppOutCodeBuffer = pCodeBuffer;
	*pdwOutCodeSize = dwCodeSize;
	*pOutLastWriteTime = sysLastWriteTime;
	bResult = TRUE;

lb_close_return:
	CloseHandle(hFile);

lb_return:
	return bResult;

}

void DeleteShaderCode(BYTE* pCodeBuffer)
{
	delete[] pCodeBuffer;
}
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, DWORD ShaderParams, SYSTEMTIME* pOutLastWriteTime)
{
	HRESULT hr = E_FAIL;


	SYSTEMTIME	lastWriteTime;
	BYTE*		pCodeBuffer = NULL;
	DWORD		dwCodeSize = 0;
	if (!CreateShaderCodeFromFile(&pCodeBuffer, &dwCodeSize, &lastWriteTime, szFileName))
	{
		return E_FAIL;
	}

	DWORD	dwOptimizeFlag = D3D10_SHADER_OPTIMIZATION_LEVEL3;
	DWORD	dwDebugFlag = 0;

#if defined( DEBUG ) || defined( _DEBUG )
	dwOptimizeFlag = D3D10_SHADER_SKIP_OPTIMIZATION;
	//dwDebugFlag = D3DCOMPILE_DEBUG;
#endif
	DWORD	dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	dwShaderFlags |= (dwDebugFlag | dwOptimizeFlag);
	D3D_SHADER_MACRO pDefine[] = { 0 };
	/*
	D3D_SHADER_MACRO pDefine[] = {
									"LIGHTING_TYPE","0",						// 0
									"SHADER_PARAMETER_ATT_LIGHT","0",			// 1
									"SHADER_PARAMETER_RECV_SHADOW","0",			// 2
									"SHADER_PARAMETER_PHYSIQUE","0",			// 3
									"SHADER_PARAMETER_LIGHT_PRB","0",			// 4
									NULL,NULL};

	switch(LightingType)
	{
	case LIGHTING_TYPE_DEFAULT:
		pDefine[0].Definition = "0";
		break;
	case LIGHTING_TYPE_PER_VERTEX:
		pDefine[0].Definition = "1";
		break;
	case LIGHTING_TYPE_TOON:
		pDefine[0].Definition = "2";
		break;
	case LIGHTING_TYPE_PER_PIXEL:
		pDefine[0].Definition = "3";
		break;
	case LIGHTING_TYPE_LIGHTMAP:
		pDefine[0].Definition = "4";
		break;
	};

	if (ShaderParams & SHADER_PARAMETER_ATT_LIGHT)
		pDefine[1].Definition = "1";

	if (ShaderParams & SHADER_PARAMETER_RECV_SHADOW)
		pDefine[2].Definition = "1";

	if (ShaderParams & SHADER_PARAMETER_PHYSIQUE)
		pDefine[3].Definition = "1";

	if (ShaderParams & SHADER_PARAMETER_LIGHT_PRB)
		pDefine[4].Definition = "1";

		*/


	ID3DBlob* pErrorBlob = NULL;
	/*
	HRESULT D3DX11CompileFromMemory(
  _In_   LPCSTR pSrcData,
  _In_   SIZE_T SrcDataLen,
  _In_   LPCSTR pFileName,
  _In_   const D3D10_SHADER_MACRO *pDefines,
  _In_   LPD3D10INCLUDE pInclude,
  _In_   LPCSTR pFunctionName,
  _In_   LPCSTR pProfile,
  _In_   UINT Flags1,
  _In_   UINT Flags2,
  _In_   ID3DX11ThreadPump *pPump,
  _Out_  ID3D10Blob **ppShader,
  _Out_  ID3D10Blob **ppErrorMsgs,
  _Out_  HRESULT *pHResult
);
*/


	hr = D3DCompile2(pCodeBuffer, (size_t)dwCodeSize, szFileName, pDefine, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel, dwShaderFlags, 0, 0, NULL, 0, ppBlobOut, &pErrorBlob);
	//hr = D3DX11CompileFromMemory((LPCSTR)pCodeBuffer,dwCodeSize,szFileName,pDefine, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );

	DeleteShaderCode(pCodeBuffer);

	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob)
			pErrorBlob->Release();

		__debugbreak();
		return hr;
	}
	if (pErrorBlob)
		pErrorBlob->Release();

	*pOutLastWriteTime = lastWriteTime;
	hr = S_OK;

	return hr;
}

void SetDefaultReasterizeValue(D3D11_RASTERIZER_DESC* pOutDesc)
{
	pOutDesc->FillMode = D3D11_FILL_SOLID;
	pOutDesc->CullMode = D3D11_CULL_BACK;
	pOutDesc->FrontCounterClockwise = FALSE;
	pOutDesc->DepthBias = 0;
	pOutDesc->DepthBiasClamp = 0.0f;
	pOutDesc->SlopeScaledDepthBias = 0.0f;
	pOutDesc->DepthClipEnable = TRUE;
	pOutDesc->ScissorEnable = FALSE;
	pOutDesc->MultisampleEnable = FALSE;
	pOutDesc->AntialiasedLineEnable = FALSE;

}

void SetDefaultBlendValue(D3D11_BLEND_DESC* pOutDesc)
{
	pOutDesc->AlphaToCoverageEnable = FALSE;
	pOutDesc->IndependentBlendEnable = TRUE;

	for (DWORD i = 0; i < 8; i++)
	{

		pOutDesc->RenderTarget[i].BlendEnable = FALSE;
		pOutDesc->RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		pOutDesc->RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		pOutDesc->RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		pOutDesc->RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
		pOutDesc->RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		pOutDesc->RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		pOutDesc->RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
}
void SetBlendValueColorWriteDisable(D3D11_BLEND_DESC* pOutDesc)
{
	SetDefaultBlendValue(pOutDesc);

	for (DWORD i = 0; i < 8; i++)
	{
		pOutDesc->RenderTarget[i].RenderTargetWriteMask = 0;
	}
}

void SetDefaultSamplerValue(D3D11_SAMPLER_DESC* pOutDesc)
{
	pOutDesc->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	pOutDesc->AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	pOutDesc->AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	pOutDesc->AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	pOutDesc->MipLODBias = -1.0f;
	pOutDesc->MaxAnisotropy = 16;
	pOutDesc->ComparisonFunc = D3D11_COMPARISON_NEVER;
	pOutDesc->BorderColor[0] = 1.0f;
	pOutDesc->BorderColor[1] = 1.0f;
	pOutDesc->BorderColor[2] = 1.0f;
	pOutDesc->BorderColor[3] = 1.0f;
	pOutDesc->MinLOD = -FLT_MAX;
	pOutDesc->MaxLOD = D3D11_FLOAT32_MAX;

}
void SetDefaultDepthStencilValue(D3D11_DEPTH_STENCIL_DESC* pOutDesc)
{
	pOutDesc->DepthEnable = TRUE;
	pOutDesc->DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	pOutDesc->DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	pOutDesc->StencilEnable = FALSE;
	pOutDesc->StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	pOutDesc->StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	pOutDesc->FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	pOutDesc->FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	pOutDesc->FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	pOutDesc->FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;;

	pOutDesc->BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	pOutDesc->BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	pOutDesc->BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	pOutDesc->BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

}





void SetHFieldAlphaTexMatrix(MATRIX4* pMat, VECTOR3* pv3Rect)
{
	// uvw좌표는 각각 다음과 같이 구해진다.
	// u = x-x0 / width
	// v = z-z0 / height

	float	width = pv3Rect[3].x - pv3Rect[0].x;
	float	height = pv3Rect[1].z - pv3Rect[0].z;

	memset(pMat, 0, sizeof(MATRIX4));
	pMat->_11 = 1.0f / width;
	pMat->_32 = 1.0f / height;
	pMat->_41 = -1.0f * pv3Rect[0].x / width;
	pMat->_42 = -1.0f * pv3Rect[0].z / height;
	pMat->_44 = 1.0f;
}

void SetHFieldTileTexMatrix(MATRIX4* pMatQuad, VECTOR3* pv3Rect, float fFaceSize, DWORD dwWidthHeight)
{
	VECTOR3	v3Rect[4];
	memcpy(v3Rect, pv3Rect, sizeof(v3Rect));


	// uvw좌표는 각각 다음과 같이 구해진다.
	// u = x-x0 / fFaceSize
	// v = z-z0 / fFaceSize

	pMatQuad->_11 = 1.0f / fFaceSize;
	pMatQuad->_12 = 0.0f;
	pMatQuad->_13 = 0.0f;
	pMatQuad->_14 = 0.0f;

	pMatQuad->_21 = 0.0f;
	pMatQuad->_22 = 0.0f;
	pMatQuad->_23 = 0.0f;
	pMatQuad->_24 = 0.0f;

	pMatQuad->_31 = 0.0f;
	pMatQuad->_32 = 1.0f / fFaceSize;
	pMatQuad->_33 = 0.0f;
	pMatQuad->_34 = 0.0f;


	pMatQuad->_43 = 0.0f;
	pMatQuad->_44 = 1.0f;

	pMatQuad->_41 = -1.0f * v3Rect[0].x / fFaceSize;
	pMatQuad->_42 = -1.0f * v3Rect[0].z / fFaceSize;
}

/*
D3D11_ERROR_FILE_NOT_FOUND The file was not found.
D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS There are too many unique instances of a particular type of state object.
D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS There are too many unique instance of a particular type of view object.
D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD The first call to ID3D11DeviceContext::Map after either ID3D11Device::CreateDeferredContext or ID3D11DeviceContext::FinishCommandList per Resource was not D3D11_MAP_WRITE_DISCARD.
D3DERR_INVALIDCALL The method call is invalid. For example, a method's parameter may not be a valid pointer.
D3DERR_WASSTILLDRAWING The previous blit operation that is transferring information to or from this surface is incomplete.
E_FAIL Attempted to create a device with the debug layer enabled and the layer is not installed.
E_INVALIDARG An invalid parameter was passed to the returning function.
E_OUTOFMEMORY Direct3D could not allocate sufficient memory to complete the call.
S_FALSE Alternate success value, indicating a successful but nonstandard completion (the precise meaning depends on context).
S_OK
*/
void OutputD3DErrorMsg(HRESULT hr, WCHAR* wchMsg)
{
	WCHAR	txt[512] = { 0 };

	const WCHAR*	errMsg = L"Unknown";
	switch (hr)
	{
		case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
			errMsg = L"D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
			break;

		case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
			errMsg = L"D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";
			break;

		case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
			errMsg = L"D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";
			break;

		case DXGI_DDI_ERR_WASSTILLDRAWING:
			errMsg = L"DXGI_DDI_ERR_WASSTILLDRAWING";
			break;

		case E_FAIL:
			errMsg = L"E_FAIL";
			break;

		case E_INVALIDARG:
			errMsg = L"E_INVALIDARG";
			break;

		case E_OUTOFMEMORY:
			errMsg = L"E_OUTOFMEMORY";
			break;

		case DXGI_ERROR_DEVICE_REMOVED:
			errMsg = L"DXGI_ERROR_DEVICE_REMOVED";
			break;
		case DXGI_ERROR_INVALID_CALL:
			errMsg = L"DXGI_ERROR_INVALID_CALL";
			break;

	};

	swprintf_s(txt, L"%s - %s\n", wchMsg, errMsg);

	OutputDebugStringW(txt);
	__debugbreak();
}




void ConvertSceenCoord2DTo3D(VECTOR3* pv3OutPoint, const VECTOR2* pv2Point, float rcp_width, float rcp_height)
{
	pv3OutPoint->x = ((pv2Point->x * rcp_width) - 0.5f) * 2.0f;
	pv3OutPoint->y = ((1.0f - (pv2Point->y * rcp_height)) - 0.5f) * 2.0f;
	pv3OutPoint->z = 0.0f;
}


