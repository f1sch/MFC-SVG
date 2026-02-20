#include "pch.h"
#include "SvgCache.h"

#include "DebugLogger.h"

#include <algorithm>

using namespace SvgLib::Parser;

void SvgCache::LoadCache(ID2D1SvgDocument* doc)
{
	Microsoft::WRL::ComPtr<ID2D1SvgElement> root;
	doc->GetRoot(root.GetAddressOf());
	if (!root) return;
	
	cache_.clear();
	
	ReadSvgElement(root.Get(), svg_);
	TraverseElement(root.Get(), D2D1::Matrix3x2F::Identity());
}

void SvgCache::ClearCache()
{
	cache_.clear();
}

bool SvgCache::IsEmpty()
{
	return cache_.empty();
}

const SvgCache::CacheEntry* SvgCache::GetCacheEntryById(std::wstring_view& id) const
{
	auto it = std::find_if(cache_.begin(), cache_.end(),
		[id](const CacheEntry& entry) { 
			return entry.id == id; 
		});

	return (it != cache_.end()) ? &(*it) : nullptr;
}

void SvgCache::TraverseElement(ID2D1SvgElement* element, const D2D1_MATRIX_3X2_F& parentMatrix)
{
	if (!element) return;

	Microsoft::WRL::ComPtr<ID2D1SvgElement> current = element;
	const D2D1_MATRIX_3X2_F cumulativeTransform = HandleElement(current.Get(), parentMatrix);

	Microsoft::WRL::ComPtr<ID2D1SvgElement> child;
	element->GetFirstChild(child.GetAddressOf());

	while (child)
	{
		TraverseElement(child.Get(), cumulativeTransform);

		Microsoft::WRL::ComPtr<ID2D1SvgElement> next;
		HRESULT hr = element->GetNextChild(child.Get(), next.GetAddressOf());
		if (FAILED(hr)) break;

		child = next;
	}
}

D2D1_MATRIX_3X2_F SvgCache::HandleElement(ID2D1SvgElement* element, const D2D1_MATRIX_3X2_F& parentMatrix)
{
	UINT32 nameLen = 0;
	nameLen = element->GetTagNameLength();
	std::wstring tag = {};
	if (nameLen > 0)
	{
		tag.resize(nameLen);
		element->GetTagName(tag.data(), nameLen + 1);
	}

	std::wstring id = {};
	if (element->IsAttributeSpecified(L"id", nullptr))
	{
		UINT32 lenChars = 0;
		HRESULT hrLen = element->GetAttributeValueLength(
			L"id",
			D2D1_SVG_ATTRIBUTE_STRING_TYPE_ID,
			&lenChars
		);
		if (SUCCEEDED(hrLen) && lenChars > 0)
		{
			std::wstring buf;
			buf.resize(lenChars + 1);

			HRESULT hrGet = element->GetAttributeValue(
				L"id",
				D2D1_SVG_ATTRIBUTE_STRING_TYPE_ID,
				&buf[0],
				lenChars + 1
			);
			if (SUCCEEDED(hrGet))
			{
				buf[lenChars] = L'\0';
				id = buf.c_str();
			}
		}
	}
	const D2D1_MATRIX_3X2_F local = ReadTransformOrIdentity(element);
	const D2D1_MATRIX_3X2_F cumulative = local * parentMatrix;
	
	using SvgGeometryHandler = Microsoft::WRL::ComPtr<ID2D1Geometry>(SvgCache::*)(ID2D1SvgElement*);
	static const std::unordered_map<std::wstring, SvgGeometryHandler> handlers =
	{
		{L"path",		&SvgCache::CreateGeometryFromPath},
		{L"circle",		&SvgCache::CreateGeometryFromCircle},
		{L"rect",		&SvgCache::CreateGeometryFromRect},
		{L"ellipse",	&SvgCache::CreateGeometryFromEllipse},
		{L"line",		&SvgCache::CreateGeometryFromLine},
		{L"polyline",	&SvgCache::CreateGeometryFromPolyline},
		{L"polygon",	&SvgCache::CreateGeometryFromPolygon}
	};

	if (tag == L"g")
	{
		return cumulative;
	}
	if (tag == L"svg")
	{
		return cumulative;
	}
	if (tag == L"image")
	{
		ReadImageElement(element);
		return cumulative;
	}
	
	Microsoft::WRL::ComPtr<ID2D1Geometry> geometry = nullptr;
	if (auto it = handlers.find(tag); it != handlers.end())
	{
		geometry = (this->*(it->second))(element);
	}
	else
	{
		SvgLib::Core::DebugLogger::Info(std::format(L"Unsupported SVG tag: <{}>", tag));
	}

	if (geometry)
	{
		cache_.push_back(CacheEntry(
			geometry,
			id,
			cumulative
		));
	}
	return cumulative;
}

Microsoft::WRL::ComPtr<ID2D1Geometry> SvgCache::CreateGeometryFromPath(
	ID2D1SvgElement* element)
{
	if (!factory_ || !element) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1SvgPathData> pathData;
	HRESULT hr = element->GetAttributeValue(L"d", pathData.GetAddressOf());
	if (FAILED(hr) || !pathData) return nullptr;

	const UINT32 commandCount = pathData->GetCommandsCount();
	const UINT32 segmentCount = pathData->GetSegmentDataCount();
	if (commandCount == 0) return nullptr;

	std::vector<D2D1_SVG_PATH_COMMAND> commands(commandCount);
	std::vector<float> segments(segmentCount);

	
	hr = pathData->GetCommands(commands.data(), commandCount, 0);
	if (FAILED(hr)) return nullptr;

	hr = pathData->GetSegmentData(segments.data(), segmentCount, 0);
	if (FAILED(hr)) return nullptr;
	
	Microsoft::WRL::ComPtr<ID2D1PathGeometry> pathGeometry;
	hr = factory_->CreatePathGeometry(pathGeometry.GetAddressOf());
	if (FAILED(hr) || !pathGeometry) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
	hr =pathGeometry->Open(sink.GetAddressOf());
	if (FAILED(hr) || !sink) return nullptr;

	MapSvgCommandsToSink(commands, segments, sink.Get());

	hr = sink->Close();
	if (FAILED(hr)) return nullptr;
	
	Microsoft::WRL::ComPtr<ID2D1Geometry> base;
	pathGeometry.As(&base);
	return base;
}

Microsoft::WRL::ComPtr<ID2D1Geometry> SvgCache::CreateGeometryFromCircle(ID2D1SvgElement* element)
{
	float cx = 0.0f, cy = 0.0f, r = 0.0f;
	
	(void)element->GetAttributeValue(L"cx", &cx);
	(void)element->GetAttributeValue(L"cy", &cy);
	
	if(FAILED(element->GetAttributeValue(L"r", &r)) || r <= 0.0f)
		return nullptr;

	Microsoft::WRL::ComPtr<ID2D1EllipseGeometry> ellipse;
	HRESULT hr = factory_->CreateEllipseGeometry(
		D2D1::Ellipse(D2D1::Point2F(cx, cy), r, r),
		ellipse.GetAddressOf()
	);
	if (FAILED(hr)) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1Geometry> geometry;
	ellipse.As(&geometry);

	return geometry;
}

Microsoft::WRL::ComPtr<ID2D1Geometry> SvgCache::CreateGeometryFromRect(ID2D1SvgElement* element)
{
	float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f, rx = 0.0f, ry = 0.0f;

	(void)element->GetAttributeValue(L"x", &x);
	(void)element->GetAttributeValue(L"y", &y);

	if (FAILED(element->GetAttributeValue(L"width", &w)) || 
		FAILED(element->GetAttributeValue(L"height", &h)) || 
		w <= 0.0f || h <= 0.0f) return nullptr;
	const bool hasRx = element->IsAttributeSpecified(L"rx", nullptr);
	const bool hasRy = element->IsAttributeSpecified(L"ry", nullptr);
	
	if (hasRx && FAILED(element->GetAttributeValue(L"rx", &rx))) return nullptr;
	if (hasRy && FAILED(element->GetAttributeValue(L"ry", &ry))) return nullptr;
	
	if (hasRx && !hasRy) ry = rx;
	if (hasRy && !hasRx) rx = ry;

	if (rx < 0.0f) rx = 0.0f;
	if (ry < 0.0f) ry = 0.0f;

	rx = (std::min)(rx, w * 0.5f);
	ry = (std::min)(ry, w * 0.5f);

	const auto rect = D2D1::RectF(x, y, x + w, y + h);

	if (rx > 0.0f || ry > 0.0f)
	{
		Microsoft::WRL::ComPtr<ID2D1RoundedRectangleGeometry> rounded;
		HRESULT hr = factory_->CreateRoundedRectangleGeometry(
			D2D1::RoundedRect(rect, rx, ry),
			rounded.GetAddressOf()
		);
		if (FAILED(hr)) return nullptr;

		Microsoft::WRL::ComPtr<ID2D1Geometry> base;
		rounded.As(&base);
		return base;
	}
	else
	{
		Microsoft::WRL::ComPtr<ID2D1RectangleGeometry> geometry;
		HRESULT hr = factory_->CreateRectangleGeometry(rect, geometry.GetAddressOf());
		if (FAILED(hr))	return nullptr;

		Microsoft::WRL::ComPtr<ID2D1Geometry> base;
		geometry.As(&base);
		return base;
	}
}

Microsoft::WRL::ComPtr<ID2D1Geometry> SvgLib::Parser::SvgCache::CreateGeometryFromEllipse(ID2D1SvgElement* element)
{
	float cx = 0.0f, cy = 0.0f, rx = 0.0f, ry = 0.0f;

	(void)element->GetAttributeValue(L"cx", &cx);
	(void)element->GetAttributeValue(L"cy", &cy);

	if (FAILED(element->GetAttributeValue(L"rx", &rx))) return nullptr;
	if (FAILED(element->GetAttributeValue(L"ry", &ry))) return nullptr;
	if (rx <= 0 || ry <= 0) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1EllipseGeometry> ellipse;
	HRESULT hr = factory_->CreateEllipseGeometry(
		D2D1::Ellipse(D2D1::Point2F(cx, cy), rx, ry),
		ellipse.GetAddressOf()
	);
	if (FAILED(hr)) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1Geometry> base;
	ellipse.As(&base);
	return base;
}

Microsoft::WRL::ComPtr<ID2D1Geometry> SvgLib::Parser::SvgCache::CreateGeometryFromLine(ID2D1SvgElement* element)
{
	HRESULT hr = S_OK;
	float x1 = 0, y1 = 0, x2 = 0, y2 = 0;

	if (FAILED(element->GetAttributeValue(L"x1", &x1))) return nullptr;
	if (FAILED(element->GetAttributeValue(L"y1", &y1))) return nullptr;
	if (FAILED(element->GetAttributeValue(L"x2", &x2))) return nullptr;
	if (FAILED(element->GetAttributeValue(L"y2", &y2))) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1PathGeometry> path;
	hr = factory_->CreatePathGeometry(path.GetAddressOf());
	if (FAILED(hr)) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
	hr = path->Open(sink.GetAddressOf());
	if (FAILED(hr))	return nullptr;

	sink->BeginFigure(D2D1::Point2F(x1, y1), D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddLine(D2D1::Point2F(x2, y2));
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	hr = sink->Close();
	if (FAILED(hr)) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1Geometry> base;
	path.As(&base);
	return base;
}

Microsoft::WRL::ComPtr<ID2D1Geometry> SvgLib::Parser::SvgCache::CreateGeometryFromPolyline(ID2D1SvgElement* element)
{
	HRESULT hr = S_OK;
	
	Microsoft::WRL::ComPtr<ID2D1SvgPointCollection> points;
	if (FAILED(element->GetAttributeValue(L"points", points.GetAddressOf()))) 
		return nullptr;

	const UINT32 count = points->GetPointsCount();
	if (count < 2) return nullptr;
	
	std::vector<D2D1_POINT_2F> bufferPoints(count);
	hr = points->GetPoints(bufferPoints.data(), count);
	if (FAILED(hr)) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1PathGeometry> path;
	hr = factory_->CreatePathGeometry(path.GetAddressOf());
	if (FAILED(hr)) return nullptr;

	Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
	hr = path->Open(sink.GetAddressOf());
	if (FAILED(hr)) return nullptr;


	sink->BeginFigure(bufferPoints[0], D2D1_FIGURE_BEGIN_HOLLOW);

	for (UINT32 i = 1; i < count; ++i)
	{
		sink->AddLine(bufferPoints[i]);
	}

	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	hr = sink->Close();
	if (FAILED(hr))
		return nullptr;

	return path;
}

Microsoft::WRL::ComPtr<ID2D1Geometry> SvgCache::CreateGeometryFromPolygon(ID2D1SvgElement* element)
{
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<ID2D1SvgPointCollection> points;
	if (FAILED(element->GetAttributeValue(L"points", points.GetAddressOf())))
		return nullptr;

	const UINT32 count = points->GetPointsCount();
	if (count < 2)
		return nullptr;

	std::vector<D2D1_POINT_2F> bufferPoints(count);
	hr = points->GetPoints(bufferPoints.data(), count);
	if (FAILED(hr))
		return nullptr;

	Microsoft::WRL::ComPtr<ID2D1PathGeometry> path;
	hr = factory_->CreatePathGeometry(path.GetAddressOf());
	if (FAILED(hr))
		return nullptr;

	Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
	hr = path->Open(sink.GetAddressOf());
	if (FAILED(hr))
		return nullptr;


	sink->BeginFigure(bufferPoints[0], D2D1_FIGURE_BEGIN_FILLED);

	for (UINT32 i = 1; i < count; ++i)
	{
		sink->AddLine(bufferPoints[i]);
	}

	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	hr = sink->Close();
	if (FAILED(hr))
		return nullptr;

	return path;
}

void SvgCache::ReadSvgElement(ID2D1SvgElement* element, SvgElementInfo& elemInfo)
{
	if (!element) return;

	HRESULT hr = S_OK;
	
	// id
	if (element->IsAttributeSpecified(L"id", nullptr))
	{
		std::wstring id = {};
		UINT32 lenChars = 0;
		HRESULT hrLen = element->GetAttributeValueLength(
			L"id",
			D2D1_SVG_ATTRIBUTE_STRING_TYPE_ID,
			&lenChars
		);
		if (SUCCEEDED(hrLen) && lenChars > 0)
		{
			std::wstring buf;
			buf.resize(lenChars + 1);

			HRESULT hrGet = element->GetAttributeValue(
				L"id",
				D2D1_SVG_ATTRIBUTE_STRING_TYPE_ID,
				&buf[0],
				lenChars + 1
			);
			if (SUCCEEDED(hrGet))
			{
				buf[lenChars] = L'\0';
				id = buf.c_str();
			}
			elemInfo.id = id;
		}
	}

	// width
	if (element->IsAttributeSpecified(L"width", nullptr))
	{
		float w = {};
		hr = element->GetAttributeValue(L"width", &w);
		if (SUCCEEDED(hr))
			elemInfo.width = w;
	}

	// height
	if (element->IsAttributeSpecified(L"height", nullptr))
	{
		float h = {};
		hr = element->GetAttributeValue(L"height", &h);
		if (SUCCEEDED(hr))
			elemInfo.height = h;
	}

	// style
	if (element->IsAttributeSpecified(L"style", nullptr))
	{
		// TODO: style attribute 
		//std::wstring style;
		//hr = element->GetAttributeValue(L"style", &style);
		OutputDebugString(L"display");
	}

	// aspectRatio
	if (element->IsAttributeSpecified(L"preserveAspectRatio"))
	{
		D2D1_SVG_PRESERVE_ASPECT_RATIO par = {};
		hr = element->GetAttributeValue(L"preserveAspectRatio", &par);
		if (SUCCEEDED(hr))
			elemInfo.preserveAspectRatio = par;
	}
	else
	{
		elemInfo.preserveAspectRatio.align = D2D1_SVG_ASPECT_ALIGN_X_MID_Y_MID;
		elemInfo.preserveAspectRatio.meetOrSlice = D2D1_SVG_ASPECT_SCALING_MEET;
	}
	
}
// TODO: delete me? ReadSvgElement() macht das gleiche
void SvgCache::ReadImageElement(ID2D1SvgElement* element)
{
	float width = {};
	float height = {};
	D2D1_SVG_PRESERVE_ASPECT_RATIO par = {};
	std::wstring id = {};
	
	// width
	{
		D2D1_SVG_LENGTH length = {};
		HRESULT hr = element->GetAttributeValue(L"width", &length);
		if (SUCCEEDED(hr))
		{
			width = length.value;
		}
	}
	// height
	{
		D2D1_SVG_LENGTH length = {};
		HRESULT hr = element->GetAttributeValue(L"height", &length);
		if (SUCCEEDED(hr))
		{
			height = length.value;
		}
	}
	// aspectRatio
	{
		HRESULT hr = element->GetAttributeValue(L"preserveAspectRatio", &par);
		if (FAILED(hr))
		{
			return;
		}
	}
	// id
	{
		UINT32 lenChars = 0;
		HRESULT hrLen = element->GetAttributeValueLength(
			L"id",
			D2D1_SVG_ATTRIBUTE_STRING_TYPE_ID,
			&lenChars
		);
		if (SUCCEEDED(hrLen) && lenChars > 0)
		{
			std::wstring buf;
			buf.resize(lenChars + 1);

			HRESULT hrGet = element->GetAttributeValue(
				L"id",
				D2D1_SVG_ATTRIBUTE_STRING_TYPE_ID,
				&buf[0],
				lenChars + 1
			);
			if (SUCCEEDED(hrGet))
			{
				buf[lenChars] = L'\0';
				id = buf.c_str();
			}
		}
	}
	image_.width = width;
	image_.height = height;
	image_.preserveAspectRatio = par;
	image_.id = id;
}

void SvgCache::ReadGroupElement(ID2D1SvgElement* element, const RenderState& parent)
{
	RenderState childState = parent;

	// Group transform
	D2D1_MATRIX_3X2_F groupTransform = ReadTransformOrIdentity(element);
	childState.transform = parent.transform * groupTransform;
}

D2D1_MATRIX_3X2_F SvgLib::Parser::SvgCache::ReadTransformOrIdentity(ID2D1SvgElement* element)
{
	D2D1_MATRIX_3X2_F m = D2D1::Matrix3x2F::Identity();
	if (element->IsAttributeSpecified(L"transform", nullptr))
	{
		if (FAILED(element->GetAttributeValue(L"transform", &m)))
			m = D2D1::Matrix3x2F::Identity();
	}
	return m;
}

void SvgCache::MapSvgCommandsToSink(
	const std::vector<D2D1_SVG_PATH_COMMAND>& cmds, 
	const std::vector<float>& data, 
	ID2D1GeometrySink* sink)
{
	if (!sink) return;

	size_t di = 0;
	D2D1_POINT_2F cur = {};
	D2D1_POINT_2F start = {};

	D2D1_POINT_2F lastControlPoint = {};
	bool lastWasQuadratic = false;
	bool lastWasCubic = false;

	for (size_t i = 0; i < cmds.size(); ++i)
	{
		auto cmd = cmds[i];

		switch (cmd)
		{
			case D2D1_SVG_PATH_COMMAND_CLOSE_PATH: // Z,z: closepath
			{
				sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				cur = start;

				lastWasCubic = false;
				lastWasQuadratic = false;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_MOVE_ABSOLUTE: // M: moveto
			case D2D1_SVG_PATH_COMMAND_MOVE_RELATIVE: // m: moveto
			{
				//sink->EndFigure(D2D1_FIGURE_END_OPEN);
				
				const bool relative = 
					(cmd == D2D1_SVG_PATH_COMMAND_MOVE_RELATIVE);

				float x = data[di++];
				float y = data[di++];
				cur = relative ? D2D1_POINT_2F{ cur.x + x, cur.y + y }
					: D2D1_POINT_2F{ x, y };
				
				start = cur;
				sink->BeginFigure(cur, D2D1_FIGURE_BEGIN_FILLED);

				lastWasCubic = false;
				lastWasQuadratic = false;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_LINE_ABSOLUTE: // L: lineto
			case D2D1_SVG_PATH_COMMAND_LINE_RELATIVE: // l: lineto
			{
				const bool relative =
					(cmd == D2D1_SVG_PATH_COMMAND_LINE_RELATIVE);

				float x = data[di++];
				float y = data[di++];
				D2D1_POINT_2F p = relative ? D2D1_POINT_2F{ cur.x + x, cur.y + y }
					: D2D1_POINT_2F{ x, y };
				sink->AddLine(p);
				cur = p;

				lastWasCubic = false;
				lastWasQuadratic = false;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_CUBIC_ABSOLUTE: // C: curveto
			case D2D1_SVG_PATH_COMMAND_CUBIC_RELATIVE: // c: curveto
			{
				const bool relative =
					(cmd == D2D1_SVG_PATH_COMMAND_CUBIC_RELATIVE);

				float x1 = data[di++], y1 = data[di++];
				float x2 = data[di++], y2 = data[di++];
				float x3 = data[di++], y3 = data[di++];

				D2D1_POINT_2F p1 = relative ? D2D1_POINT_2F{cur.x + x1, cur.y + y1} : D2D1_POINT_2F{ x1, y1 };
				D2D1_POINT_2F p2 = relative ? D2D1_POINT_2F{cur.x + x2, cur.y + y2} : D2D1_POINT_2F{ x2, y2 };
				D2D1_POINT_2F p3 = relative ? D2D1_POINT_2F{cur.x + x3, cur.y + y3} : D2D1_POINT_2F{ x3, y3 };

				sink->AddBezier(D2D1::BezierSegment(p1, p2, p3));
				cur = p3;

				lastWasCubic = true;
				lastWasQuadratic = false;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_QUADRADIC_ABSOLUTE: // Q: quadratic Bézier curveto
			case D2D1_SVG_PATH_COMMAND_QUADRADIC_RELATIVE: // q: quadratic Bézier curveto
			{
				const bool relative =
					(cmd == D2D1_SVG_PATH_COMMAND_QUADRADIC_RELATIVE);

				float x1 = data[di++], y1 = data[di++];
				float x2 = data[di++], y2 = data[di++];

				D2D1_POINT_2F p1 = relative ? D2D1_POINT_2F{ cur.x + x1, cur.y + y1 } : D2D1_POINT_2F{ x1, y1 };
				D2D1_POINT_2F p2 = relative ? D2D1_POINT_2F{ cur.x + x2, cur.y + y2 } : D2D1_POINT_2F{ x2, y2 };

				D2D1_QUADRATIC_BEZIER_SEGMENT seg = { p1, p2 };

				sink->AddQuadraticBezier(seg);
				cur = p2;

				lastWasCubic = false;
				lastWasQuadratic = true;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_ARC_ABSOLUTE: // A: elliptical arc
			case D2D1_SVG_PATH_COMMAND_ARC_RELATIVE: // a: elliptical arc
			{
				const bool relative =
					(cmd == D2D1_SVG_PATH_COMMAND_ARC_RELATIVE);

				float rx = data[di++], ry = data[di++], xAxisRotation = data[di++];
				float largeArcFlag = data[di++], sweepFlag = data[di++];
				float x = data[di++], y = data[di++];

				D2D1_POINT_2F arcEndPoint = relative ? D2D1_POINT_2F{ cur.x + x, cur.y + y } : D2D1_POINT_2F{ x, y };

				const bool large = (largeArcFlag != 0.0);
				const bool sweep = (sweepFlag != 0.0);

				D2D1_ARC_SEGMENT arcSeg =
				{
					arcEndPoint,
					D2D1::SizeF(rx, ry),
					xAxisRotation,
					sweep ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					large ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL
				};
				sink->AddArc(arcSeg);
				cur = arcEndPoint;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_HORIZONTAL_ABSOLUTE: // H: horizontal lineto
			case D2D1_SVG_PATH_COMMAND_HORIZONTAL_RELATIVE: // h: horizontal lineto
			{
				const bool relative =
					(cmd == D2D1_SVG_PATH_COMMAND_HORIZONTAL_RELATIVE);

				float x = data[di++];
				D2D1_POINT_2F pt = relative ? D2D1_POINT_2F{ cur.x + x, cur.y } : D2D1_POINT_2F{ x, cur.y };
				sink->AddLine(pt);
				cur = pt;

				lastWasCubic = false;
				lastWasQuadratic = false;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_VERTICAL_ABSOLUTE: // V: vertical lineto
			case D2D1_SVG_PATH_COMMAND_VERTICAL_RELATIVE: // v: vertical lineto
			{
				const bool relative =
					(cmd == D2D1_SVG_PATH_COMMAND_VERTICAL_RELATIVE);

				float y = data[di++];
				D2D1_POINT_2F pt = relative ? D2D1_POINT_2F{ cur.x, cur.y + y } : D2D1_POINT_2F{ cur.x, y };
				sink->AddLine(pt);
				cur = pt;

				lastWasCubic = false;
				lastWasQuadratic = false;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_CUBIC_SMOOTH_ABSOLUTE: // S: shorthand/smooth curveto
			case D2D1_SVG_PATH_COMMAND_CUBIC_SMOOTH_RELATIVE: // s: shorthand/smooth curveto
			{
				const bool relative =
					(cmd == D2D1_SVG_PATH_COMMAND_CUBIC_SMOOTH_RELATIVE);

				D2D1_POINT_2F control1 =
					lastWasCubic
					? D2D1_POINT_2F{ 2.0f * cur.x - lastControlPoint.x,
									 2.0f * cur.y - lastControlPoint.y }
					: cur;

				float x2 = data[di++], y2 = data[di++];
				float x = data[di++], y = data[di++];
				
				D2D1_POINT_2F control2 = relative ? D2D1_POINT_2F{ cur.x + x2, cur.y + y2 } : D2D1_POINT_2F{ x2, y2 };
				D2D1_POINT_2F end = relative ? D2D1_POINT_2F{ cur.x + x, cur.y + y } : D2D1_POINT_2F{ x, y };

				sink->AddBezier(D2D1::BezierSegment(
					control1, control2, end
				));

				lastControlPoint = control2;
				lastWasCubic = true;
				lastWasQuadratic = false;

				cur = end;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_QUADRADIC_SMOOTH_ABSOLUTE: // T: shorthand/smooth quadratic Bézier curveto
			case D2D1_SVG_PATH_COMMAND_QUADRADIC_SMOOTH_RELATIVE: // t: shorthand/smooth quadratic Bézier curveto
			{
				const bool relative =
					(cmd == D2D1_SVG_PATH_COMMAND_QUADRADIC_SMOOTH_RELATIVE);

				D2D1_POINT_2F control = 
					lastWasQuadratic 
					? D2D1_POINT_2F{ 2.0f * cur.x - lastControlPoint.x, 
									 2.0f * cur.y - lastControlPoint.y }
					: cur;
				
				float x = data[di++], y = data[di++];

				D2D1_POINT_2F end = relative 
					? D2D1_POINT_2F{ cur.x + x, cur.y + y } 
					: D2D1_POINT_2F{ x, y };

				sink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(
					control, end
				));

				lastControlPoint = control;
				lastWasCubic = false;
				lastWasQuadratic = true;

				cur = end;
				break;
			}

			case D2D1_SVG_PATH_COMMAND_FORCE_DWORD:
				// Error
				break;
			default:
				break;
			} // switch
	} // for
	//sink->EndFigure(D2D1_FIGURE_END_OPEN);
	//sink->Close();
}
