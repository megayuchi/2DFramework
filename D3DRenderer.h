#pragma once

#include "d3d_type.h"


class CD3DRenderer
{
public:
	enum IMAGE_FORMAT
	{
		IMAGE_FORMAT_YUV,
		IMAGE_FORMAT_RGBA
	};
private:
	HWND	m_hWnd = nullptr;
	DWORD	m_dwWidth = 0;
	DWORD	m_dwHeight = 0;
	DWORD	m_dwCreateFlags = 0;

	IMAGE_FORMAT	m_ImageFormat = IMAGE_FORMAT_RGBA;
	D3D11_FILL_MODE				m_FillMode = D3D11_FILL_SOLID;
	D3D_FEATURE_LEVEL			m_FeatureLevel = D3D_FEATURE_LEVEL_9_1;
	D3D_DRIVER_TYPE				m_driverType = D3D_DRIVER_TYPE_UNKNOWN;
	ID3D11Device*				m_pD3DDevice = nullptr;
	ID3D11DeviceContext*		m_pImmediateContext = nullptr;
	IDXGISwapChain*				m_pSwapChain = nullptr;
	ID3D11Texture2D*			m_pBackBuffer = nullptr;
	ID3D11RenderTargetView*		m_pDiffuseRTV = nullptr;
	ID3D11DepthStencilView*		m_pDSV = nullptr;
	ID3D11Texture2D*			m_pDepthStencil = nullptr;

	WCHAR	m_wchShaderPath[_MAX_PATH] = {};
	ID3D11DepthStencilState*	m_ppDepthStencilState[MAX_DEPTH_TYPE_NUM] = {};
	ID3D11BlendState*			m_ppBlendState[BLEND_TYPE_NUM] = {};
	ID3D11RasterizerState*		m_ppRasterizeState[MAX_RASTER_TYPE_NUM] = {};
	ID3D11SamplerState*			m_ppSamplerState[MAX_SAMPLER_TYPE_NUM] = {};
	D3D11_VIEWPORT				m_vp = {};


	DWORD						m_dwTextureWidth = 0;
	DWORD						m_dwTextureHeight = 0;
	ID3D11Texture2D*			m_pTexture = nullptr;
	ID3D11ShaderResourceView*	m_pTextureSRV = nullptr;

	ID3D11Buffer*				m_pVertexBuffer = nullptr;
	ID3D11Buffer*				m_pIndexBuffer = nullptr;
	ID3D11Buffer*				m_pConstantBuffer = nullptr;
	ID3D11InputLayout*			m_pVertexLayout = nullptr;
	SHADER_HANDLE*				m_pVS = nullptr;
	SHADER_HANDLE*				m_pPS_YUV = nullptr;
	SHADER_HANDLE*				m_pPS_RGBA = nullptr;


	BOOL	CreateBackBuffer(UINT uiWidth, UINT uiHeight);
	void	CreateStates();
	BOOL	CreateShaderResourceViewFromTex2D(ID3D11ShaderResourceView** ppOutSRV, ID3D11Texture2D** ppOutTexture, UINT Width, UINT Height, DXGI_FORMAT Format, D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags, D3D11_SUBRESOURCE_DATA* pInitData);
	BOOL	CreateTextureFromFile(ID3D11ShaderResourceView** ppOutTexResource, DWORD* pdwWidth, DWORD* pdwHeight, BOOL* pbHasAlpha, DWORD* pdwBPP, const WCHAR* wchFileName, BOOL bUseMipMap);
	

	SHADER_HANDLE*	CreateShader(char* szShaderFileName, char* szEntryName, SHADER_TYPE ShaderType, DWORD ShaderParams);
	SHADER_HANDLE*	CreateShaderHandle(char* szShaderName, DWORD dwShaderNameLen, char* szShaderFileName, DWORD dwShaderFileNameLen, SYSTEMTIME* pCreationTime, void* pCodeBuffer, DWORD dwCodeSize, SHADER_TYPE ShaderType);
	void			ReleaseShader(SHADER_HANDLE* pShaderHandle);
	void			OutputFailToLoadShader(char* szUniqShaderName);


	BOOL	InitShader();
	void	CleanupShader();

	BOOL	InitBuffer();
	void	CleanupBuffer();

	BOOL	SetD3DDebugSetting();

	void	Cleanup();
public:
	BOOL	Initialize(HWND hWnd, IMAGE_FORMAT fmt, const WCHAR* wchShaderPath, DWORD dwFlags);
	BOOL	CreateWritableTexture(DWORD dwWidth, DWORD dwHeight);
	void	DeleteWritableTexture();
	BOOL	CreateImageFromFile(char** ppOutBits, DWORD* pdwWidth, DWORD* pdwHeight, BOOL* pbHasAlpha, DWORD* pdwBPP, const WCHAR* wchFileName);
	void	DeleteImage(char* pBits);
	BOOL	UpdateTextureAsYUV(DWORD dwWidth, DWORD dwHeight, BYTE* pYBuffer, BYTE* pUBuffer, BYTE* pVBuffer, DWORD Stride);
	BOOL	UpdateTextureAsYUV10Bits(DWORD dwWidth, DWORD dwHeight, BYTE* pYBuffer, BYTE* pUBuffer, BYTE* pVBuffer, DWORD Stride);

	void	BeginRender(DWORD dwColor, DWORD dwFlags);
	BOOL	Draw(DWORD dwWidth, DWORD dwHeight, DWORD dwPosX, DWORD dwPosY, DWORD dwColor, DWORD dwFlags);
	void	EndRender();
	void	Present(HWND hWnd);

	DWORD	GetWidth() { return m_dwWidth; }
	DWORD	GetHeight() { return m_dwHeight; }
	void	GetTextureSize(DWORD* pdwOutWidth, DWORD* pdwOutHeight) { *pdwOutWidth = m_dwTextureWidth; *pdwOutHeight = m_dwTextureHeight; }
	BOOL	UpdateWindowSize(UINT uiWidth, UINT uiHeight);

	ID3D11SamplerState*		INL_GetSamplerState(SAMPLER_TYPE type) { return m_ppSamplerState[type]; }
	CD3DRenderer();
	~CD3DRenderer();
};

