#pragma once

#include "IHitTestListener.h"

#include <afx.h>
#include <afxwin.h>
#include <atltypes.h>

#include <d2d1.h>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <Windows.h>
#include <wrl/client.h>
#include <afxcmn.h>

namespace SvgLib::Graphics
{
	class Devices;
	class Renderer;
}

namespace SvgLib::Window
{
	class Window : public CWnd
	{
	public:
		Window();	
		virtual ~Window();

		HRESULT Initialize(CWnd* pParent, const CRect& rect);
	
		void SetEventListener(IEventListener* listener) { listener_ = listener; }

		void SetGraphicToLoad(std::wstring filename);
	protected:
	
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnPaint();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);

		afx_msg void OnMenuRange(UINT nID);

		D2D1_POINT_2F GetClientPixelInDPI();
	
		// Enables dynamic creation and access to run-time class information 
		DECLARE_DYNCREATE(Window);
		DECLARE_MESSAGE_MAP()
	
	private:
		BOOL CreateInPlace(CWnd* pParent, const CRect& rect);

	private:
		IEventListener* listener_ = nullptr;
		std::unique_ptr<SvgLib::Graphics::Devices> devices_;
		std::unique_ptr<SvgLib::Graphics::Renderer> renderer_;
		
		BOOL mouseOver_ = FALSE;
		std::wstring mouseOverId_;
		bool isInitialized_;
		//bool isGettingDestroyed_;
	};

}