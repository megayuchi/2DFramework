#include "pch.h"
#include <d2d1.h>
#include <dwrite.h>
#include "D2DView.h"


#pragma comment( lib, "d2d1.lib" )
#pragma comment( lib, "Dwrite.lib" )





/*
	ID2D1Factory* m_pDirect2dFactory;


	ID2D1SolidColorBrush* m_pStarBrush;
	ID2D1SolidColorBrush* m_pContinentBrush;
	ID2D1SolidColorBrush* m_pSmallStarBrush;
	ID2D1RadialGradientBrush* m_pStartOutlineBrush;
	ID2D1PathGeometry* m_pStarOutline;
	ID2D1PathGeometry* m_pPlanetUpPath;
	ID2D1PathGeometry* m_pPlanetDownPath;

	*/


CD2DView::CD2DView()
{
	m_hWnd = NULL;
	m_pRenderTarget = NULL;
	m_pBitmap = NULL;
	m_pPlanetBackgroundBrush = NULL;
	m_pBlackBrush = NULL;
	m_pTextFormat = NULL;
	m_pBitmapBuffer = NULL;

	m_BitmapWidth = 0;
	m_BitmapHeight = 0;

	m_dpiScaleX = 0.0f;
	m_dpiScaleY = 0.0f;
	m_bIsBeginRender = FALSE;
}
BOOL CD2DView::Initialize(HWND hWnd)
{
	m_hWnd = hWnd;

	HRESULT hr = S_OK;

	ID2D1Factory* pDirect2dFactory = NULL;

	//__uuidof(ID2D1Factory2)
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pDirect2dFactory);
	if (S_OK != hr)
		__debugbreak();

	RECT	rect = { 0 };
	GetClientRect(hWnd, &rect);

	D2D1_SIZE_U size = D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top);

	hr = pDirect2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY), &m_pRenderTarget);
	if (S_OK != hr)
		__debugbreak();

	///pDirect2dFactory->CreateDxgiSurfaceRenderTarget(&pSurface,D2D1::HwndRenderTargetProperties(hWnd, size),

	D2D1_BITMAP_PROPERTIES	prop;
	prop.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	prop.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

	m_pRenderTarget->GetDpi(&prop.dpiX, &prop.dpiY);
	hr = m_pRenderTarget->CreateBitmap(size, prop, &m_pBitmap);
	if (S_OK != hr)
		__debugbreak();

	DWORD	Width = rect.right;
	DWORD	Height = rect.bottom;

	D2D_RECT_U	destRect;
	destRect.left = 0;
	destRect.top = 0;
	destRect.right = Width;
	destRect.bottom = Height;

	DWORD	Pitch = Width * 4;

	BYTE* pSrc = new BYTE[Width * Height * 4];
	memset(pSrc, 0xf0, Width * Height * 4);

	m_pBitmap->CopyFromMemory(&destRect, pSrc, Pitch);



	delete[] pSrc;
	pSrc = NULL;
	//g_pRenderTarget->DrawBitmap(pBitmap,

	// Create a SolidColorBrush (planet background).
	hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(0x007DD5)), &m_pPlanetBackgroundBrush);
	if (S_OK != hr)
		__debugbreak();

	hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBlackBrush);
	if (S_OK != hr)
		__debugbreak();


	//get the dpi information
	HDC screen = GetDC(0);
	m_dpiScaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
	m_dpiScaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
	ReleaseDC(0, screen);



	IDWriteFactory* pDWriteFactory = NULL;
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (S_OK != hr)
		__debugbreak();

	// Create a text format using Gabriola with a font size of 72.
	// This sets the default font, weight, stretch, style, and locale.


	hr = pDWriteFactory->CreateTextFormat(
		L"Arial",                // Font family name.
		NULL,                       // Font collection (NULL sets it to use the system font collection).
		DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		18.0f, L"en-us",
		&m_pTextFormat);

	if (S_OK != hr)
		__debugbreak();


	// Center align (horizontally) the text.

	hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	if (S_OK != hr)
		__debugbreak();

	hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	if (S_OK != hr)
		__debugbreak();


	if (pDWriteFactory)
	{
		pDWriteFactory->Release();
		pDWriteFactory = NULL;
	}

	if (pDirect2dFactory)
	{
		pDirect2dFactory->Release();
		pDirect2dFactory = NULL;
	}
	return TRUE;
}
BOOL CD2DView::DrawText(const WCHAR* wchTxt, UINT32 cTexLen, RECT* pRect)
{
	RECT rc = { 0 };

	GetClientRect(
		m_hWnd,
		&rc
	);

	// Create a D2D rect that is the same size as the window.

	//D2D1_RECT_F layoutRect = D2D1::RectF(
	//    static_cast<FLOAT>(rc.left) / g_dpiScaleX,
	//    static_cast<FLOAT>(rc.top) / g_dpiScaleY,
	//    static_cast<FLOAT>(rc.right - rc.left) / g_dpiScaleX,
	//    static_cast<FLOAT>(rc.bottom - rc.top) / g_dpiScaleY
	//    );

	D2D1_RECT_F layoutRect = D2D1::RectF(0, 32, 256, 100);
	if (pRect)
	{
		layoutRect.left = (float)pRect->left;
		layoutRect.top = (float)pRect->top;
		layoutRect.right = (float)pRect->right;
		layoutRect.bottom = (float)pRect->bottom;
	}

	// Use the DrawText method of the D2D render target interface to draw.

	m_pRenderTarget->DrawText(
		wchTxt,        // The string to render.
		cTexLen,    // The string's length.
		m_pTextFormat,    // The text format.
		layoutRect,       // The region of the window where the text will be rendered.
		m_pBlackBrush     // The brush used to draw the text.
	);

	return TRUE;
}


void CD2DView::UpdateSurface(BYTE* pBuf, int Width, int Height, int Pitch)
{
	if (0 == Width || 0 == Height)
		__debugbreak();

	/*
	if (!g_pBitmapBuffer)
	{
		g_BitmapWidth = Width;
		g_BitmapHeight = Height;
		g_pBitmapBuffer = new BYTE[Width*Height * 4];
	}
	BYTE*	pDest = g_pBitmapBuffer;
	BYTE*	pSrc = pBuf;
	for (int y = 0; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			pDest[0] = pSrc[2];
			pDest[1] = pSrc[1];
			pDest[2] = pSrc[0];
			pDest[3] = 255;//pSrc[2];
			pSrc += 4;
			pDest += 4;
		}
	}
	*/
	D2D_RECT_U	destRect;
	destRect.left = 0;
	destRect.top = 0;
	destRect.right = Width;
	destRect.bottom = Height;

	m_pBitmap->CopyFromMemory(&destRect, pBuf, Width * 4);

}
void CD2DView::DrawSurface()
{
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Blue));
	//g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	m_pRenderTarget->DrawBitmap(m_pBitmap, NULL, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, NULL);



	//g_pRenderTarget->DrawBitmap


}

BOOL CD2DView::BeginDraw()
{
	if (m_bIsBeginRender)
	{
	#ifdef _DEBUG
		__debugbreak();
	#endif
		return FALSE;
	}


	m_pRenderTarget->BeginDraw();
	//	m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Blue));
	//	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	//	m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

	m_bIsBeginRender = TRUE;

	return TRUE;

}
BOOL CD2DView::EndDraw()
{
	if (!m_bIsBeginRender)
	{
	#ifdef _DEBUG
		__debugbreak();
	#endif
		return FALSE;
	}

	HRESULT hr = m_pRenderTarget->EndDraw();
	if (S_OK != hr)
		__debugbreak();

	m_bIsBeginRender = FALSE;

	return TRUE;
}

void CD2DView::Cleanup()
{
	if (m_pBitmapBuffer)
	{
		delete[] m_pBitmapBuffer;
		m_pBitmapBuffer = NULL;
	}
	if (m_pBitmap)
	{
		m_pBitmap->Release();
		m_pBitmap = NULL;
	}
	if (m_pTextFormat)
	{
		m_pTextFormat->Release();
		m_pTextFormat = NULL;
	}
	if (m_pPlanetBackgroundBrush)
	{
		m_pPlanetBackgroundBrush->Release();
		m_pPlanetBackgroundBrush = NULL;
	}
	if (m_pBlackBrush)
	{
		m_pBlackBrush->Release();
		m_pBlackBrush = NULL;
	}
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Release();
		m_pRenderTarget = NULL;
	}
}

CD2DView::~CD2DView()
{
	Cleanup();
}