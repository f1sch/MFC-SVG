#include "pch.h"

#include "Window.h"

#include "DebugLogger.h"
#include "Devices.h"
#include "Renderer.h"

#include <afx.h>
#include <afxwin.h>
#include <afxmsg_.h>
#include <atltypes.h>
#include <d2d1helper.h>
#include <d2d1.h>
#include <format>
#include <memory>
#include <string>
#include <utility>
#include <Windows.h>
#include <dxgiformat.h>
#include <d2d1_1helper.h>
#include <d2d1_1.h>
#include <dcommon.h>
#include <afxcmn.h>

using namespace SvgLib::Core;
using namespace SvgLib::Graphics;
using namespace SvgLib::Window;

namespace SvgLib::Window
{
	IMPLEMENT_DYNCREATE(Window, CWnd);

	BEGIN_MESSAGE_MAP(Window, CWnd)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_SIZE()
		ON_WM_PAINT()
		ON_WM_LBUTTONDOWN()
		ON_WM_MOUSEMOVE()
		ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
		ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
		ON_WM_MOUSEWHEEL()
	END_MESSAGE_MAP()
}

Window::Window()
	: isInitialized_(false), /*isGettingDestroyed_(false),*/ mouseOverId_(L"")
{
	auto& log = DebugLogger::Instance();
	log.Info(">>>>>>>>>>>>>>>> DebugLogger started in SvgLib >>>>>>>>>>>>>>>>");
}

Window::~Window()
{
	DebugLogger::Info("<<<<<<<<<<<<<<<< DebugLogger stopped in SvgLib <<<<<<<<<<<<<<<<");
}

HRESULT Window::Initialize(CWnd* pParent, const CRect& rect)
{
	BOOL created = CreateInPlace(pParent, rect);
	
	HRESULT hr = S_OK;
	if (created)
	{
		isInitialized_ = true;
		hr = S_OK;
	}
	else
	{
		isInitialized_ = false;
		hr = S_FALSE;
	}
	return hr;
}

BOOL Window::CreateInPlace(CWnd* pParent, const CRect& rect)
{
	return Create(
		AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW),
		_T("SvgLibRenderWindow"),
		WS_CHILD | WS_VISIBLE,
		rect,
		pParent,
		0
	);
}

void Window::SetGraphicToLoad(std::wstring filename)
{
	devices_->SetFilePathForSvgLoading(filename);
	// Request a repaint so the new SVG is picked up via ProcessPending() + Render().
	if (GetSafeHwnd() != nullptr)
	{
		Invalidate(FALSE);
	}
}

// Only called if CWnd::Create() is called
int Window::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	devices_ = std::make_unique<Devices>();
	if (devices_)
	{
		HRESULT hr = S_OK;
		hr = devices_->Initialize(GetSafeHwnd());
		if (FAILED(hr))
			return -2;
		renderer_ = std::make_unique<Renderer>(devices_.get());
		renderer_->InitializeRenderer();
	}
	return 0;
}

void Window::OnDestroy()
{
	//isGettingDestroyed_ = true;
	devices_->BeginShutdown();
	
	renderer_->ReleaseDeviceDependentResources();
	renderer_.reset();
	
	devices_->DiscardEverything();
	devices_->ReportLiveDXObjects(-1);
	
	devices_.reset();
	
	CWnd::OnDestroy();
}

void Window::OnPaint()
{	
	CPaintDC dc(this);
	//if (isGettingDestroyed_ || !renderer_) return;
	if (!renderer_ || !devices_) return;
	
	HRESULT hr = devices_->ProcessPending();
	if (hr == S_FALSE) return;
	if (FAILED(hr)) return;
	
	RECT rcPaint = dc.m_ps.rcPaint;
	if (rcPaint.right <= rcPaint.left || rcPaint.bottom <= rcPaint.top)
		GetClientRect(&rcPaint);

	hr = renderer_->Render(rcPaint);
	if (FAILED(hr))
	{
		DebugLogger::Warning("Something went wrong while Rendering");
	}
}

void Window::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (!devices_) return;
	
	devices_->RequestResize();
	Invalidate(FALSE);
	
	if (renderer_)
		renderer_->CreateDeviceDependentResources();
}

void Window::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (renderer_)
	{
		UINT dpi = ::GetDpiForWindow(m_hWnd);
		float dpiScale = static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);
		float xDip = point.x / dpiScale;
		float yDip = point.y / dpiScale;
		DebugLogger::Info(std::format("Point:{}|{} is inside Direct2D Window", point.x, point.y));
		
		std::wstring geometryId = renderer_->OnMouseDown({ xDip, yDip });
		if (!geometryId.empty())
		{
			DebugLogger::Info(std::format(L"Geometry {} was hit!", geometryId));
		}
		
		//if (requestContextMenuCallback_)
		//{
		//	requestContextMenuCallback_(geometryId, { static_cast<int>(xDip), static_cast<int>(yDip) });
		//}

	}
	CWnd::OnLButtonDown(nFlags, point);
}

void Window::OnMouseMove(UINT nFlags, CPoint point)
{
	UINT dpi = GetDpiForWindow(GetSafeHwnd());
	float sx = 96.0f / dpi;
	float sy = 96.0f / dpi;
	std::wstring id = renderer_->HitTesting2D(static_cast<float>(point.x * sx), static_cast<float>(point.y * sy));
	
	if (id != mouseOverId_)
	{
		mouseOverId_ = id;
		if (listener_)
		{
			listener_->OnSvgHit(id);
		}
		renderer_->SetHoveredElement(id);

		Invalidate();
	}
	
	
	if (!mouseOver_)
	{
		TRACE(_T("Mouse enter\n"));
		mouseOver_ = TRUE;
		//DebugLogger::Info(std::format(L"OnMouseMove(): mouseOverId = {}", mouseOverId_));

		TRACKMOUSEEVENT tme = {};
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.hwndTrack = devices_->GetHwnd();
		tme.dwHoverTime = HOVER_DEFAULT;
		::TrackMouseEvent(&tme);
	}

}

LRESULT Window::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	TRACE(_T("Mouse leave\n"));
	mouseOver_ = FALSE;
	mouseOverId_ = L"";
	renderer_->SetHoveredElement(L"");
	Invalidate();
	return 0;
}

LRESULT Window::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
	//TRACE(_T("Mouse hover (x=%d, y=%d)\n"),
	//	LOWORD(lParam), HIWORD(lParam));
	float px = (float)GET_X_LPARAM(lParam);
	float py = (float)GET_Y_LPARAM(lParam);
	
	UINT dpi = GetDpiForWindow(m_hWnd);
	float sx = 96.0f / dpi;
	float sy = 96.0f / dpi;

	mouseOverId_ = renderer_->HitTesting2D(px * sx, py * sy);
	//DebugLogger::Info(std::format(L"OnMouseHover(): mouseOverId = {}", mouseOverId_));
	if (listener_)
	{
		listener_->OnSvgHit(mouseOverId_);
	}
	if (renderer_->GetHoveredElement() != mouseOverId_)
	{
		renderer_->SetHoveredElement(mouseOverId_);
		//TRACE(_T("Test inside:\n"));
		Invalidate();
	}

	TRACKMOUSEEVENT tme = {};
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_HOVER | TME_LEAVE;
	tme.hwndTrack = devices_->GetHwnd();
	tme.dwHoverTime = HOVER_DEFAULT;
	::TrackMouseEvent(&tme);

	return 0;
}

BOOL Window::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	BOOL up = TRUE;
	int delta = zDelta;

	if (zDelta < 0)
	{
		up = FALSE;
		delta = -delta;
	}

	UINT wheelScrollLines = 0;
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &wheelScrollLines, 0);

	if (wheelScrollLines == WHEEL_PAGESCROLL)
	{
		SendMessage(WM_VSCROLL, MAKEWPARAM(up ? SB_PAGEUP : SB_PAGEDOWN, 0), 0);
	}
	else
	{
		int lines = (delta * wheelScrollLines) / WHEEL_DELTA;
		while (lines--)
		{
			SendMessage(WM_VSCROLL, MAKEWPARAM(up ? SB_LINEUP : SB_LINEDOWN, 0), 0);
		}
	}
	return TRUE;
}

D2D1_POINT_2F Window::GetClientPixelInDPI()
{
	float dpiX, dpiY;
	devices_->GetD2DDeviceContext()->GetDpi(&dpiX, &dpiY);
	CRect rc;
	GetClientRect(&rc);
	float width = rc.Width() * 96.0f / dpiX;
	float height = rc.Height() * 96.0f / dpiY;
	return D2D1::Point2F(width, height);
}
