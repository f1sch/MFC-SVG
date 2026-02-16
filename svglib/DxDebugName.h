#pragma once

#include <d3d11.h>
#include <dxgidebug.h>
#include <string>
#include <wrl.h>

#ifdef _DEBUG
// D3D11
inline void SetDebugName(ID3D11DeviceChild* obj, const char* name)
{
	if (obj && name)
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
}

// D2D1
template<typename T>
inline void SetDebugNameD2D(T* obj, const wchar_t* name)
{
	if (obj && name)
		obj->SetDebugName(name);
}

#define NAME_D3D11(obj) SetDebugName(obj.Get(), #obj)
#define NAME_D3D11_AS(obj, n) SetDebugName(obj.Get(), n)
#define NAME_D3D11_CLASS(obj) SetDebugName(obj.Get(), (std::string(typeid(*this).name()) + "::" + #obj).c_str())
#define NAME_D2D(obj) SetDebugNameD2D(obj.Get(), L#obj)
#define NAME_D2D_AS(obj, n) SetDebugNameD2D(obj.Get(), n)

#else
#define NAME_D3D11(obj)
#define NAME_D3D11_AS(obj, n)
#define NAME_D2D(obj)
#define NAME_D2_AS(obj, n)
#endif