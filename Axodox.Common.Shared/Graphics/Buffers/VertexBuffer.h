#pragma once
#ifdef PLATFORM_WINDOWS
#include "GraphicsBuffer.h"
#include "Graphics/Meshes/VertexDefinitions.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API VertexBuffer : public GraphicsBuffer
  {
    friend class SimpleMesh;

  public:
    template<typename T>
    VertexBuffer(const GraphicsDevice& device, TypedCapacityOrImmutableData<T> source) :
      VertexBuffer(device, source, VertexDefinition(T::Definition), sizeof(T))
    { }
    
    VertexDefinition Definition() const;

    uint32_t ItemSize() const;
    uint32_t Capacity() const;
    uint32_t Size() const;

    void Bind(uint32_t slot = 0, GraphicsDeviceContext* context = nullptr);

  private:
    uint32_t _itemSize;
    VertexDefinition _vertexDefinition;

    VertexBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, VertexDefinition vertexDefinition, uint32_t itemSize);
  };
}
#endif