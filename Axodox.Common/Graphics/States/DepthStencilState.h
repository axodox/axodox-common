#pragma once
#ifdef PLATFORM_WINDOWS
#include "Graphics/Devices/GraphicsResource.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API DepthStencilState : public GraphicsResource
  {
  public:
    DepthStencilState(const GraphicsDevice& device, bool writeDepth, D3D11_COMPARISON_FUNC comparison = D3D11_COMPARISON_LESS);

    void Bind(GraphicsDeviceContext* context = nullptr);

  private:
    winrt::com_ptr<ID3D11DepthStencilState> _state;
  };
}
#endif