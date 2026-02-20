#include "pch.h"
#include <string>

class IEventListener
{
public:
	virtual void OnSvgHit(const std::wstring& id) = 0;
	virtual void OnSvgHitRightMouseButton(const std::wstring& id, CPoint point) = 0;
	virtual ~IEventListener() = default;
};