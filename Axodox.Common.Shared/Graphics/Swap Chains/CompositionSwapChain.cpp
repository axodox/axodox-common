#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#ifdef WINRT_Windows_UI_Composition_H
#include "CompositionSwapChain.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace DirectX;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::UI::Composition;

namespace Axodox::Graphics
{
  CompositionSwapChain::CompositionSwapChain(
    const GraphicsDevice& device,
    const Compositor& compositor,
    uint32_t width,
    uint32_t height,
    SwapChainFlags flags) :
    SwapChain(device, flags),
    _compositor(compositor),
    _width(max(_minBufferSize, width)),
    _height(max(_minBufferSize, height))
  {
    //Create swap chain
    com_ptr<IDXGISwapChain1> swapChain;
    {
      auto swapChainDescription = CreateDescription();

      com_ptr<IDXGIFactory4> dxgiFactory;
      check_hresult(CreateDXGIFactory2(0, guid_of<IDXGIFactory4>(), dxgiFactory.put_void()));

      check_hresult(dxgiFactory->CreateSwapChainForComposition(
        device.get(),
        &swapChainDescription,
        nullptr,
        swapChain.put()
      ));
    }

    //Create visual
    {
      auto interop = _compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>();
      com_ptr<ABI::Windows::UI::Composition::ICompositionSurface> surface;
      check_hresult(interop->CreateCompositionSurfaceForSwapChain(
        swapChain.get(),
        surface.put()
      ));

      auto compositionSurface = surface.as<ICompositionSurface>();
      _brush = _compositor.CreateSurfaceBrush(compositionSurface);
      _brush.Stretch(CompositionStretch::Fill);

      _visual = _compositor.CreateSpriteVisual();
      _visual.Size({ float(_width), float(_height) });
      _visual.Brush(_brush);
    }

    //Initialize swap chain
    InitializeSwapChain(swapChain);
  }

  Visual CompositionSwapChain::Visual() const
  {
    return _visual;
  }

  void CompositionSwapChain::Resize(uint32_t width, uint32_t height)
  {
    _width = max(_minBufferSize, width);
    _height = max(_minBufferSize, height);
    _visual.Size({ float(_width), float(_height) });
    SwapChain::Resize();
  }

  XMUINT2 CompositionSwapChain::GetSize() const
  {
    return { _width, _height };
  }

  DXGI_MATRIX_3X2_F CompositionSwapChain::GetTransformation() const
  {
    return {};
  }
}
#endif
#endif
