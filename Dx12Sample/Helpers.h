#pragma once
#include "stdafx.h"

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		char s_str[64] = {};
		sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
		throw std::runtime_error(std::string(s_str));
	}
}