#pragma once

class CD2DView
{
	HWND					m_hWnd;
	ID2D1HwndRenderTarget*	m_pRenderTarget;
	ID2D1Bitmap*			m_pBitmap;
	ID2D1SolidColorBrush*	m_pPlanetBackgroundBrush;
	ID2D1SolidColorBrush*	m_pBlackBrush;
	IDWriteTextFormat*		m_pTextFormat;
	BYTE*					m_pBitmapBuffer;
	BOOL					m_bIsBeginRender;
	int		m_BitmapWidth;
	int		m_BitmapHeight;

	float	m_dpiScaleX;
	float	m_dpiScaleY;

	void	Cleanup();
public:
	BOOL	Initialize(HWND hWnd);
	void	UpdateSurface(BYTE* pBuf, int Width, int Height, int Pitch);

	BOOL	BeginDraw();
	void	DrawSurface();
	BOOL	DrawText(const WCHAR* wchTxt, UINT32 cTexLen, RECT* pRect);
	BOOL	EndDraw();
	CD2DView();
	~CD2DView();

};
