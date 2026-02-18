#include "pch.h"
#include "Devices.h"
#include "DebugLogger.h"
#include "DxDebugName.h"

#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <d2d1_1helper.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <dcommon.h>
#include <dwrite_3.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <dxgiformat.h>
#include <afxwin.h>
#include <combaseapi.h>
#include <objbase.h>
#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace SvgLib::Core;
using namespace SvgLib::Graphics;

Devices::Devices() :
	state_(DeviceState::Uninitialized),
	dpi_(96.0f),
	hasBitmap_(false),
	fileName_(L""),
	deviceNotify_(nullptr)
{
}

Devices::~Devices()
{	
	if (state_ != DeviceState::ShuttingDown)
	{
		state_ = DeviceState::ShuttingDown;
		
		DiscardWindowSizeDependentResources();
		DiscardDeviceResources();
		DiscardDeviceIndependentResources();
	}
}

HRESULT Devices::Initialize(HWND hwnd)
{
	state_ = DeviceState::Initializing;

	hwnd_ = hwnd;
	
	auto fail = [&](HRESULT e) {
		DiscardWindowSizeDependentResources();
		DiscardDeviceResources();
		DiscardDeviceIndependentResources();
		state_ = DeviceState::Failed;
		return e;
	};

	HRESULT hr = CreateDeviceIndependentResources();
	if (FAILED(hr)) return fail(hr);
	
	hr = CreateDeviceResources();
	if (FAILED(hr)) return fail(hr);
	
	hr = CreateWindowSizeDependentResources();
	if (FAILED(hr)) return fail(hr);

	state_ = DeviceState::Ready;
	
	return S_OK;
}

HRESULT Devices::CreateDeviceIndependentResources()
{
	HRESULT hr = S_FALSE;

	D2D1_FACTORY_OPTIONS options = {};
#if defined(_DEBUG)
	// DO NOT USE IF GRAPHIC TOOLS ARE NOT INSTALLED
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif // defined(_DEBUG)

	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory6),
		&options,
		reinterpret_cast<void**>(d2dFactory_.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to create d2DFactory");
		return hr;
	}

	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory2),
		reinterpret_cast<IUnknown**>(dwriteFactory_.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to create DWrite factory");
		return hr;
	}

	hr = CoCreateInstance(
		CLSID_WICImagingFactory2,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&wicFactory_)
	);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to CoCreateinstance() for WICImagingFactory");
		return hr;
	}

	return hr;
}

HRESULT Devices::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

// Requires Windows Graphic Tools
#if defined(_DEBUG)
	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_NULL,
		0,
		D3D11_CREATE_DEVICE_DEBUG,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		nullptr, nullptr, nullptr
	);
	if (SUCCEEDED(hr))
	{
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif // DEBUG

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		device.ReleaseAndGetAddressOf(),
		//&d3dFeatureLevel_,
		nullptr,
		context.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
	{
		// Fall back to WARP device
		hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			0,
			creationFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			device.ReleaseAndGetAddressOf(),
			//&d3dFeatureLevel_,
			nullptr,
			context.ReleaseAndGetAddressOf()
		);
		DebugLogger::Error("Failed to create d3d11-Device");
		return hr;
	}

	hr = device.As(&d3dDevice_);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to query local D3DDevice");
		return hr;
	}

	hr = context.As(&d3dDeviceContext_);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to query local D3DDeviceContext");
		return hr;
	}

	Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
	hr = d3dDevice_.As(&dxgiDevice);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to query dxgi-Device");
		return hr;
	}

	hr = d2dFactory_->CreateDevice(dxgiDevice.Get(), d2dDevice_.GetAddressOf());
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to create dxgi-Device");
		return hr;
	}
	
	hr = d2dDevice_->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE, 
		d2dContext_.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to create d2dDeviceContext");
		return hr;
	}
	d2dContext_->SetDpi(dpi_, dpi_);

	hr = CreateSvgDocument();
	if (FAILED(hr))
	{
		DebugLogger::Warning("Failed to create SvgDocument");
	}

	hr = CreateBrushes();
	if (FAILED(hr))
	{
		DebugLogger::Warning("Failed to create Brushes");
	}

	if (hasBitmap_)
	{
		hr = LoadImageIntoBitmap(L"../svglib/humanBodyFront.png");
		if (FAILED(hr))
		{
			DebugLogger::Warning("Failed to load bitmap");
		}
	}
	return hr;
}

HRESULT Devices::CreateSwapChain(UINT width, UINT height)
{
	// Create
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
	hr = d3dDevice_.As(&dxgiDevice);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to query dxgi-Device");
		return hr;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to get dxgi Adapter");
		return hr;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to GetParent of dxgi-Factory");
		return hr;
	}

	hr = dxgiFactory->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(hr))
	{
		DebugLogger::Warning("Cannot set prevention for alt+enter toggle in dxgiFactory");
	}

	DXGI_SWAP_CHAIN_DESC1 scDesc = { 0 };
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	scDesc.Flags = 0;
	scDesc.Scaling = DXGI_SCALING_STRETCH;//DXGI_SCALING_NONE; // TODO: Bild skalierung hier?
	scDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	hr = dxgiFactory->CreateSwapChainForHwnd(
		d3dDevice_.Get(),
		hwnd_,
		&scDesc,
		nullptr,
		nullptr,
		dxgiSwapchain_.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to create Swapchain for Hwnd");
		return hr;
	}

	hr = SetSwapChainBackBuffer();
	if (FAILED(hr))
	{
		DebugLogger::Warning("Failed to set SwapChainBackBuffer in CreateWindowSizeDependentResources");
		return hr;
	}

	return hr;
}

HRESULT Devices::CreateWindowSizeDependentResources()
{
	// Calculate the necessary swap chain and render target size in pixels
	RECT rc;
	::GetClientRect(hwnd_, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	if (width == 0 || height == 0)
	{
		DiscardWindowSizeDependentResources();
		return S_FALSE;
	}
	
	// Prevent zero size DirectX content from being created
	width = max(width, 1);
	height = max(height, 1);
	

	HRESULT hr = S_OK;
	if (dxgiSwapchain_)
	{
		hr = ResetAndResizeSwapchain(width, height);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			return HandleDeviceLost();
		}
		if (FAILED(hr))
			return hr;
	}
	else
	{
		hr = CreateSwapChain(width, height);
		if (FAILED(hr))
			return hr;
	}
	return S_OK;
}

void Devices::DiscardEverything()
{
	DiscardWindowSizeDependentResources();
	DiscardDeviceResources();
	DiscardDeviceIndependentResources();
}

void Devices::DiscardDeviceIndependentResources()
{
	wicFactory_.Reset();
	dwriteFactory_.Reset();
	d2dFactory_.Reset();
}

void Devices::DiscardDeviceResources()
{
	if (d2dContext_)
	{
		d2dContext_->SetTarget(nullptr);
	}
	
	d2dContext_.Reset();
	d2dDevice_.Reset();
	
	if (d3dDeviceContext_)
	{
		if (state_ != DeviceState::ShuttingDown)
		{
			d3dDeviceContext_->ClearState();
			d3dDeviceContext_->Flush();
		}
	}
	
	d3dDeviceContext_.Reset();
	d3dDevice_.Reset();
}
void Devices::DiscardWindowSizeDependentResources()
{
	if (d2dContext_)
	{
		d2dContext_->SetTarget(nullptr);
	}

	d2dSwapChainBackBuffer_.Reset();
	dxgiSwapchain_.Reset();

	blackBrush_.Reset();
	highlightBrush_.Reset();
	brush_.Reset();

	d2dSvgDocument_.Reset();
	d2dLoadedBitmap_.Reset();
}

HRESULT Devices::CreateSvgDocument()
{
	RECT rect;
	GetClientRect(hwnd_, &rect);
	float width = static_cast<FLOAT>(rect.right - rect.left);
	float height = static_cast<FLOAT>(rect.bottom - rect.top);
	Microsoft::WRL::ComPtr<IStream> iStream;
	SHCreateStreamOnFile(svgToLoad_.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE, iStream.GetAddressOf());
	
	HRESULT hr = d2dContext_->CreateSvgDocument(iStream.Get(), D2D1::SizeF(width, height), d2dSvgDocument_.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		DebugLogger::Warning("Failed to create svg doc");
	}
	return hr;
}

HRESULT Devices::CreateSvgDocument(Microsoft::WRL::ComPtr<IStream> stream)
{
	RECT rect;
	GetClientRect(hwnd_, &rect);
	float width = static_cast<FLOAT>(rect.right - rect.left);
	float height = static_cast<FLOAT>(rect.bottom - rect.top);
	SHCreateStreamOnFile(svgToLoad_.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE, stream.GetAddressOf());
	HRESULT hr = d2dContext_->CreateSvgDocument(stream.Get(), D2D1::SizeF(width, height), d2dSvgDocument_.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		DebugLogger::Warning("Failed to create svg doc from given stream");
	}
	return hr;
}

HRESULT Devices::ProcessPending()
{
	if (deviceLostPending_.exchange(false))
		return HandleDeviceLost();

	if (recreateTargetPending_.exchange(false) || resizePending_.exchange(false))
		return CreateWindowSizeDependentResources();

	return S_OK;
}

void Devices::BeginShutdown()
{
	state_ = DeviceState::ShuttingDown;
}

bool Devices::HasRenderTarget() const
{
	if (!d2dContext_ || !dxgiSwapchain_ || !d2dSwapChainBackBuffer_)
		return false;

	Microsoft::WRL::ComPtr<ID2D1Image> target;
	d2dContext_->GetTarget(target.GetAddressOf());
	return target.Get() == d2dSwapChainBackBuffer_.Get();
}

void Devices::SetFilePathForImageLoading(const wchar_t* fileName)
{
	fileName_ = fileName;
	hasBitmap_ = true;
}

void SvgLib::Graphics::Devices::SetFilePathForSvgLoading(std::wstring fileName)
{
	// Store new SVG path.
	svgToLoad_ = std::move(fileName);

	if (state_ == DeviceState::Ready)
	{
		RequestDeviceLost();
	}
}

HRESULT Devices::ResetAndResizeSwapchain(UINT cx, UINT cy)
{
	// Unbind D2D1DeviceContext
	if (d2dContext_) d2dContext_->SetTarget(nullptr);

	// Release dxgi back buffer (IDXGISurface) 
	d2dSwapChainBackBuffer_.Reset();

	// Unbind D3D11DeviceContext
	if (d3dDeviceContext_) d3dDeviceContext_->ClearState();

	HRESULT hr = dxgiSwapchain_->ResizeBuffers( 0, cx, cy, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		// If the device was removed for any reason, a new device and swap chain will need to be created.
		// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
		// and correctly set up the new device.
		return hr;
	}

	hr = SetSwapChainBackBuffer();
	if (FAILED(hr))
	{
		DebugLogger::Warning("SetSwapChainBackBuffer failed in ResizeSwapChain");
		return hr;
	}

	return S_OK;
}

HRESULT Devices::HandleDeviceLost()
{
	if (state_ == DeviceState::Initializing || state_ == DeviceState::Lost) 
		return S_FALSE;
	
	state_ = DeviceState::Lost;
	if (deviceNotify_)
	{
		deviceNotify_->OnDeviceLost();
	}

	DiscardWindowSizeDependentResources();
	DiscardDeviceResources();
	
	state_ = DeviceState::Initializing;
	
	HRESULT hr = CreateDeviceResources();
	if (FAILED(hr)) 
	{ 
		state_ = DeviceState::Failed; 
		return hr; 
	}
	
	hr = CreateWindowSizeDependentResources();
	if (FAILED(hr)) 
	{ 
		state_ = DeviceState::Failed; 
		return hr; 
	}
	
	state_ = DeviceState::Ready;

	if (deviceNotify_)
	{
		deviceNotify_->OnDeviceRestored();
	}
	return S_OK;
}

HRESULT Devices::SetSwapChainBackBuffer()
{
	d2dContext_->SetTarget(nullptr);
	d2dSwapChainBackBuffer_.Reset();

	HRESULT hr = S_FALSE;
	Microsoft::WRL::ComPtr<IDXGISurface2> dxgiSurface;
	hr = dxgiSwapchain_->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to get Buffer for DXGISwapchain");
		return hr;
	}

	dpi_ = GetDpiForWindow(hwnd_);

	auto bmpProps =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE/*D2D1_ALPHA_MODE_PREMULTIPLIED*/),
			dpi_, dpi_
		);
	
	hr = d2dContext_->CreateBitmapFromDxgiSurface(
		dxgiSurface.Get(),
		&bmpProps,
		d2dSwapChainBackBuffer_.GetAddressOf()
	);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to create Bitmap from DXGISurface");
		return hr;
	}
	
	d2dContext_->SetTarget(d2dSwapChainBackBuffer_.Get());
	d2dContext_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
	return S_OK;
}

HRESULT Devices::LoadImageIntoBitmap(const wchar_t* imgPath)
{
	HRESULT hr = S_FALSE;

	Microsoft::WRL::ComPtr<IWICBitmapDecoder> pDecoder;
	hr = wicFactory_->CreateDecoderFromFilename(
		imgPath,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		pDecoder.GetAddressOf()
	);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to create decoder from filename");
		return hr;
	}

	Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> pFrame;
	hr = pDecoder->GetFrame(0, pFrame.GetAddressOf());
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to get frame for IWICBitmapDecoder");
		return hr;
	}

	Microsoft::WRL::ComPtr<IWICFormatConverter> pConverter;
	hr = wicFactory_->CreateFormatConverter(pConverter.GetAddressOf());
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to create format converter");
		return hr;
	}

	hr = pConverter->Initialize(
		pFrame.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.f,
		WICBitmapPaletteTypeMedianCut
	);
	if (FAILED(hr))
	{
		DebugLogger::Error("Failed to Initialize IWICBitmapDecoder");
		return hr;
	}

	hr = d2dContext_->CreateBitmapFromWicBitmap(
		pConverter.Get(),
		nullptr,
		d2dLoadedBitmap_.GetAddressOf()
	);
	if (FAILED(hr))
		DebugLogger::Warning("Failed to create Bitmap from WicBitmap");

	return hr;
}

HRESULT Devices::CreateBrushes()
{
	HRESULT hr = S_FALSE;
	if (!blackBrush_)
	{
		hr = d2dContext_->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Black),
			blackBrush_.ReleaseAndGetAddressOf()
		);
		if (FAILED(hr))
		{
			DebugLogger::Warning("Failed to create black brush");
			return hr;
		}
	}
	if (!highlightBrush_)
	{
		hr = d2dContext_->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Red),
			highlightBrush_.ReleaseAndGetAddressOf()
		);
		if (FAILED(hr))
		{
			DebugLogger::Error("Failed to create brush for highlighting geometry outlines");
			return hr;
		}
	}
	if (!brush_)
	{
		hr = d2dContext_->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Green),
			brush_.ReleaseAndGetAddressOf()
		);
		if (FAILED(hr))
		{
			DebugLogger::Error("Failed to create brush for path geometry outlines");
			return hr;
		}
	}
	return S_OK;
}

void Devices::RegisterDeviceNotify(IDeviceNotify* deviceNotify)
{
	deviceNotify_ = deviceNotify;
}

void Devices::ReportLiveDXObjects(int when)
{
	Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
		const wchar_t* whenText = (when < 0) ? L"BEFORE" : L"AFTER";
		wchar_t buffer[128];
		swprintf(buffer, 128, L"***[DXGI Debug Report %s Reset()] ***\n", whenText);
		OutputDebugStringW(buffer);
		//dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		OutputDebugStringW(L"******************************************\n");
	}
}
