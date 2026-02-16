#include "pch.h"
#include <string>

class IEventListener
{
public:
	virtual void OnSvgHit(const std::wstring& id) = 0;
	virtual ~IEventListener() = default;
};