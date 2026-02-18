#pragma once
#include <optional>

namespace SvgLib::Parser
{
	class SvgCache
	{
		struct RenderState
		{
			D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Identity();
		};

	public:
		struct CacheEntry
		{
			Microsoft::WRL::ComPtr<ID2D1Geometry> geometry{};
			std::wstring id{};
			D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Identity();
			std::optional<std::wstring> groupId{};
		};

		struct SvgElementInfo
		{
			float width{};
			float height{};
			D2D1_SVG_PRESERVE_ASPECT_RATIO preserveAspectRatio{};
			std::wstring id{};
		};



		SvgCache() = default;
		SvgCache(ID2D1Factory* factory, ID2D1DeviceContext* dc)
			: factory_(factory), deviceContext_(dc)
		{ }
		~SvgCache() = default;

		void LoadCache(ID2D1SvgDocument* doc);
		void ClearCache();
		bool IsEmpty();
		
		const std::vector<CacheEntry>& GetCache() const { return cache_; }
		const CacheEntry* GetCacheEntryById(std::wstring_view& id) const;

		const SvgElementInfo& GetImageElementInfo() const { return image_; }
		const SvgElementInfo& GetSvgElementInfo() const { return svg_; }

	private:
		void TraverseElement(ID2D1SvgElement* element, const D2D1_MATRIX_3X2_F& parentMatrix);
		D2D1_MATRIX_3X2_F HandleElement(ID2D1SvgElement* element, const D2D1_MATRIX_3X2_F& parentMatrix);

		Microsoft::WRL::ComPtr<ID2D1Geometry> CreateGeometryFromPath(ID2D1SvgElement* element);
		Microsoft::WRL::ComPtr<ID2D1Geometry> CreateGeometryFromCircle(ID2D1SvgElement* element);
		Microsoft::WRL::ComPtr<ID2D1Geometry> CreateGeometryFromRect(ID2D1SvgElement* element);
		Microsoft::WRL::ComPtr<ID2D1Geometry> CreateGeometryFromEllipse(ID2D1SvgElement* element);
		Microsoft::WRL::ComPtr<ID2D1Geometry> CreateGeometryFromLine(ID2D1SvgElement* element);
		Microsoft::WRL::ComPtr<ID2D1Geometry> CreateGeometryFromPolyline(ID2D1SvgElement* element);
		Microsoft::WRL::ComPtr<ID2D1Geometry> CreateGeometryFromPolygon(ID2D1SvgElement* element);
		
		void ReadSvgElement(ID2D1SvgElement* element, SvgElementInfo& elemInfo);
		void ReadImageElement(ID2D1SvgElement* element);
		void ReadGroupElement(ID2D1SvgElement* element, const RenderState& parent);

		D2D1_MATRIX_3X2_F ReadTransformOrIdentity(ID2D1SvgElement* element);

		void MapSvgCommandsToSink(
			const std::vector<D2D1_SVG_PATH_COMMAND>& cmds,
			const std::vector<float>& data,
			ID2D1GeometrySink* sink
		);

	private:
		ID2D1Factory* factory_ = nullptr;
		ID2D1DeviceContext* deviceContext_ = nullptr;

		std::vector<CacheEntry> cache_;
		SvgElementInfo image_;
		SvgElementInfo svg_;
	};
}