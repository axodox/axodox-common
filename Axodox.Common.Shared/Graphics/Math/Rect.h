#pragma once
#include "Point.h"
#include "Size.h"

namespace Axodox::Graphics
{
  struct AXODOX_COMMON_API Rect
  {
    static const Rect Empty;

    int32_t Left, Top, Right, Bottom;

    static Rect FromSize(Size size);
    static Rect FromCenterSize(Point center, Size size);
    static Rect FromLeftTopSize(Point leftTop, Size size);

    Rect Extend(Point point) const;
    Rect Offset(int32_t value) const;
    Rect Fit(float aspectRatio) const;
    Rect Clamp(Size size) const;
    Rect PushClamp(Size size) const;

    bool Contains(Rect rect) const;

    Point LeftTop() const;
    Point Center() const;
    Size Size() const;

    int32_t Width() const;
    int32_t Height() const;

    explicit operator bool() const;   
    
  };
}