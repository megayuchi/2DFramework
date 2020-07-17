// ImageToTris.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "2DFramework.h"
#include "D3DRenderer.h"
#include "D2DView.h"


#if defined(WIN64)
	#ifdef _DEBUG
		#pragma comment(lib, "../DirectXTex/Bin/Desktop_2019/x64/Debug/DirectXTex.lib")	
	#else
		#pragma comment(lib, "../DirectXTex/Bin/Desktop_2019/x64/Release/DirectXTex.lib")	
	#endif
#elif defined(ARM64)
	#ifdef _DEBUG
		//#pragma comment(lib, "../DirectXTex/Bin/Desktop_2019/ARM64/Release/DirectXTex.lib")	
		//#pragma comment(lib, "../DirectXTex/Bin/Desktop_2019/ARM64/Debug/DirectXTex.lib")	
		//#pragma comment(lib, "../DirectXTex/Bin/Desktop_2019/ARM64/Release/DirectXTex.lib")	
	#else
		//#pragma comment(lib, "../DirectXTex/Bin/Desktop_2019/ARM64/Release/DirectXTex.lib")	
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "../DirectXTex/Bin/Desktop_2019/Win32/Debug/DirectXTex.lib")	
	#else
		#pragma comment(lib, "../DirectXTex/Bin/Desktop_2019/Win32/Release/DirectXTex.lib")	
	#endif
#endif

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


HWND		g_hMainWindow = nullptr;
CD2DView*	g_pD2DView = nullptr;
CD3DRenderer*	g_pD3DRenderer = nullptr;

BOOL	g_bIsPlaying = FALSE;
float	g_fRefreshRate = 60.0f;
ULONGLONG	g_PrvRenderTick = 0;

void ProcessD2D();
void ProcessD3D(const BYTE* pImageBits, DWORD dwImageWidth, DWORD dwImageHeight);

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_2DFRAMEWORK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);


#ifdef _DEBUG
	int	flag = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;

	_CrtSetDbgFlag(flag);
#endif
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_2DFRAMEWORK));

	MSG msg;

	//g_pD2DView = new CD2DView;
	//g_pD2DView->Initialize(g_hMainWindow);

	g_pD3DRenderer = new CD3DRenderer;
	g_pD3DRenderer->Initialize(g_hMainWindow, CD3DRenderer::IMAGE_FORMAT_YUV, L"./Shaders", 0);

	DWORD	dwWidth = g_pD3DRenderer->GetWidth();
	DWORD	dwHeight = g_pD3DRenderer->GetHeight();

	g_pD3DRenderer->CreateWritableTexture(dwWidth, dwHeight);
	BYTE*	pImageBits = nullptr;
	DWORD	dwImageWidth = 0;
	DWORD	dwImageHeight = 0;
	if (!g_pD3DRenderer->Create32BitsImageFromFile(&pImageBits, &dwImageWidth, &dwImageHeight, L"./Data/03_chara.png"))
	{
		__debugbreak();
	}
	


	//g_pD2DView->BeginDraw();
	//g_pD2DView->DrawSurface();
	//g_pD2DView->EndDraw();



	/*
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	*/
	while (TRUE)
	{
		BOOL	bHasMsg = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (bHasMsg)
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);


		}
		else
		{
			//ProcessD2D();
			ProcessD3D(pImageBits, dwImageWidth, dwImageHeight);
		}
	}
	if (pImageBits)
	{
		g_pD3DRenderer->DeleteImage(pImageBits);
		pImageBits = nullptr;
	}

	if (g_pD2DView)
	{
		delete g_pD2DView;
		g_pD2DView = nullptr;
	}
	if (g_pD3DRenderer)
	{
		g_pD3DRenderer->DeleteWritableTexture();

		delete g_pD3DRenderer;
		g_pD3DRenderer = nullptr;
	}

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif
	return (int)msg.wParam;
}


void ProcessD2D()
{
	if (!g_pD2DView)
		return;

	WCHAR	wchTxt[128] = {};
	DWORD	dwLen = swprintf_s(wchTxt, L"D2DTest");
	//
	// 카피
	//
	DWORD	dwWidth = 0;
	DWORD	dwHeight = 0;
	BYTE*	pImageBuffer = nullptr;
	//g_pD2DView->UpdateSurface(pImageBuffer, dwWidth, dwHeight, dwWidth * 4);

	g_pD2DView->BeginDraw();

	g_pD2DView->DrawSurface();
	g_pD2DView->DrawText(wchTxt, dwLen, nullptr);
	g_pD2DView->EndDraw();


}

void ProcessD3D(const BYTE* pImageBits, DWORD dwImageWidth, DWORD dwImageHeight)
{
	if (!g_pD3DRenderer)
		return;

	//
	// 카피
	//
	


	DWORD	dwScreenWidth = g_pD3DRenderer->GetWidth();
	DWORD	dwScreenHeight = g_pD3DRenderer->GetHeight();

	DWORD	dwRenderWidth = dwScreenWidth;
	DWORD	dwRenderHeight = dwScreenHeight;

	g_pD3DRenderer->UpdateTextureAsRGBA(pImageBits, dwImageWidth, dwImageHeight);

	//DWORD	dwImageWidth = 0;
	//DWORD	dwImageHeight = 0;
	//DWORD	dwImageStride = 0;
	//BYTE*	pYBuffer = nullptr;
	//BYTE*	pUBuffer = nullptr;
	//BYTE*	pVBuffer = nullptr;
	//g_pD3DRenderer->UpdateYUVTexture(dwImageWidth, dwImageHeight, pYBuffer, pUBuffer, pVBuffer, dwImageStride);

	g_pD3DRenderer->BeginRender(0xff0000ff, 0);
	g_pD3DRenderer->Draw(dwRenderWidth, dwRenderHeight, 0, 0, 0xff00ff00, 0);
	g_pD3DRenderer->EndRender();

	g_pD3DRenderer->Present(g_hMainWindow);
}

void OnWindowSize()
{
	RECT	rect;
	GetClientRect(g_hMainWindow, &rect);
	UINT	uiWidth = rect.right - rect.left;
	UINT	uiHeight = rect.bottom - rect.top;

	if (g_pD3DRenderer)
	{
		g_pD3DRenderer->UpdateWindowSize(uiWidth, uiHeight);
	}
}
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_2DFRAMEWORK));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_2DFRAMEWORK);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
							  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}
	g_hMainWindow = hWnd;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);
				// Parse the menu selections:
				switch (wmId)
				{
					case IDM_ABOUT:
						DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
						break;
					case IDM_EXIT:
						DestroyWindow(hWnd);
						break;
					default:
						return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code that uses hdc here...
				EndPaint(hWnd, &ps);
			}
			break;
		
		case	WM_SIZE:
			OnWindowSize();
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
