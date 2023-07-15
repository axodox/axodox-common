#pragma once
#ifdef PLATFORM_WINDOWS
#include "GraphicsBuffer.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API ConstantBuffer : public GraphicsBuffer
  {
  public:
    template<typename T>
    explicit ConstantBuffer(const GraphicsDevice& device) :
      ConstantBuffer(device, sizeof(T))
    { }

    explicit ConstantBuffer(const GraphicsDevice& device, size_t size);

    void Bind(ShaderStage stage, uint32_t slot = 0u, GraphicsDeviceContext* context = nullptr);

    template<typename T>
    void Upload(const T& value, GraphicsDeviceContext* context = nullptr)
    {
      static_assert(std::is_trivially_copyable_v<T>);
      GraphicsBuffer::Upload(std::span<const uint8_t>{ reinterpret_cast<const uint8_t*>(&value), sizeof(value) }, context);
    }
  };
}
#endif