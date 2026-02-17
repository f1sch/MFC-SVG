#include "pch.h"

#include "Renderer.h"

#include "DebugLogger.h"
#include "Devices.h"

#include <d2d1helper.h>
#include <d2d1.h>
#include <dcommon.h>
#include <algorithm>
#include <format>
#include <memory>
#include <string>
//#include <Windows.h>
#include <wrl/client.h>
#include <cassert>

#pragma comment(lib, "d2d1.lib")

using namespace SvgLib::Core;
using namespace SvgLib::Graphics;
using namespace SvgLib::Parser;

// ***************************************************
// **************** Window Management ****************
// ***************************************************

std::wstring Renderer::HitTesting2D(float x, float y) const
{
	//float dpiX = 96.0f, dpiY = 96.0f;
	//devices_->GetD2DDeviceContext()->GetDpi(&dpiX, &dpiY);
	//D2D1_POINT_2F pt{ 
	//	x * 96.0f / dpiX, 
	//	y * 96.0f / dpiY 
	//};
	D2D1_POINT_2F pt{x, y};

	const auto& cache = svgCache_->GetCache();
	//for (const auto& geometry : svgCache_->GetCache())
	for (auto it = cache.rbegin(); it != cache.rend(); ++it)
	{
		const auto& geometry = *it;
		if (!geometry.geometry) continue;

		D2D1_MATRIX_3X2_F total = geometry.transform * worldTransform_;
		
		D2D1_RECT_F bounds{};
		geometry.geometry->GetBounds(&total, &bounds);
		if (pt.x < bounds.left || pt.x > bounds.right || pt.y < bounds.top || pt.y > bounds.bottom)
			continue;

		BOOL hitFill = FALSE;//, hitStroke = FALSE;

		HRESULT hr = geometry.geometry->FillContainsPoint(
			pt,
			&total,//worldTransform_,
			0.25f,
			&hitFill
		);
		if (FAILED(hr))
		{
			DebugLogger::Warning("Hittesting for Geometry Fill check failed");
			continue;
		}

		//if (!hitFill)
		//{
		//	hr = geometry.geometry->StrokeContainsPoint(
		//		pt,
		//		0,
		//		nullptr,
		//		&total,
		//		&hitStroke
		//	);
		//	if (FAILED(hr))
		//	{
		//		DebugLogger::Warning("Hittesting for Geometry Stroke check failed");
		//		continue;
		//	}
		//
		//}
		if (hitFill /* || hitStroke*/)
			return geometry.id;
	}
	//DebugLogger::Info("No Geometry was hit");
	return {};
}

const SvgCache::CacheEntry* Renderer::HitTest(const D2D1_POINT_2F& p) const
{
	const auto& cache = svgCache_->GetCache();

	for (const auto& entry : cache)
	{
		BOOL hit = FALSE;
		HRESULT hr = entry.geometry->FillContainsPoint(
			p, worldTransform_, &hit
		);

		if (SUCCEEDED(hr) && hit)
			return &entry;
	}
	return nullptr;
}

void Renderer::ToggleElementDisplay(PCWSTR id)
{
	if (devices_->GetD2DSvgDocument())
	{
		Microsoft::WRL::ComPtr<ID2D1SvgElement> toggleElement = nullptr;
		// style="display
		HRESULT hr = devices_->GetD2DSvgDocument()->FindElementById(id, &toggleElement);
		if (FAILED(hr))
		{
			DebugLogger::Warning(std::format(L"Failed to get Element {}", id));
			return;
			//DebugLogger::Warning("Failed to get Element");
		}
		if (!toggleElement)
			return;

		D2D1_SVG_DISPLAY displayValue = {};
		hr = toggleElement->GetAttributeValue(L"display", &displayValue);
		if (FAILED(hr))
		{
			DebugLogger::Warning(std::format(L"Failed to get display value from {}", id));
			//DebugLogger::Warning("Failed to get display value");
		}
		if (displayValue < 0)
			return;
		else if (displayValue == D2D1_SVG_DISPLAY::D2D1_SVG_DISPLAY_NONE)
		{
			displayValue = D2D1_SVG_DISPLAY::D2D1_SVG_DISPLAY_INLINE;
		}
		else
		{
			displayValue = D2D1_SVG_DISPLAY::D2D1_SVG_DISPLAY_NONE;
		}
		hr = toggleElement->SetAttributeValue(L"display", displayValue);
		if (FAILED(hr))
		{
			DebugLogger::Warning(std::format(L"Failed to set display value for {}", id));
			//DebugLogger::Warning("Failed to set display value ");
		}
	}
}

void Renderer::PrepareLayout(float width, float height)
{
	float bitmapWidth = {};
	float bitmapHeight = {};

	if (devices_->HasBitmap())
	{
		bitmapWidth = devices_->GetD2DLoadedBitmap()->GetSize().width;
		bitmapHeight = devices_->GetD2DLoadedBitmap()->GetSize().height;
	}
	else
	{
		bitmapWidth = svgCache_->GetSvgElementInfo().width;
		bitmapHeight = svgCache_->GetSvgElementInfo().height;
	}

	// calculate scaling factors to maintain aspect ratio
	if (bitmapWidth <= 0 || bitmapHeight <= 0)
	{
		scale_ = 1.0f;
		offset_ = { 0,0 };
		worldTransform_ = D2D1::Matrix3x2F::Identity();
		return;
	}

	float scaleX = width / bitmapWidth;
	float scaleY = height / bitmapHeight;
	scale_ = (std::min)(scaleX, scaleY);
	

	// calculate destination rectangle to center the scaled image
	float scaledWidth = bitmapWidth * scale_;
	float scaledHeight = bitmapHeight * scale_;
	offset_.x = (width - scaledWidth) / 2.0f;
	offset_.y = (height - scaledHeight) / 2.0f;
	
	// Set transform for bitmap and shapes
	//worldTransform_ =
	//	D2D1::Matrix3x2F::Translation(offset_.x, offset_.y) *
	//	D2D1::Matrix3x2F::Scale(scale_, scale_);
	worldTransform_ =
		D2D1::Matrix3x2F::Scale(scale_, scale_) *
	D2D1::Matrix3x2F::Translation(offset_.x, offset_.y);
}

HRESULT Renderer::Render(const RECT& paintRc)
{	
	if (!devices_ || !devices_->IsReady())
		return S_FALSE;
	if (state_ != RendererState::Ready)
		return S_FALSE;

	if (cacheNeedsRebuild)
	{
		svgCache_->LoadCache(devices_->GetD2DSvgDocument());
		cacheNeedsRebuild = false;
	}

#ifdef DEBUG
	assert(!inDraw_ && "Render called while already in a draw!");
	inDraw_ = true;
#endif // DEBUG

	auto ctx = devices_->GetD2DDeviceContext();
	if (!ctx || !devices_->GetDXGISwapChain())
		return S_FALSE;

	D2D1_TAG tag1 = 0, tag2 = 0;
	ctx->SetTags(tag1, tag2);

	if (!devices_->HasRenderTarget())
		return S_FALSE;
	ctx->BeginDraw();
	tag1++;

	ctx->SetTransform(D2D1::Matrix3x2F::Identity());

	// Clipping nur sinnvoll f�r Teilinvalidierungen des Renderbereichs
	//D2D1_RECT_F clip;
	//if (paintRc.right <= paintRc.left || paintRc.bottom <= paintRc.top)
	//{
	//	auto s = ctx->GetSize();
	//	clip = D2D1::RectF(0, 0, s.width, s.height);
	//}
	//else
	//{
	//	clip = D2D1::RectF(
	//		(FLOAT)paintRc.left, (FLOAT)paintRc.top,
	//		(FLOAT)paintRc.right, (FLOAT)paintRc.bottom
	//	);
	//}
	//ctx->PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	
	ctx->Clear(D2D1::ColorF(D2D1::ColorF::White));
	tag1++;

	D2D1_SIZE_F renderTargetSize = ctx->GetSize();
	PrepareLayout(renderTargetSize.width, renderTargetSize.height);
	
	ctx->SetTransform(worldTransform_);
	tag1++;
	
	// v **** static rendering **** v
	// Render Bitmap
	if (devices_->HasBitmap() && devices_->GetD2DLoadedBitmap())
	{
		D2D1_SIZE_F bitmapSize = devices_->GetD2DLoadedBitmap()->GetSize();
		ctx->DrawBitmap(
			devices_->GetD2DLoadedBitmap(),
			D2D1::RectF(0, 0, bitmapSize.width, bitmapSize.height),
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
		);
		tag1++;
	}
	// Render SVG Document
	if (devices_->GetD2DSvgDocument() && svgDocRendered_)
	{
		//ToggleElementDisplay(L"Head"); // testing
		ctx->DrawSvgDocument(devices_->GetD2DSvgDocument());
	}
	// Render Geometries
	D2D1_MATRIX_3X2_F current = worldTransform_;
	if (!svgCache_->IsEmpty() && svgGeomsRendered_)
	{
		for (const auto& geometry : svgCache_->GetCache())
		{
			//auto total = geometry.transform * worldTransform_;
			//if (memcmp(&total, &current, sizeof(total)) != 0)
			//{
			//	ctx->SetTransform(total);
			//	current = total;
			//}
			auto total = geometry.transform * worldTransform_;
			ctx->SetTransform(total);
			if (geometry.id == hoveredId_) {
				ctx->DrawGeometry(
					geometry.geometry.Get(),
					devices_->GetHighlightBrush(),
					5.0f
				);
			}
			else 
			{
				ctx->DrawGeometry(
					geometry.geometry.Get(),
					devices_->GetBlackBrush(),
					2.0f
				);
			}
			tag1++;
		}
	}
	// ^ **** static rendering **** ^
	
	ctx->SetTransform(D2D1::Matrix3x2F::Identity());
	//ctx->PopAxisAlignedClip();
	tag1++;
	
	HRESULT hr = ctx->EndDraw(&tag1, &tag2);

#ifdef DEBUG
	inDraw_ = false;
#endif // DEBUG
	
	if (hr == D2DERR_RECREATE_TARGET)
	{
		DebugLogger::Warning("Presentation Error: Caller needs to recreate Rendertarget");
		// alle D2D-Ressourcen freigeben, die von ID2D1RenderTarget abh�ngen (Bitmaps, Brushes, Layers �)
		// RenderTarget/SwapChain/Context neu aufbauen.
		
		if (!devices_->IsShuttingDown())
		{
			devices_->RequestRecreateTarget();
			return hr;
		}
	}
	if (FAILED(hr))
	{
		DebugLogger::Warning(std::format("DrawScene failed at {}", tag1));
		return hr;
	}

	hr = devices_->GetDXGISwapChain()->Present(1, 0); //Present1()
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		devices_->RequestDeviceLost();
		return hr;
	}

	return hr;
}

D2D1_POINT_2F Renderer::TransformPoint(const D2D1_POINT_2F& point) const
{
	return D2D1::Point2F(
		point.x * scale_ + offset_.x,
		point.y * scale_ + offset_.y
	);
}

void SvgLib::Graphics::Renderer::SetStyleAttribute(
	ID2D1SvgElement* element, 
	std::wstring attribute, 
	D2D1::ColorF color)
{
	// TODO:
	//element->SetAttributeValue(attribute, color);
}

void Renderer::OnMouseMove(int x, int y)
{
	mousePos_ = D2D1::Point2F(static_cast<FLOAT>(x), static_cast<FLOAT>(y));
	//DebugLogger::Info(std::string("Mouse move: %d,%d\n", x, y));
}

std::wstring Renderer::OnMouseDown(D2D1_POINT_2F point) const
{
	DebugLogger::Info(std::format("Point clicked: [{}|{}]", point.x, point.y));
	return HitTesting2D(point.x, point.y);
}

// ******************************************
// **************** Direct2D ****************
// ******************************************

Renderer::Renderer(Devices* devices)
	:
	devices_(devices),
	scale_(1.0f),
	currentScale_(1.0f),
	currentOffset_(D2D1::Point2F(0.0f, 0.0f)),
	offset_(D2D1::Point2F(0.0f, 0.0f)),
	mousePos_(D2D1::Point2F(0.0f, 0.0f)),
	worldTransform_({ D2D1::Matrix3x2F::Identity()})
{
	svgCache_ = std::make_unique<SvgCache>(devices->GetD2DFactory(), devices->GetD2DDeviceContext());
	devices_->RegisterDeviceNotify(this);
}

Renderer::~Renderer()
{
	if (devices_)
		devices_->RegisterDeviceNotify(nullptr);
	state_ = RendererState::Uninitialized;
}

void SvgLib::Graphics::Renderer::InitializeRenderer()
{
	state_ = RendererState::Initializing;

	CreateDeviceIndependentResources();
	CreateDeviceDependentResources();

	state_ = RendererState::Ready;
}

void Renderer::ReleaseDeviceDependentResources()
{
	svgCache_->ClearCache();
	cacheNeedsRebuild = true;
	//svgCache_.release();
	//devices_ = nullptr;
	//devices_.reset();
}

void Renderer::CreateDeviceIndependentResources()
{
	
}

void Renderer::CreateDeviceDependentResources()
{
	//if (!devices_->IsReady()) return;
	//
	//if (devices_->GetD2DSvgDocument() && svgCache_->IsEmpty())
	//	svgCache_->LoadCache(devices_->GetD2DSvgDocument());
	cacheNeedsRebuild = true;
}

void Renderer::OnDeviceLost()
{
	ReleaseDeviceDependentResources();
}

void Renderer::OnDeviceRestored()
{
	CreateDeviceDependentResources();
}

/*

HRESULT Renderer::CreateGeometry()
{
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<ID2D1SvgElement> root;
	devices_->GetD2DSvgDocument()->GetRoot(&root);



	// <form>-Elemente lesen

	// <form>-Elemente in ID2D1Geometry umwandeln

	// <form>-Elemente in cachedGeometry speichern

	return hr;
}
*/

// FIX ME: unused
HRESULT Renderer::UpdateGeometryScale(
	ID2D1PathGeometry* inGeometry, 
	float scale, 
	ID2D1TransformedGeometry** outGeometry) 
{
	// Create a transform matrix
	D2D1::Matrix3x2F scaleMatrix = D2D1::Matrix3x2F::Scale(scale, scale);
	D2D1::Matrix3x2F translateMatrix = D2D1::Matrix3x2F::Translation(
		currentOffset_.x, currentOffset_.y);
	D2D1::Matrix3x2F transform = scaleMatrix * translateMatrix;

	// Apply the transform to the geometry
	//Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> transformedGeometry;
	HRESULT hr = S_FALSE;
	hr = devices_->GetD2DFactory()->CreateTransformedGeometry(
		inGeometry,
		transform,
		outGeometry
	);
	if (FAILED(hr))
		DebugLogger::Warning("Failed to update geometry scale");

	return hr;
}
