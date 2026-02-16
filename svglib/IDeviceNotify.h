#pragma once

namespace SvgLib::Graphics
{
	class IDeviceNotify
	{
	public:
		virtual ~IDeviceNotify() = default;
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
	};
}