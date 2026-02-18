#pragma once

#include "IDeviceNotify.h"
#include "SvgCache.h"

#include <d2d1.h>
#include <d2d1helper.h>
#include <dcommon.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>
//#include <Windows.h>
#include <wrl/client.h>

namespace SvgLib::Parser
{
	struct SvgPathElement;
}

namespace SvgLib::Graphics 
{
	class Devices;

	class Renderer : public IDeviceNotify
	{
		enum class RendererState { Uninitialized, Initializing, Ready, DeviceLost };
	
	public:
		explicit Renderer(Devices* devices);
		~Renderer();
		void InitializeRenderer();
		void ReleaseDeviceDependentResources();

		void CreateDeviceIndependentResources();
		void CreateDeviceDependentResources();
		HRESULT Render(const RECT& paintRc);

		void SetHoveredElement(const std::wstring& id) { hoveredId_ = id; }
		std::wstring_view GetHoveredElement() { return hoveredId_; }

		void SetSvgDocRendered(const bool rendered) { svgDocRendered_ = rendered; }
		void SetSvgGeomsRendered(const bool rendererd) { svgGeomsRendered_ = rendererd; }

		void SetStyleAttribute(ID2D1SvgElement* element, std::wstring attribute, D2D1::ColorF color);
	
		void OnMouseMove(int x, int y);
		std::wstring OnMouseDown(D2D1_POINT_2F point) const;
	
		// IDeviceNotifiy methods handle device lost and restored
		void OnDeviceLost() override;
		void OnDeviceRestored() override;
	
		std::wstring HitTesting2D(float x, float y) const;
		const SvgLib::Parser::SvgCache::CacheEntry* HitTest(const D2D1_POINT_2F& p) const;
	private:
		D2D1_POINT_2F TransformPoint(const D2D1_POINT_2F& point) const;
		void PrepareLayout(float width, float height);
		void ToggleElementDisplay(PCWSTR id);
	
		// FIX ME: unused
		HRESULT UpdateGeometryScale( 
			ID2D1PathGeometry* inGeometry, float scale, ID2D1TransformedGeometry** outGeometry);
		
	private:
		RendererState state_ = RendererState::Uninitialized;

		bool inDraw_ = false; // debug

		bool cacheNeedsRebuild_ = false;
		float scale_;
		float currentScale_;
		D2D1_POINT_2F currentOffset_;
		D2D1_POINT_2F offset_;
		D2D1_POINT_2F mousePos_;
		D2D1_MATRIX_3X2_F worldTransform_;

		std::wstring hoveredId_;
	
		Devices* devices_;

		std::unique_ptr<SvgLib::Parser::SvgCache> svgCache_;

		bool svgDocRendered_ = true;
		bool svgGeomsRendered_ = true;
	};

}