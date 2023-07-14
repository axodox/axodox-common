#pragma once
#ifdef PLATFORM_WINDOWS
#ifdef WINRT_Windows_UI_Core_H
#include "SwapChain.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API CoreSwapChain : public SwapChain
  {
    typedef winrt::Windows::UI::Core::CoreWindow CoreWindow;

  public:
    CoreSwapChain(const GraphicsDevice& device, const CoreWindow& window, SwapChainFlags flags = SwapChainFlags::Default);

  private:
    CoreWindow _window;
    CoreWindow::SizeChanged_revoker _sizeChangedRevoker;
  };
}
#endif
#endif