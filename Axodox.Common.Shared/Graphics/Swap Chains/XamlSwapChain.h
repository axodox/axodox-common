#pragma once
#ifdef PLATFORM_WINDOWS
#ifdef WINRT_Windows_UI_Xaml_Controls_H
#include "SwapChain.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API XamlSwapChain : public SwapChain
  {
    typedef winrt::Windows::UI::Xaml::Controls::SwapChainPanel SwapChainPanel;

  public:
    XamlSwapChain(const GraphicsDevice& device, const SwapChainPanel& panel, SwapChainFlags flags = SwapChainFlags::Default);

  protected:
    virtual DirectX::XMUINT2 GetSize() const override;
    virtual DXGI_MATRIX_3X2_F GetTransformation() const override;

  private:
    SwapChainPanel _panel;
    SwapChainPanel::SizeChanged_revoker _sizeChangedRevoker;
    SwapChainPanel::CompositionScaleChanged_revoker _compositionScaleChangedRevoker;
  };
}
#endif
#endif