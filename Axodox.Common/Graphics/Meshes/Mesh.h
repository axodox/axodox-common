#pragma once
#ifdef PLATFORM_WINDOWS
#include "VertexDefinitions.h"
#include "Graphics/Devices/GraphicsResource.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API Mesh : public GraphicsResource
  {
  public:
    using GraphicsResource::GraphicsResource;
    virtual ~Mesh() = default;

    virtual void Draw(GraphicsDeviceContext* context = nullptr) = 0;
    virtual void DrawInstanced(uint32_t instanceCount, GraphicsDeviceContext* context = nullptr) = 0;

    virtual VertexDefition Defition() const = 0;
  };
}
#endif