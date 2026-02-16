// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <afxcmn.h>   // MFC support for Windows Common Controls
#include <afxdisp.h>  // MFC Automation classes
#include <afxext.h>   // MFC extensions
#include <afxmt.h>    // MFC multithreading
#include <afxtempl.h> // MFC Template classes
#include <afxwin.h>   // MFC core and standard components

#include <d2d1_3.h>
#include <d2d1_1helper.h>
#include <d3d11_3.h>
#include <d3dcommon.h>
#include <dcommon.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgiformat.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <format>
#include <fstream>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <wincodec.h>
#include <wrl/client.h>

#include <Windows.h>

#endif //PCH_H
