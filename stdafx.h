// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <process.h>

// TODO: reference additional headers your program requires here
extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
}
#include <d2d1.h>
#include <dwrite.h>
#include "decoder_typedef.h"
#include "Util.h"
#include <d3d11.h>
#include <float.h>
#include "math.inl"
#include "d3d_type.h"
