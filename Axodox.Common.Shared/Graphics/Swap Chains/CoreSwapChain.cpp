#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#ifdef WINRT_Windows_UI_Core_H
#include "CoreSwapChain.h"
#include "Threading/UwpThreading.h"

using namespace Axodox::Threading;
using namespace DirectX;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  CoreSwapChain::CoreSwapChain(const GraphicsDevice& device, const CoreWindow& window, SwapChainFlags flags) :
    SwapChain(device, flags),
    _window(window)
  {
    //Create swap chain
    com_ptr<IDXGISwapChain1> swapChain;
    {
      auto swapChainDescription = CreateDescription();

      com_ptr<IDXGIFactory4> dxgiFactory;
      check_hresult(CreateDXGIFactory2(0, guid_of<IDXGIFactory4>(), dxgiFactory.put_void()));

      check_hresult(dxgiFactory->CreateSwapChainForCoreWindow(
        device.get(),
        get_unknown(_window),
        &swapChainDescription,
        nullptr,
        swapChain.put()
      ));
    }

    //Subscribe to events
    _sizeChangedRevoker = window.SizeChanged(auto_revoke, [this](auto&, auto&) {
      _postPresentActions.invoke_async([this]() { Resize(); });
      });

    //Initialize swap chain
    InitializeSwapChain(swapChain);
  }

  XMUINT2 CoreSwapChain::GetSize() const
  {
    const auto bounds = _window.Bounds();
    return {
      max<uint32_t>(_minBufferSize, uint32_t(bounds.Width)),
      max<uint32_t>(_minBufferSize, uint32_t(bounds.Height))
    };
  }

  DXGI_MATRIX_3X2_F CoreSwapChain::GetTransformation() const
  {
    //Return a zero-initialized matrix so SwapChain::InitializeSwapChain's
    //is_default() check skips SetMatrixTransform. That API is only legal
    //on swap chains created with CreateSwapChainForComposition; calling it
    //on a CoreWindow swap chain raises DXGI error #225.
    return {};
  }
}
#endif
#endif