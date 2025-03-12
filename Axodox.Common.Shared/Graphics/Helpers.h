#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"

namespace Axodox::Graphics
{
  AXODOX_COMMON_API size_t BitsPerPixel(DXGI_FORMAT format);
  AXODOX_COMMON_API bool HasAlpha(DXGI_FORMAT format);

  AXODOX_COMMON_API bool IsUByteN1Compatible(DXGI_FORMAT format);
  AXODOX_COMMON_API bool IsUByteN4Compatible(DXGI_FORMAT format);

  AXODOX_COMMON_API IWICImagingFactory* WicFactory();

#ifdef WINRT_Windows_Graphics_Imaging_H
  AXODOX_COMMON_API winrt::Windows::Graphics::Imaging::BitmapPixelFormat ToBitmapPixelFormat(DXGI_FORMAT format);
  AXODOX_COMMON_API DXGI_FORMAT ToDxgiFormat(winrt::Windows::Graphics::Imaging::BitmapPixelFormat format);
#endif

  AXODOX_COMMON_API WICPixelFormatGUID ToWicPixelFormat(DXGI_FORMAT format);
  AXODOX_COMMON_API DXGI_FORMAT ToDxgiFormat(WICPixelFormatGUID format);
}
#endif