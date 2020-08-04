#include "pch.h"
#include <d3d11.h>
#include "d3d_type.h"
#include "D3DHelper.h"
#include "D3DRenderer.h"
#include "DirectXTex/DirectXTex.h"
#include "Util.h"

using namespace DirectX;

#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "DXGI.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "D3DCompiler.lib" )

BOOL DecompressDXT1ToRGBA(const uint8_t* pCompressedImage, int iWidth, int iHeight, uint8_t* pDestBits, size_t DestPitch);
BOOL DecompressDXT3ToRGBA(const uint8_t* pCompressedImage, int iWidth, int iHeight, uint8_t* pDestBits, size_t DestPitch);
BOOL DecompressDXT5ToRGBA(const uint8_t* pCompressedImage, int iWidth, int iHeight, uint8_t* pDestBits, size_t DestPitch);
BOOL DecompressDXTtoRGBA(const uint8_t* pCompressedImage, size_t CompressedImagePitch, int iWidth, int iHeight, DXGI_FORMAT srcFormat, uint8_t* pDestBits, size_t DestPitch);

CD3DRenderer::CD3DRenderer()
{

}

BOOL CD3DRenderer::Initialize(HWND hWnd, IMAGE_FORMAT fmt, const WCHAR* wchShaderPath, DWORD dwFlags)
{
	BOOL	bResult = FALSE;

	HRESULT hr = S_OK;

	m_hWnd = hWnd;
	m_dwCreateFlags = dwFlags;
	m_ImageFormat = fmt;

	WCHAR	wchOldPath[_MAX_PATH] = {};
	GetCurrentDirectory(_MAX_PATH, wchOldPath);

	SetCurrentDirectory(wchShaderPath);
	GetCurrentDirectory((DWORD)_countof(m_wchShaderPath), m_wchShaderPath);

	SetCurrentDirectory(wchOldPath);

	UINT createDeviceFlags = 0;
#ifdef _USE_DEBUG_DEVICE
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;

	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;	// 0 == no limit or 60 
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;


	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
										   D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pD3DDevice, &m_FeatureLevel, &m_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}


	if (FAILED(hr))
		goto lb_return;

	//DisableAltEnter();

	//ref_count = GetRefCount(m_pD3DDevice);

	DXGI_SWAP_CHAIN_DESC desc;
	m_pSwapChain->GetDesc(&desc);


	if (!desc.Windowed)
	{
		SetWindowPos(hWnd, HWND_TOP, 0, 0, desc.BufferDesc.Width, desc.BufferDesc.Height, 0);
		SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE);
		SetMenu(hWnd, NULL);
	}

	RECT	rect;
	::GetClientRect(hWnd, &rect);

	UINT uiWidth = rect.right - rect.left;
	UINT uiHeight = rect.bottom - rect.top;



	if (!CreateBackBuffer(uiWidth, uiHeight))
		goto lb_return;


	D3D11_DEPTH_STENCIL_DESC	depthDesc;
	SetDefaultDepthStencilValue(&depthDesc);


	struct DEPTH_TYPE
	{
		BOOL	bDisableWrite;
		BOOL	bDisableTest;
	};

	DEPTH_TYPE	depthTypeList[MAX_DEPTH_TYPE_NUM] =
	{
		FALSE,FALSE,
		TRUE,FALSE,
		TRUE,TRUE,
		FALSE,TRUE
	};
	for (DWORD i = 0; i < MAX_DEPTH_TYPE_NUM; i++)
	{
		if (depthTypeList[i].bDisableWrite)
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		else
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;


		if (depthTypeList[i].bDisableTest)
			depthDesc.DepthEnable = FALSE;
		else
			depthDesc.DepthEnable = TRUE;



		DWORD	dwIndex = GetDepthTypeIndex(depthTypeList[i].bDisableWrite, depthTypeList[i].bDisableTest);

		hr = m_pD3DDevice->CreateDepthStencilState(&depthDesc, m_ppDepthStencilState + dwIndex);
		if (FAILED(hr))
			__debugbreak();
	}

	m_pImmediateContext->OMSetDepthStencilState(m_ppDepthStencilState[0], 0);

	CreateStates();

#ifdef _DEBUG
	SetD3DDebugSetting();
#endif

	InitBuffer();
	InitShader();

	bResult = TRUE;
lb_return:

	return bResult;

}

BOOL CD3DRenderer::CreateBackBuffer(UINT uiWidth, UINT uiHeight)
{
	BOOL	bResult = FALSE;

	// Create a render target view
	HRESULT	hr = S_FALSE;

	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_pBackBuffer);
	if (FAILED(hr))
		goto lb_return;

	hr = m_pD3DDevice->CreateRenderTargetView(m_pBackBuffer, NULL, &m_pDiffuseRTV);

	if (FAILED(hr))
		goto lb_return;



	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = uiWidth;
	desc.Height = uiHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	hr = m_pD3DDevice->CreateTexture2D(&desc, NULL, &m_pDepthStencil);
	if (FAILED(hr))
		goto lb_return;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = desc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = m_pD3DDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDSV);
	if (FAILED(hr))
		goto lb_return;

	m_pImmediateContext->OMSetRenderTargets(1, &m_pDiffuseRTV, m_pDSV);

	float	rgba[4] = { 0.0f,0.0f,0.0f,1.0f };

	m_pImmediateContext->ClearRenderTargetView(m_pDiffuseRTV, rgba);


	m_pImmediateContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_dwWidth = uiWidth;
	m_dwHeight = uiHeight;


	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)uiWidth;
	vp.Height = (FLOAT)uiHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	m_vp = vp;
	m_pImmediateContext->RSSetViewports(1, &m_vp);

	bResult = TRUE;

lb_return:
	return bResult;

}

void CD3DRenderer::CreateStates()
{
	HRESULT		hr;
	// 레스터라이즈 상태
	D3D11_RASTERIZER_DESC rasterDesc;
	SetDefaultReasterizeValue(&rasterDesc);

	struct RASTER_TYPE
	{
		D3D11_CULL_MODE cullMode;
		D3D11_FILL_MODE fillMode;
	};
	RASTER_TYPE	rasterTypelist[] =
	{
		D3D11_CULL_BACK , D3D11_FILL_SOLID,
		D3D11_CULL_BACK , D3D11_FILL_WIREFRAME,
		D3D11_CULL_NONE , D3D11_FILL_SOLID,
		D3D11_CULL_NONE , D3D11_FILL_WIREFRAME,
		D3D11_CULL_FRONT , D3D11_FILL_SOLID,
		D3D11_CULL_FRONT , D3D11_FILL_WIREFRAME


	};
	const	DWORD	dwRasterTypeNum = _countof(rasterTypelist);
	for (DWORD i = 0; i < dwRasterTypeNum; i++)
	{
		DWORD	dwIndex = GetRasterTypeIndex(rasterTypelist[i].cullMode, rasterTypelist[i].fillMode);

		rasterDesc.CullMode = rasterTypelist[i].cullMode;
		rasterDesc.FillMode = rasterTypelist[i].fillMode;
		hr = m_pD3DDevice->CreateRasterizerState(&rasterDesc, m_ppRasterizeState + dwIndex);

		if (FAILED(hr))
			__debugbreak();
	}


	// 블랜딩 상태들
	D3D11_BLEND_DESC	blendDesc;
	SetDefaultBlendValue(&blendDesc);

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	hr = m_pD3DDevice->CreateBlendState(&blendDesc, m_ppBlendState + BLEND_TYPE_TRANSP);

	if (FAILED(hr))
		__debugbreak();

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	hr = m_pD3DDevice->CreateBlendState(&blendDesc, m_ppBlendState + BLEND_TYPE_ADD);

	if (FAILED(hr))
		__debugbreak();

	// 컬러출력 안하는 블랜딩 상태
	SetBlendValueColorWriteDisable(&blendDesc);
	hr = m_pD3DDevice->CreateBlendState(&blendDesc, m_ppBlendState + BLEND_TYPE_NO_COLOR);

	if (FAILED(hr))
		__debugbreak();



	// 샘플러 스테이트
	D3D11_SAMPLER_DESC	samplerDesc;
	SetDefaultSamplerValue(&samplerDesc);

	struct SAMPLER_TYPE
	{
		D3D11_TEXTURE_ADDRESS_MODE	Address;
		D3D11_FILTER				Filter;
	};
	SAMPLER_TYPE	samplerTypelist[] =
	{
		D3D11_TEXTURE_ADDRESS_WRAP,D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_WRAP,D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,D3D11_FILTER_ANISOTROPIC,

		D3D11_TEXTURE_ADDRESS_CLAMP,D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_CLAMP,D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_CLAMP,D3D11_FILTER_ANISOTROPIC,

		D3D11_TEXTURE_ADDRESS_MIRROR,D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_MIRROR,D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_MIRROR,D3D11_FILTER_ANISOTROPIC,

		D3D11_TEXTURE_ADDRESS_BORDER,D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_BORDER,D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_BORDER,D3D11_FILTER_ANISOTROPIC
	};

	const	DWORD	dwSamplerTypeNum = _countof(samplerTypelist);

	for (DWORD i = 0; i < dwSamplerTypeNum; i++)
	{
		samplerDesc.AddressU = samplerTypelist[i].Address;
		samplerDesc.AddressV = samplerTypelist[i].Address;
		samplerDesc.AddressW = samplerTypelist[i].Address;
		samplerDesc.Filter = samplerTypelist[i].Filter;

		hr = m_pD3DDevice->CreateSamplerState(&samplerDesc, m_ppSamplerState + i);

		if (FAILED(hr))
			__debugbreak();

	}
	D3D11_SAMPLER_DESC SamDescShad =
	{
		D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,// D3D11_FILTER Filter;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
		0,//FLOAT MipLODBias;
		0,//UINT MaxAnisotropy;
		D3D11_COMPARISON_LESS , //D3D11_COMPARISON_FUNC ComparisonFunc;
		1.0,1.0,1.0,1.0,//FLOAT BorderColor[ 4 ];
		0,//FLOAT MinLOD;
		0//FLOAT MaxLOD;   
	};

}


BOOL CD3DRenderer::SetD3DDebugSetting()
{
	BOOL	bResult = FALSE;
	ID3D11Debug*		pDebug = NULL;
	ID3D11InfoQueue*	pInfoQueue = NULL;

	HRESULT	hr;
	hr = m_pD3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&pDebug);
	if (FAILED(hr))
		goto lb_return;


	hr = pDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&pInfoQueue);
	if (FAILED(hr))
		goto lb_return;

	pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);

	//D3D11 WARNING: ID3D11DeviceContext::DrawIndexed: The Pixel Shader expects a Render Target View bound to slot 1, but none is bound. This is OK, as writes of an unbound Render Target View are discarded. It is also possible the developer knows the data will not be used anyway. This is only a problem if the developer actually intended to bind a Render Target View here. [ EXECUTION WARNING #3146081: DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET]

#ifdef DISABLE_D3D_WARNING
	D3D11_MESSAGE_SEVERITY hide[] =
	{
		D3D11_MESSAGE_SEVERITY_WARNING,
		// Add more message IDs here as needed
	};
	/*
			   Category	D3D11_MESSAGE_CATEGORY_EXECUTION	D3D11_MESSAGE_CATEGORY
		   Severity	D3D11_MESSAGE_SEVERITY_WARNING	D3D11_MESSAGE_SEVERITY
		   ID	3146081	D3D11_MESSAGE_ID
   +		pDescription	0x000000000c31ace0 "ID3D11DeviceContext::DrawIndexed: The Pixel Shader expects a Render Target View bound to slot 1, but none is bound. This is OK, as writes of an unbound Render Target View are discarded. It is also possible the developer knows the data will not be used anyway. This is only a problem if the developer actually intended to bind a Render Target View here."	const char *
		   DescriptionByteLength	353	unsigned __int64

		   */
	D3D11_INFO_QUEUE_FILTER filter;
	memset(&filter, 0, sizeof(filter));
	filter.DenyList.NumSeverities = _countof(hide);
	filter.DenyList.pSeverityList = hide;
	pInfoQueue->AddStorageFilterEntries(&filter);
#endif

lb_return:
	if (pInfoQueue)
	{
		pInfoQueue->Release();
		pInfoQueue = NULL;
	}
	if (pDebug)
	{
		pDebug->Release();
		pDebug = NULL;
	}

	return TRUE;

}

BOOL CD3DRenderer::CreateWritableTexture(DWORD dwWidth, DWORD dwHeight)
{
	BOOL	bResult = FALSE;
	DWORD*	pInitImage = new DWORD[dwWidth * dwHeight];
	memset(pInitImage, 0, sizeof(DWORD) * dwWidth * dwHeight);

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_SUBRESOURCE_DATA	initData;
	initData.pSysMem = pInitImage;
	initData.SysMemPitch = dwWidth * sizeof(DWORD);
	initData.SysMemSlicePitch = 0;



	// YUV Texture
	bResult = CreateShaderResourceViewFromTex2D(&m_pTextureSRV, &m_pTexture, dwWidth, dwHeight, format, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, D3D11_CPU_ACCESS_WRITE, &initData);
	if (bResult)
	{
		m_dwTextureWidth = dwWidth;
		m_dwTextureHeight = dwHeight;
	}


lb_return:
	delete[] pInitImage;
	return bResult;
}

void CD3DRenderer::DeleteWritableTexture()
{
	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = NULL;
	}
	if (m_pTextureSRV)
	{
		m_pTextureSRV->Release();
		m_pTextureSRV = NULL;
	}

}
BOOL CD3DRenderer::CreateShaderResourceViewFromTex2D(ID3D11ShaderResourceView** ppOutSRV, ID3D11Texture2D** ppOutTexture, UINT Width, UINT Height, DXGI_FORMAT Format, D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags, D3D11_SUBRESOURCE_DATA* pInitData)
{
	BOOL	bResult = FALSE;

	ID3D11Device*				pDevice = m_pD3DDevice;
	ID3D11ShaderResourceView*	pTexResource = NULL;


	D3D11_TEXTURE2D_DESC	desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = Width;
	desc.Height = Height;
	desc.ArraySize = 1;
	desc.BindFlags = BindFlags;
	desc.CPUAccessFlags = CPUAccessFlags;
	desc.Format = Format;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MiscFlags = 0;
	desc.Usage = Usage;

	ID3D11Texture2D*	pTex2D = NULL;


	HRESULT hr = pDevice->CreateTexture2D(&desc, pInitData, &pTex2D);
	if (FAILED(hr))
	{
		OutputD3DErrorMsg(hr, L"Fail to CreateShaderResourceViewFromTex2D-CreateTexture2D");
		goto lb_return;
	}

	hr = pDevice->CreateShaderResourceView(pTex2D, NULL, &pTexResource);

	if (FAILED(hr))
	{
		OutputD3DErrorMsg(hr, L"Fail to CreateShaderResourceViewFromTex2D-CreateShaderResourceView");
		pTex2D->Release();
		pTex2D = NULL;

	}
	*ppOutTexture = pTex2D;
	*ppOutSRV = pTexResource;
	bResult = TRUE;

lb_return:
	return bResult;
}

BOOL CD3DRenderer::UpdateWindowSize(UINT uiWidth, UINT uiHeight)
{
	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;

	BOOL	bResult = FALSE;


	if (!(uiWidth * uiHeight))
		goto lb_return;


	if (m_dwWidth == (DWORD)uiWidth && m_dwHeight == (DWORD)uiHeight)
		goto lb_return;



	pDeviceContext->OMSetRenderTargets(0, NULL, NULL);

	if (m_pDSV)
	{
		m_pDSV->Release();
		m_pDSV = NULL;
	}
	if (m_pDepthStencil)
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = NULL;
	}
	if (m_pDiffuseRTV)
	{
		m_pDiffuseRTV->Release();
		m_pDiffuseRTV = NULL;
	}
	if (m_pBackBuffer)
	{
		m_pBackBuffer->Release();
		m_pBackBuffer = NULL;
	}


	DXGI_SWAP_CHAIN_DESC	desc;
	HRESULT	hr = m_pSwapChain->GetDesc(&desc);

	if (FAILED(hr))
		__debugbreak();

	hr = m_pSwapChain->ResizeBuffers(desc.BufferCount, uiWidth, uiHeight, desc.BufferDesc.Format, 0);

	if (FAILED(hr))
		__debugbreak();

	if (!CreateBackBuffer(uiWidth, uiHeight))
		goto lb_return;


	bResult = TRUE;

lb_return:

	return bResult;

}

BOOL CD3DRenderer::InitBuffer()
{
	// create vertex buffer for position

	D3D11_SUBRESOURCE_DATA InitData;
	D3D11_BUFFER_DESC bd;

	HRESULT hr;

	D3DTVERTEX	vertex[4] = {
		0.0f,1.0f,1.0f , 0.0f,1.0f,
		1.0f,1.0f,1.0f , 1.0f,1.0f,
		1.0f,0.0f,1.0f , 1.0f,0.0f,
		0.0f,0.0f,1.0f , 0.0f,0.0f
	};

	ID3D11Device*	pDevice = m_pD3DDevice;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertex;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.ByteWidth = sizeof(D3DTVERTEX) * 4;

	hr = pDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr))
		__debugbreak();

	// create index buffer
	WORD	index[6];
	index[0] = 0;
	index[1] = 3;
	index[2] = 2;

	index[3] = 0;
	index[4] = 2;
	index[5] = 1;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = index;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.ByteWidth = sizeof(WORD) * 6;

	hr = pDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
		__debugbreak();


	// create constant buffer
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.ByteWidth = sizeof(CONSTANT_BUFFER_SPRITE);

	hr = pDevice->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
		__debugbreak();

	return TRUE;
}
void CD3DRenderer::CleanupBuffer()
{
	if (m_pConstantBuffer)
	{
		m_pConstantBuffer->Release();
		m_pConstantBuffer = NULL;
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = NULL;
	}
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = NULL;
	}
}
BOOL CD3DRenderer::InitShader()
{
	BOOL	bResult = FALSE;

	HRESULT	hr = S_OK;

	WCHAR	wchOldPath[_MAX_PATH] = {};
	GetCurrentDirectory(_MAX_PATH, wchOldPath);

	SetCurrentDirectory(m_wchShaderPath);
	m_pVS = CreateShader("sh_sprite.hlsl", "vsDefault", SHADER_TYPE_VERTEX_SHADER, 0);
	m_pPS_RGBA = CreateShader("sh_sprite.hlsl", "psRGBA", SHADER_TYPE_PIXEL_SHADER, 0);
	m_pPS_YUV = CreateShader("sh_sprite.hlsl", "psYUV", SHADER_TYPE_PIXEL_SHADER, 0);

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layoutSprite[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElementsSprite = ARRAYSIZE(layoutSprite);

	void*	pCodeBuffer = m_pVS->pCodeBuffer;
	DWORD	dwCodeSize = m_pVS->dwCodeSize;

	ID3D11Device*		pDevice = m_pD3DDevice;

	hr = pDevice->CreateInputLayout(layoutSprite, numElementsSprite, pCodeBuffer, dwCodeSize, &m_pVertexLayout);
	if (FAILED(hr))
		__debugbreak();

	SetCurrentDirectory(wchOldPath);

	bResult = TRUE;

lb_return:
	return bResult;
}

void CD3DRenderer::CleanupShader()
{
	if (m_pVS)
	{
		ReleaseShader(m_pVS);
		m_pVS = NULL;
	}
	if (m_pPS_RGBA)
	{
		ReleaseShader(m_pPS_RGBA);
		m_pPS_RGBA = NULL;
	}
	if (m_pPS_YUV)
	{
		ReleaseShader(m_pPS_YUV);
		m_pPS_YUV = NULL;
	}
	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();
		m_pVertexLayout = NULL;
	}
}

SHADER_HANDLE* CD3DRenderer::CreateShader(char* szShaderFileName, char* szEntryName, SHADER_TYPE ShaderType, DWORD ShaderParams)
{
	BOOL				bResult = FALSE;

	SYSTEMTIME	CreationTime = { 0 };
	ID3D11Device*		pDevice = m_pD3DDevice;
	SHADER_HANDLE*		pNewShaderHandle = NULL;

	char	szPureFileName[_MAX_PATH];

	char	szShaderName[MAX_SHADER_NAME_BUFFER_LEN];

	DWORD	dwPureFileNameLen = GetNameRemovePath(szPureFileName, szShaderFileName);

	if (!dwPureFileNameLen)
		goto lb_return;

	CharToSmallASCII(szPureFileName, szPureFileName, dwPureFileNameLen);

	// 쉐이더 네임
	DWORD	dwShaderNameLen = sprintf_s(szShaderName, _countof(szShaderName), "%s_%s", szPureFileName, szEntryName);

	IUnknown*	pD3DShader = NULL;
	ID3DBlob*	pBlob = NULL;

	char*	szVSTarget = "vs_4_0";
	char*	szPSTarget = "ps_4_0";

	if (m_FeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		szVSTarget = "vs_4_0_level_9_1";
		szPSTarget = "ps_4_0_level_9_1";

		if (ShaderType > SHADER_TYPE_PIXEL_SHADER)
			goto lb_return;
	}

	switch (ShaderType)
	{
		case SHADER_TYPE_VERTEX_SHADER:
			{
				HRESULT	hr = CompileShaderFromFile(szShaderFileName, szEntryName, "vs_5_0", &pBlob, ShaderParams, &CreationTime);
				if (FAILED(hr))
				{
					OutputFailToLoadShader(szShaderName);
					goto lb_exit;
				}

				// Create the vertex shader
				hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, (ID3D11VertexShader**)&pD3DShader);
				if (FAILED(hr))
				{
					goto lb_exit;
				}
			}
			break;
		case SHADER_TYPE_PIXEL_SHADER:
			{
				HRESULT hr = CompileShaderFromFile(szShaderFileName, szEntryName, "ps_5_0", &pBlob, ShaderParams, &CreationTime);
				if (FAILED(hr))
				{
					OutputFailToLoadShader(szShaderName);
					goto lb_exit;
				}
				hr = pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, (ID3D11PixelShader**)&pD3DShader);
				if (FAILED(hr))
				{
					goto lb_exit;
				}
			}
			break;
		case SHADER_TYPE_HULL_SHADER:
			{
				HRESULT hr = CompileShaderFromFile(szShaderFileName, szEntryName, "hs_5_0", &pBlob, ShaderParams, &CreationTime);
				if (FAILED(hr))
				{
					OutputFailToLoadShader(szShaderName);
					goto lb_exit;
				}
				hr = pDevice->CreateHullShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, (ID3D11HullShader**)&pD3DShader);
				if (FAILED(hr))
				{
					goto lb_exit;
				}
			}
			break;

		case SHADER_TYPE_DOMAIN_SHADER:
			{
				HRESULT hr = CompileShaderFromFile(szShaderFileName, szEntryName, "ds_5_0", &pBlob, ShaderParams, &CreationTime);
				if (FAILED(hr))
				{
					OutputFailToLoadShader(szShaderName);
					goto lb_exit;
				}
				hr = pDevice->CreateDomainShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, (ID3D11DomainShader**)&pD3DShader);
				if (FAILED(hr))
				{
					goto lb_exit;
				}
			}
			break;

		case SHADER_TYPE_GEOMETRY_SHADER:
			{
				HRESULT hr = CompileShaderFromFile(szShaderFileName, szEntryName, "gs_4_0", &pBlob, ShaderParams, &CreationTime);
				if (FAILED(hr))
				{
					OutputFailToLoadShader(szShaderName);
					goto lb_exit;
				}
				hr = pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, (ID3D11GeometryShader**)&pD3DShader);
				if (FAILED(hr))
				{
					goto lb_exit;
				}
			}
			break;
		case SHADER_TYPE_COMPUTE_SHADER:
			{
				HRESULT hr = CompileShaderFromFile(szShaderFileName, szEntryName, "cs_5_0", &pBlob, ShaderParams, &CreationTime);
				if (FAILED(hr))
				{
					OutputFailToLoadShader(szShaderName);
					goto lb_exit;
				}
				hr = pDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, (ID3D11ComputeShader**)&pD3DShader);
				if (FAILED(hr))
				{
					goto lb_exit;
				}
			}
	}
	DWORD	dwCodeSize = (DWORD)pBlob->GetBufferSize();
	char*	pCodeBuffer = (char*)pBlob->GetBufferPointer();


	// 중복되는 쉐이더코드는 없다. 따라서 새로 생성한다.

	pNewShaderHandle = CreateShaderHandle(szShaderName, dwShaderNameLen, szPureFileName, (DWORD)strlen(szPureFileName), &CreationTime, pCodeBuffer, dwCodeSize, ShaderType);

	if (pD3DShader)
	{
		pD3DShader->AddRef();
		pNewShaderHandle->pD3DShader = pD3DShader;
	}

lb_regist:
	pNewShaderHandle->dwRefCount++;

	bResult = TRUE;


lb_exit:
	if (pD3DShader)
	{
		pD3DShader->Release();
		pD3DShader = NULL;
	}
	if (pBlob)
	{
		pBlob->Release();
		pBlob = NULL;
	}


lb_return:
	return pNewShaderHandle;
}

SHADER_HANDLE* CD3DRenderer::CreateShaderHandle(char* szShaderName, DWORD dwShaderNameLen, char* szShaderFileName, DWORD dwShaderFileNameLen, SYSTEMTIME* pCreationTime, void* pCodeBuffer, DWORD dwCodeSize, SHADER_TYPE ShaderType)
{
	DWORD	ShaderHandleSize = sizeof(SHADER_HANDLE) - sizeof(DWORD) + dwCodeSize;
	SHADER_HANDLE*	pNewShaderHandle = (SHADER_HANDLE*)malloc(ShaderHandleSize);
	memset(pNewShaderHandle, 0, ShaderHandleSize);

	// Shader Name
	memcpy(pNewShaderHandle->szShaderName, szShaderName, dwShaderNameLen);
	pNewShaderHandle->dwShaderNameLen = dwShaderNameLen;

	// Shader File Name
	memcpy(pNewShaderHandle->szShaderFileName, szShaderFileName, dwShaderFileNameLen);
	pNewShaderHandle->dwShaderFileNameLen = dwShaderFileNameLen;

	if (!memcmp(pNewShaderHandle->szShaderFileName, "shader", 6))
		__debugbreak();

	// Creation Time
	pNewShaderHandle->CreationTime = *pCreationTime;


	pNewShaderHandle->dwCodeSize = dwCodeSize;
	memcpy(pNewShaderHandle->pCodeBuffer, pCodeBuffer, dwCodeSize);
	pNewShaderHandle->ShaderType = ShaderType;

	return pNewShaderHandle;
}
void CD3DRenderer::ReleaseShader(SHADER_HANDLE* pShaderHandle)
{
	pShaderHandle->dwRefCount--;

	if (pShaderHandle->dwRefCount)
		return;

	if (pShaderHandle->pD3DShader)
	{
		pShaderHandle->pD3DShader->Release();
		pShaderHandle->pD3DShader = NULL;
	}

	free(pShaderHandle);
}

void CD3DRenderer::BeginRender(DWORD dwColor, DWORD dwFlags)
{
	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;

	COLOR_VALUE	backColor;
	backColor.r = (float)((dwColor & 0x00ff0000) >> 16) / 255.0f;	// R
	backColor.g = (float)((dwColor & 0x0000ff00) >> 8) / 255.0f;	// G
	backColor.b = (float)(dwColor & 0x000000ff) / 255.0f;			// B
	backColor.a = (float)((dwColor & 0xff000000) >> 24) / 255.0f;	// A
	backColor.a = 1.0f;

	// 렌더타겟 설정
	pDeviceContext->OMSetRenderTargets(1, &m_pDiffuseRTV, m_pDSV);
	pDeviceContext->RSSetViewports(1, &m_vp);

	pDeviceContext->ClearRenderTargetView(m_pDiffuseRTV, backColor.rgba);
	pDeviceContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
BOOL CD3DRenderer::Draw(DWORD dwWidth, DWORD dwHeight, DWORD dwPosX, DWORD dwPosY, DWORD dwColor, DWORD dwFlags)
{
	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;


	CONSTANT_BUFFER_SPRITE		constBuffer;
	constBuffer.render_pos_x = (float)dwPosX;
	constBuffer.render_pos_y = (float)dwPosY;
	constBuffer.render_width = (float)dwWidth;
	constBuffer.render_height = (float)dwHeight;
	constBuffer.screen_width = (float)m_dwWidth;
	constBuffer.screen_height = (float)m_dwHeight;
	constBuffer.fAlpha = 1.0f;
	constBuffer.render_z = 0.5f;
	constBuffer.diffuseColor.Set(1.0f, 1.0f, 1.0f, 1.0f);

	//ID3D11ShaderResourceView* pTexResource = m_pTextureSRV;




	ID3D11SamplerState*		pSamplerState = INL_GetSamplerState(SAMPLER_TYPE_WRAP_POINT);
	//pSamplerState = INL_GetSamplerState(SAMPLER_TYPE_WRAP_POINT);

	float	fBlendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	ID3D11BlendState*		pBlendState = m_ppBlendState[BLEND_TYPE_TRANSP];

	SHADER_HANDLE*		pVS = m_pVS;
	SHADER_HANDLE*		pPS = nullptr;
	
	switch (m_ImageFormat)
	{
		case IMAGE_FORMAT_RGBA:
			pPS = m_pPS_RGBA;
			break;
		case IMAGE_FORMAT_YUV:
			pPS = m_pPS_YUV;
			break;
	}

	//if (RENDER_TYPE_SPRITE_GRAY & dwFlags)
	//{
	//	__debugbreak();
	//	pPS = m_pPS_Grey;
	//}

	pDeviceContext->OMSetBlendState(pBlendState, fBlendFactor, 0xffffffff);

	pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &constBuffer, 0, 0);


	// Set the input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	// Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// set vertex buffer

	UINT			Stride = sizeof(D3DTVERTEX);
	UINT			Offset = 0;

	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &Stride, &Offset);

	// set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pDeviceContext->VSSetShader((ID3D11VertexShader*)pVS->pD3DShader, NULL, 0);
	pDeviceContext->PSSetShader((ID3D11PixelShader*)pPS->pD3DShader, NULL, 0);

	pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);



	ID3D11ShaderResourceView* pTexResource = m_pTextureSRV;
	pDeviceContext->PSSetShaderResources(0, 1, &pTexResource);
	pDeviceContext->DrawIndexed(6, 0, 0);


	if (pBlendState)
	{
		pDeviceContext->OMSetBlendState(nullptr, fBlendFactor, 0xffffffff);
	}

	pTexResource = NULL;
	pDeviceContext->PSSetShaderResources(0, 1, &pTexResource);

	return TRUE;
}
void CD3DRenderer::EndRender()
{

	COLOR_VALUE	cvBack;
	cvBack.Set(0.0f, 1.0f, 0.0f, 0.01f);

	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;

}
void CD3DRenderer::Present(HWND hWnd)
{
	UINT uiSyncInterval = 0;


	m_pSwapChain->Present(uiSyncInterval, 0);

}
BOOL CD3DRenderer::UpdateTextureAsRGBA(const BYTE* pBits, DWORD dwWidth, DWORD dwHeight)
{
	if (0 == dwWidth || 0 == dwHeight)
		__debugbreak();

	if (m_dwTextureWidth != dwWidth || m_dwTextureHeight != dwHeight)
	{
		// 텍스쳐 다시 생성
		DeleteWritableTexture();
		CreateWritableTexture(dwWidth, dwHeight);
	}

	ID3D11Device*			pDevice = m_pD3DDevice;
	ID3D11DeviceContext*	pDeviceContext = m_pImmediateContext;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	memset(&mappedResource, 0, sizeof(mappedResource));

	HRESULT hr = pDeviceContext->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
		__debugbreak();

	const BYTE*	pSrc = pBits;
	BYTE*	pDest = (BYTE*)mappedResource.pData;
	//+(mappedResource.RowPitch * y);
	
	for (DWORD y = 0; y < dwHeight; y++)
	{
		memcpy(pDest, pSrc, dwWidth * 4);
		pDest += mappedResource.RowPitch;
		pSrc += (dwWidth*4);
	}
	pDeviceContext->Unmap(m_pTexture, 0);

	return TRUE;

}
BOOL CD3DRenderer::UpdateTextureAsYUV(DWORD dwWidth, DWORD dwHeight, BYTE* pYBuffer, BYTE* pUBuffer, BYTE* pVBuffer, DWORD Stride)
{
	if (0 == dwWidth || 0 == dwHeight)
		__debugbreak();

	if (m_dwTextureWidth != dwWidth || m_dwTextureHeight != dwHeight)
	{
		// 텍스쳐 다시 생성
		DeleteWritableTexture();
		CreateWritableTexture(dwWidth, dwHeight);
	}

	ID3D11Device*			pDevice = m_pD3DDevice;
	ID3D11DeviceContext*	pDeviceContext = m_pImmediateContext;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	memset(&mappedResource, 0, sizeof(mappedResource));

	HRESULT hr = pDeviceContext->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
		__debugbreak();

	const	BYTE*	y_buffer_entry = pYBuffer;
	const	BYTE*	u_buffer_entry = pUBuffer;
	const	BYTE*	v_buffer_entry = pVBuffer;

	DWORD	StrideHalf = Stride / 2;
	for (DWORD y = 0; y < dwHeight; y++)
	{
		y_buffer_entry = pYBuffer + (Stride * y);
		u_buffer_entry = pUBuffer + (StrideHalf * (y >> 1));
		v_buffer_entry = pVBuffer + (StrideHalf * (y >> 1));

		BYTE*		pDestEntry = (BYTE*)mappedResource.pData + (mappedResource.RowPitch * y);

		for (DWORD x = 0; x < dwWidth; x++)
		{
			pDestEntry[0] = *y_buffer_entry;
			pDestEntry[1] = *u_buffer_entry;
			pDestEntry[2] = *v_buffer_entry;
			pDestEntry[3] = 0xff;

			y_buffer_entry++;

			DWORD	uv_inc = x & 0x00000001;
			u_buffer_entry += uv_inc;
			v_buffer_entry += uv_inc;

			pDestEntry += 4;
		}
	}
	pDeviceContext->Unmap(m_pTexture, 0);

	return TRUE;

}
BOOL CD3DRenderer::UpdateTextureAsYUV10Bits(DWORD dwWidth, DWORD dwHeight, BYTE* pYBuffer, BYTE* pUBuffer, BYTE* pVBuffer, DWORD Stride)
{
	if (0 == dwWidth || 0 == dwHeight)
		__debugbreak();


	if (m_dwTextureWidth != dwWidth || m_dwTextureHeight != dwHeight)
	{
		// 텍스쳐 다시 생성
		DeleteWritableTexture();
		CreateWritableTexture(dwWidth, dwHeight);
	}

	ID3D11Device*			pDevice = m_pD3DDevice;
	ID3D11DeviceContext*	pDeviceContext = m_pImmediateContext;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	memset(&mappedResource, 0, sizeof(mappedResource));

	HRESULT hr = pDeviceContext->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
		__debugbreak();

	const WORD*	y_buffer_entry = NULL;
	const WORD*	u_buffer_entry = NULL;
	const WORD*	v_buffer_entry = NULL;

	DWORD	StrideHalf = Stride / 2;

	DWORD		x, y;
	BYTE*		pDestEntry = NULL;
	for (y = 0; y < dwHeight; y++)
	{
		y_buffer_entry = (WORD*)(pYBuffer + Stride * y);
		u_buffer_entry = (WORD*)(pUBuffer + StrideHalf * (y >> 1));
		v_buffer_entry = (WORD*)(pVBuffer + StrideHalf * (y >> 1));

		pDestEntry = (BYTE*)mappedResource.pData + (mappedResource.RowPitch * y);

		for (x = 0; x < dwWidth; x++)
		{
			pDestEntry[0] = (BYTE)((*y_buffer_entry >> 2) & 0xff);
			pDestEntry[1] = (BYTE)((*u_buffer_entry >> 2) & 0xff);
			pDestEntry[2] = (BYTE)((*v_buffer_entry >> 2) & 0xff);
			pDestEntry[3] = 0xff;

			y_buffer_entry++;

			DWORD	uv_inc = x & 0x00000001;
			u_buffer_entry += uv_inc;
			v_buffer_entry += uv_inc;

			pDestEntry += 4;
		}
	}
	pDeviceContext->Unmap(m_pTexture, 0);

	return TRUE;

}
void CD3DRenderer::OutputFailToLoadShader(char* szUniqShaderName)
{
	char	szTxt[128];
	sprintf_s(szTxt, 128, "Fail to Load Shader:%s", szUniqShaderName);

	HWND	hWnd = m_hWnd;

	MessageBoxA(hWnd, szTxt, "Error", MB_ICONSTOP);
}
	
BOOL CD3DRenderer::Create32BitsImageFromFile(BYTE** ppOutBits, DWORD* pdwOutWidth, DWORD* pdwOutHeight, const WCHAR* wchFileName)
{
	BOOL	bResult = FALSE;

	DirectX::ScratchImage DecompressedImage;
	// only understands .dds files for now
	// return true if success

	ID3D11ShaderResourceView*	pTexSRV = nullptr;

	DirectX::TexMetadata	metaData = {};
	DirectX::ScratchImage	scratchImage;

	//char	szToonTexName[_MAX_PATH];
	//GetNameRemovePath(szToonTexName,szFileName);
	//if (!memcmp("ToonTable",szToonTexName,4))

	FILE*	fp = nullptr;
	_wfopen_s(&fp, wchFileName, L"rb");
	if (!fp)
		goto lb_return;

	DWORD	dwSize = (DWORD)GetFileSizeWithFP(fp);
	char*	pRawData = new char[dwSize];
	fread(pRawData, 1, dwSize, fp);

	HRESULT hr = S_OK;
	

	hr = LoadFromDDSMemory(pRawData, dwSize, DDS_FLAGS_NONE, &metaData, scratchImage);
	if (FAILED(hr))
	{
		hr = LoadFromWICMemory(pRawData, dwSize, WIC_FLAGS_NONE, &metaData, scratchImage);
		if (FAILED(hr))
		{
#ifdef _DEBUG 
			__debugbreak();
#endif
			goto lb_close_del_return;
		}
	}
	const DirectX::Image*	pImages = scratchImage.GetImages();
	

	size_t index = metaData.ComputeIndex(0, 0, 0);
	const Image& img = pImages[0];
	
	if (img.format != metaData.format)
		goto lb_close_del_return;
    
	if (!img.pixels)
		goto lb_close_del_return;

	DWORD	dwMemSize = (DWORD)(img.width * img.height * 4);
	BYTE* pBits = (BYTE*)malloc(dwMemSize);
	memset(pBits, 0, dwMemSize);

	
	
	if (img.format != DXGI_FORMAT_R8G8B8A8_UNORM && img.format != DXGI_FORMAT_B8G8R8A8_UNORM)
	{
		BOOL	bDecompressResult = FALSE;
		switch (img.format)
		{
			case DXGI_FORMAT_BC1_UNORM:
				bDecompressResult = DecompressDXT1ToRGBA(img.pixels, img.width, img.height, pBits, img.width * 4);
				break;
				
			case DXGI_FORMAT_BC2_UNORM:
				bDecompressResult = DecompressDXT3ToRGBA(img.pixels, img.width, img.height, pBits, img.width * 4);
				break;

			case DXGI_FORMAT_BC3_UNORM:
				bDecompressResult = DecompressDXT5ToRGBA(img.pixels, img.width, img.height, pBits, img.width * 4);
				break;
			
		}
		//DirectX::Image dxtImage = {};
		//dxtImage.width = img.width;
		//dxtImage.height = img.height;
		//dxtImage.format = metaData.format;
		//dxtImage.pixels = img.pixels;
		//dxtImage.rowPitch = img.rowPitch;
		
		//HRESULT hr1 = DirectX::Decompress(&dxtImage, 1, metaData, DXGI_FORMAT_R8G8B8A8_UNORM, DecompressedImage);
		//HRESULT hr = DirectX::Decompress(img, DXGI_FORMAT_R8G8B8A8_UNORM, DecompressedImage);
		//pSrc = DecompressedImage.GetPixels();
		
	}
	else
	{
		BYTE*	pSrc = (BYTE*)img.pixels;
		BYTE*	pDest = pBits;

		for (DWORD y = 0; y < img.height; y++)
		{
			memcpy(pDest, pSrc, img.width * 4);
			pDest += (img.width * 4);
			pSrc += img.rowPitch;
		}
	}

	*ppOutBits = pBits;
	*pdwOutWidth = (DWORD)img.width;
	*pdwOutHeight = (DWORD)img.height;
			
	bResult = TRUE;

lb_close_del_return:
	delete[] pRawData;
	fclose(fp);

lb_return:
	return bResult;
}
void CD3DRenderer::DeleteImage(BYTE* pBits)
{
	free(pBits);
}
BOOL CD3DRenderer::CreateTextureFromFile(ID3D11ShaderResourceView** ppOutTexResource, DWORD* pdwWidth, DWORD* pdwHeight, BOOL* pbHasAlpha, DWORD* pdwBPP, const WCHAR* wchFileName, BOOL bUseMipMap)
{
	BOOL	bResult = FALSE;

	// only understands .dds files for now
	// return true if success

	ID3D11ShaderResourceView*	pTexSRV = nullptr;

	DirectX::TexMetadata	metaData = {};
	DirectX::ScratchImage	scratchImage;

	*pbHasAlpha = FALSE;
	*pdwBPP = 0;

	//char	szToonTexName[_MAX_PATH];
	//GetNameRemovePath(szToonTexName,szFileName);
	//if (!memcmp("ToonTable",szToonTexName,4))

	FILE*	fp = nullptr;
	_wfopen_s(&fp, wchFileName, L"rb");
	if (!fp)
		goto lb_return;

	DWORD	dwSize = (DWORD)GetFileSizeWithFP(fp);
	char*	pRawData = new char[dwSize];
	fread(pRawData, 1, dwSize, fp);

	HRESULT hr = S_OK;



	ID3D11Texture2D*	pTex = nullptr;


	hr = LoadFromDDSMemory(pRawData, dwSize, DDS_FLAGS_NONE, &metaData, scratchImage);
	if (FAILED(hr))
	{
		hr = LoadFromWICMemory(pRawData, dwSize, WIC_FLAGS_NONE, &metaData, scratchImage);
		if (FAILED(hr))
		{
#ifdef _DEBUG 
			__debugbreak();
#endif
			goto lb_close_del_return;
		}
	}
	const DirectX::Image*	pImages = scratchImage.GetImages();

	hr = CreateShaderResourceViewEx(m_pD3DDevice, pImages, scratchImage.GetImageCount(), metaData, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, &pTexSRV);
	if (FAILED(hr))
	{
#ifdef _DEBUG 
		__debugbreak();
#endif
		goto lb_close_del_return;
	}
	pTexSRV->GetResource((ID3D11Resource**)&pTex);

	D3D11_TEXTURE2D_DESC desc;
	pTex->GetDesc(&desc);


	if (DXGI_FORMAT_R8G8B8A8_UNORM == desc.Format || DXGI_FORMAT_B8G8R8A8_UNORM == desc.Format || DXGI_FORMAT_BC3_UNORM == desc.Format)
	{
		*pbHasAlpha = TRUE;
	}

	*pdwBPP = 4;
	*pdwWidth = (DWORD)desc.Width;
	*pdwHeight = (DWORD)desc.Height;

	bResult = TRUE;

lb_close_del_return:
	if (pTex)
	{
		pTex->Release();
		pTex = nullptr;
	}
	delete[] pRawData;
	fclose(fp);

lb_return:
	*ppOutTexResource = pTexSRV;
	return bResult;
}
void CD3DRenderer::Cleanup()
{
	CleanupShader();
	CleanupBuffer();

	DeleteWritableTexture();

	if (m_pImmediateContext)
	{
		m_pImmediateContext->OMSetRenderTargets(0, NULL, NULL);
		m_pImmediateContext->ClearState();
	}

	for (DWORD i = 0; i < BLEND_TYPE_NUM; i++)
	{
		if (m_ppBlendState[i])
		{
			m_ppBlendState[i]->Release();
			m_ppBlendState[i] = NULL;
		}
	}
	for (DWORD i = 0; i < MAX_RASTER_TYPE_NUM; i++)
	{
		if (m_ppRasterizeState[i])
		{
			m_ppRasterizeState[i]->Release();
			m_ppRasterizeState[i] = NULL;
		}
	}

	for (DWORD i = 0; i < MAX_DEPTH_TYPE_NUM; i++)
	{
		if (m_ppDepthStencilState[i])
		{
			m_ppDepthStencilState[i]->Release();
			m_ppDepthStencilState[i] = NULL;
		}
	}
	for (DWORD i = 0; i < MAX_SAMPLER_TYPE_NUM; i++)
	{
		if (m_ppSamplerState[i])
		{
			m_ppSamplerState[i]->Release();
			m_ppSamplerState[i] = NULL;
		}
	}


	if (m_pDSV)
	{
		m_pDSV->Release();
		m_pDSV = NULL;

	}
	if (m_pDepthStencil)
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = NULL;
	}


	if (m_pDiffuseRTV)
	{
		m_pDiffuseRTV->Release();
		m_pDiffuseRTV = NULL;
	}

	if (m_pImmediateContext)
	{
		m_pImmediateContext->Release();
		m_pImmediateContext = NULL;
	}
	if (m_pBackBuffer)
	{
		m_pBackBuffer->Release();
		m_pBackBuffer = NULL;
	}

	if (m_pSwapChain)
	{
		m_pSwapChain->SetFullscreenState(FALSE, NULL);
		m_pSwapChain->Release();
		m_pSwapChain = NULL;
	}




	if (m_pD3DDevice)
	{
		ULONG	ref_count = m_pD3DDevice->Release();
		if (ref_count)
		{
			__debugbreak();
		}
		m_pD3DDevice = NULL;
	}
}
CD3DRenderer::~CD3DRenderer()
{
	Cleanup();
}

/*
BOOL CD3DRenderer::UpdateWritableTexture(BYTE* pSrc, DWORD dwWidth,DWORD dwHeight,DWORD dwPitch)
{
	__debugbreak();

	if (0 == dwHeight || 0 == dwHeight)
		__debugbreak();

	if (m_dwTextureWidth != dwWidth || m_dwTextureHeight != dwHeight)
	{
		DeleteWritableTexture();
		CreateWritableTexture(dwWidth,dwHeight);
	}

	ID3D11Device*			pDevice = m_pD3DDevice;
	ID3D11DeviceContext*	pDeviceContext = m_pImmediateContext;


	D3D11_TEXTURE2D_DESC ddsc;
	m_pTexture->GetDesc(&ddsc);


	D3D11_MAPPED_SUBRESOURCE mappedResource;
	memset(&mappedResource,0,sizeof(mappedResource));

	HRESULT hr = pDeviceContext->Map(m_pTexture,0,D3D11_MAP_WRITE_DISCARD,0,&mappedResource);
	if (FAILED(hr))
		__debugbreak();


	BYTE*		pDest = (BYTE*)mappedResource.pData;

	for (DWORD y=0; y<dwHeight; y++)
	{
		memcpy(pDest,pSrc,sizeof(DWORD)*dwWidth);
		pDest += mappedResource.RowPitch;
		pSrc += dwPitch;
	}
//	pOutLockedRect->pBits = pDest;
//	pOutLockedRect->Pitch = mappedResource.RowPitch;




	pDeviceContext->Unmap(m_pTexture,0);

	return TRUE;

}
*/

BOOL DecompressDXT1ToRGBA(const uint8_t* pCompressedImage, int iWidth, int iHeight, uint8_t* pDestBits, size_t DestPitch)
{
	const uint32_t BlockSize = 8;
	uint32_t BlockWidth = ((iWidth + 3) / 4);
	uint32_t BlockHeight = ((iHeight + 3) / 4);
	size_t CompressedImagePitch = BlockWidth * BlockSize;
	uint32_t Size = CompressedImagePitch * BlockHeight;

	BOOL bResult = DecompressDXTtoRGBA(pCompressedImage, CompressedImagePitch, iWidth, iHeight, DXGI_FORMAT_BC1_UNORM, pDestBits, DestPitch);
	return bResult;
}
BOOL DecompressDXT3ToRGBA(const uint8_t* pCompressedImage, int iWidth, int iHeight, uint8_t* pDestBits, size_t DestPitch)
{
	const uint32_t BlockSize = 16;
	uint32_t BlockWidth = ((iWidth + 3) / 4);
	uint32_t BlockHeight = ((iHeight + 3) / 4);
	size_t CompressedImagePitch = BlockWidth * BlockSize;
	uint32_t Size = CompressedImagePitch * BlockHeight;

	BOOL bResult = DecompressDXTtoRGBA(pCompressedImage, CompressedImagePitch, iWidth, iHeight, DXGI_FORMAT_BC2_UNORM, pDestBits, DestPitch);
	return bResult;
}
BOOL DecompressDXT5ToRGBA(const uint8_t* pCompressedImage, int iWidth, int iHeight, uint8_t* pDestBits, size_t DestPitch)
{
	const uint32_t BlockSize = 16;
	uint32_t BlockWidth = ((iWidth + 3) / 4);
	uint32_t BlockHeight = ((iHeight + 3) / 4);
	size_t CompressedImagePitch = BlockWidth * BlockSize;
	uint32_t Size = CompressedImagePitch * BlockHeight;

	BOOL bResult = DecompressDXTtoRGBA(pCompressedImage, CompressedImagePitch, iWidth, iHeight, DXGI_FORMAT_BC3_UNORM, pDestBits, DestPitch);
	return bResult;
}
BOOL DecompressDXTtoRGBA(const uint8_t* pCompressedImage, size_t CompressedImagePitch, int iWidth, int iHeight, DXGI_FORMAT srcFormat, uint8_t* pDestBits, size_t DestPitch)
{
	DirectX::ScratchImage DecompressedImage;
	DirectX::TexMetadata	metaData = {};

	DirectX::Image dxtImage = {};
	dxtImage.pixels = (uint8_t*)pCompressedImage;
	dxtImage.width = iWidth;
	dxtImage.height = iHeight;
	dxtImage.format = srcFormat;
	dxtImage.rowPitch = CompressedImagePitch;

	metaData.width = iWidth;
	metaData.height = iHeight;
	metaData.depth = 1;
	metaData.arraySize = 1;
	metaData.mipLevels = 1;
	metaData.format = srcFormat;
	metaData.dimension = TEX_DIMENSION_TEXTURE2D;

	HRESULT hr = DirectX::Decompress(&dxtImage, 1, metaData, DXGI_FORMAT_R8G8B8A8_UNORM, DecompressedImage);
	if (S_OK != hr)
	{
		return FALSE;
	}
	BYTE*	pSrc = DecompressedImage.GetPixels();
	BYTE*	pDest = pDestBits;

	const DirectX::Image* pSrcImage = DecompressedImage.GetImages();
	size_t DecompressedSrcPitch = pSrcImage->rowPitch;

	for (DWORD y = 0; y < iHeight; y++)
	{
		memcpy(pDest, pSrc, iWidth * 4);
		pDest += DestPitch;
		pSrc += DecompressedSrcPitch;
	}
	return TRUE;
}