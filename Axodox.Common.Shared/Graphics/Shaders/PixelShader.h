#pragma once
#ifdef PLATFORM_WINDOWS
#include "Graphics/Devices/GraphicsResource.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API PixelShader : public GraphicsResource
  {
  public:
    PixelShader(const GraphicsDevice& device, std::span<const uint8_t> buffer);

    void Bind(GraphicsDeviceContext* context = nullptr);

  private:
    winrt::com_ptr<ID3D11PixelShader> _shader;
  };
}
#endif