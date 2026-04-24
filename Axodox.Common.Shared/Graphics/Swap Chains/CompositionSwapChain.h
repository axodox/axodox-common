#pragma once
#ifdef PLATFORM_WINDOWS
#ifdef WINRT_Windows_UI_Composition_H
#include "SwapChain.h"

namespace Axodox::Graphics
{
  //Swap chain created via CreateSwapChainForComposition and wrapped in a
  //SpriteVisual. The caller owns placement: attach Visual() anywhere in a
  //composition tree. Size is driven explicitly via Resize() — unlike the
  //Core/Xaml variants, there is no panel or window to query, so the host
  //is responsible for propagating size changes.
  class AXODOX_COMMON_API CompositionSwapChain : public SwapChain
  {
  public:
    CompositionSwapChain(
      const GraphicsDevice& device,
      const winrt::Windows::UI::Composition::Compositor& compositor,
      uint32_t width,
      uint32_t height,
      SwapChainFlags flags = SwapChainFlags::Default);

    winrt::Windows::UI::Composition::Visual Visual() const;

    void Resize(uint32_t width, uint32_t height);

  protected:
    virtual DirectX::XMUINT2 GetSize() const override;
    virtual DXGI_MATRIX_3X2_F GetTransformation() const override;

  private:
    winrt::Windows::UI::Composition::Compositor _compositor;
    winrt::Windows::UI::Composition::SpriteVisual _visual{ nullptr };
    winrt::Windows::UI::Composition::CompositionSurfaceBrush _brush{ nullptr };
    uint32_t _width;
    uint32_t _height;
  };
}
#endif
#endif
