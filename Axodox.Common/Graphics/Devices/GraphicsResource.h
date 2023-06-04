#pragma once
#ifdef PLATFORM_WINDOWS
#include "GraphicsDevice.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API GraphicsResource
  {
  public:
    GraphicsResource(const GraphicsDevice& device);
    virtual ~GraphicsResource() = default;

    GraphicsDevice* Owner();
    const GraphicsDevice* Owner() const;

  protected:
    GraphicsDevice _device;
  };
}
#endif