#pragma once
#ifdef PLATFORM_WINDOWS
#include "SwapChain.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API HwndSwapChain : public SwapChain
  {
  public:
    HwndSwapChain(const GraphicsDevice& device, HWND window, SwapChainFlags flags = SwapChainFlags::Default);

  protected:
    virtual DirectX::XMUINT2 GetSize() const override;
    virtual DXGI_MATRIX_3X2_F GetTransformation() const override;
  };
}
#endif