#pragma once
#ifdef PLATFORM_WINDOWS
#include "pch.h"

namespace Axodox::Graphics
{
  AXODOX_COMMON_API size_t BitsPerPixel(DXGI_FORMAT format);
  AXODOX_COMMON_API bool HasAlpha(DXGI_FORMAT format);

  AXODOX_COMMON_API bool IsUByteN1Compatible(DXGI_FORMAT format);
  AXODOX_COMMON_API bool IsUByteN4Compatible(DXGI_FORMAT format);

  AXODOX_COMMON_API IWICImagingFactory* WicFactory();

  AXODOX_COMMON_API winrt::Windows::Graphics::Imaging::BitmapPixelFormat ToBitmapPixelFormat(DXGI_FORMAT format);
  AXODOX_COMMON_API DXGI_FORMAT ToDxgiFormat(winrt::Windows::Graphics::Imaging::BitmapPixelFormat format);

  AXODOX_COMMON_API WICPixelFormatGUID ToWicPixelFormat(DXGI_FORMAT format);
  AXODOX_COMMON_API DXGI_FORMAT ToDxgiFormat(WICPixelFormatGUID format);
}
#endif