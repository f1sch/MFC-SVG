#pragma once
#include <d2d1_3.h>
#include <d2d1svg.h>
#include <d3d11_3.h>
#include <d3dcommon.h>
#include <dwrite_3.h>
#include <dxgi.h>
#include <dxgi1_2.h>


#include <afxwin.h>
#include <vector>
#include <wrl/client.h>
#include <wincodec.h>
#include <Windows.h>

#include "IDeviceNotify.h"

namespace SvgLib::Graphics
{
	class Devices 
	{
		enum class DeviceState { Uninitialized, Initializing, Ready, Failed, ShuttingDown, Lost };
	public:
		Devices();
		~Devices();

		HRESULT Initialize(HWND hwnd);

		void DiscardEverything();

		void RegisterDeviceNotify(IDeviceNotify* deviceNotify);

		void RequestDeviceLost() { deviceLostPending_ = true; }
		void RequestRecreateTarget() { recreateTargetPending_ = true; }
		void RequestResize() { resizePending_ = true; }
		HRESULT ProcessPending();

		DeviceState GetState() const { return state_; }
		bool IsReady() const { return state_ == DeviceState::Ready; }
		bool IsShuttingDown() const { return state_ == DeviceState::ShuttingDown; }
		void BeginShutdown();

		bool HasRenderTarget() const;
		bool HasBitmap() const { return hasBitmap_; }

		void SetFilePathForImageLoading(const wchar_t* fileName);
		void SetFilePathForSvgLoading(std::wstring fileName);

		// Debug
		void ReportLiveDXObjects(int when);

		// Getters
		HWND GetHwnd() const { return hwnd_; }

		ID3D11Device* GetD3DDevice() const { return d3dDevice_.Get(); }
		ID3D11DeviceContext* GetD3DDeviceContext() const { return d3dDeviceContext_.Get(); }

		IDXGISwapChain1* GetDXGISwapChain() const { return dxgiSwapchain_.Get(); }

		ID2D1Factory6* GetD2DFactory() const { return d2dFactory_.Get(); }
		ID2D1Device5* GetD2DDevice() const { return d2dDevice_.Get(); }
		ID2D1DeviceContext5* GetD2DDeviceContext() const { return d2dContext_.Get(); }
		ID2D1Bitmap* GetD2DLoadedBitmap() const { return d2dLoadedBitmap_.Get(); }
		ID2D1Bitmap1* GetD2DSwapChainBackBuffer() const { return d2dSwapChainBackBuffer_.Get(); }

		ID2D1SvgDocument* GetD2DSvgDocument() const { return d2dSvgDocument_.Get(); }

		ID2D1SolidColorBrush* GetBlackBrush() const { return blackBrush_.Get(); }
		ID2D1SolidColorBrush* GetHighlightBrush() const { return highlightBrush_.Get(); }
		ID2D1SolidColorBrush* GetBrush() const { return brush_.Get(); }

		IWICImagingFactory2* GetWICFactory() const { return wicFactory_.Get(); }
		IDWriteFactory2* GetDWriteFactory() const { return dwriteFactory_.Get(); }

	protected:
		// TODO: void BeginShutdown();

	private:
		HRESULT CreateDeviceIndependentResources();
		HRESULT CreateDeviceResources();
		HRESULT CreateWindowSizeDependentResources();

		HRESULT CreateSwapChain(UINT width, UINT height);
		
		HRESULT CreateSvgDocument();
		HRESULT CreateSvgDocument(Microsoft::WRL::ComPtr<IStream> stream);
		HRESULT CreateBrushes();

		HRESULT SetSwapChainBackBuffer();
		HRESULT ResetAndResizeSwapchain(UINT cx, UINT cy);
		
		void DiscardDeviceIndependentResources();
		void DiscardDeviceResources();
		void DiscardWindowSizeDependentResources();
		
		HRESULT HandleDeviceLost();

		HRESULT LoadImageIntoBitmap(const wchar_t* imgPath);
		
	private:
		HWND hwnd_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D11Device3> d3dDevice_;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext4> d3dDeviceContext_;
	
		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgiSwapchain_;
		
		Microsoft::WRL::ComPtr<ID2D1Factory6> d2dFactory_;
		Microsoft::WRL::ComPtr<ID2D1Device5> d2dDevice_;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext5> d2dContext_;

		Microsoft::WRL::ComPtr<ID2D1Bitmap> d2dLoadedBitmap_;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dSwapChainBackBuffer_;
		Microsoft::WRL::ComPtr<ID2D1SvgDocument> d2dSvgDocument_;
		
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush_;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> highlightBrush_;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_;
	
		Microsoft::WRL::ComPtr<IWICImagingFactory2> wicFactory_;
		Microsoft::WRL::ComPtr<IDWriteFactory2> dwriteFactory_;

		//std::wstring svgToLoad_ = L""; 
		
		std::wstring svgToLoad_ = L"../svglib/humanBodyFront.svg"; 
		
		//const wchar_t* svgToLoad_ = L"../svglib/humanBodyFront.svg"; 
		//const wchar_t* svgToLoad_ = L"../svglib/humanBodyFrontTEST.svg"; 
		//const wchar_t* svgToLoad_ = L"../svglib/kartesianGraph.svg"; // For testing

		//const wchar_t* svgToLoad_ = L"../svglib/d2d_transform_test_groups.svg"; // For testing
		//const wchar_t* svgToLoad_ = L"../svglib/d2d_stress_test_01_paths.svg"; // For testing
		//const wchar_t* svgToLoad_ = L"../svglib/d2d_stress_test_01_paths_supported.svg"; // For testing
		//const wchar_t* svgToLoad_ = L"../svglib/d2d_stress_test_02_strokes_transforms.svg"; // For testing
		//const wchar_t* svgToLoad_ = L"../svglib/d2d_stress_test_03_clip.svg"; // For testing
		//const wchar_t* svgToLoad_ = L"../svglib/d2d_stress_test_03_clip_mask.svg"; // For testing
		
		//const wchar_t* svgToLoad_ = L"../svglib/testSVGElements.svg"; // For testing
		//const wchar_t* svgToLoad_ = L"../svglib/testingParserGenerated.svg"; // For testing
		
		std::atomic<bool> deviceLostPending_{ false };
		std::atomic<bool> recreateTargetPending_{ false };
		std::atomic<bool> resizePending_{ false };

		DeviceState state_;
		//bool comOwned_ = false;
		const wchar_t* fileName_;
		bool hasBitmap_;
		float dpi_;
		IDeviceNotify* deviceNotify_;
	};
}