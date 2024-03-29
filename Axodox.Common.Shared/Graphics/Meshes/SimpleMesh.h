#pragma once
#ifdef PLATFORM_WINDOWS
#include "Mesh.h"
#include "Graphics/Buffers/VertexBuffer.h"

namespace Axodox::Graphics
{
  template<typename TVertex>
  struct SimpleMeshDescription
  {
    std::vector<TVertex> Vertices;
    D3D11_PRIMITIVE_TOPOLOGY Topology;
  };

  class AXODOX_COMMON_API SimpleMesh : public Mesh
  {
  public:
    template<typename T>
    SimpleMesh(const GraphicsDevice& device, TypedCapacityOrImmutableData<T> source, D3D11_PRIMITIVE_TOPOLOGY topology) :
      Mesh(device),
      _vertexBuffer(device, source),
      _topology(topology)
    { }

    template<typename T>
    SimpleMesh(const GraphicsDevice& device, const SimpleMeshDescription<T>& description) :
      SimpleMesh(device, description.Vertices, description.Topology)
    { }

    virtual void Draw(GraphicsDeviceContext* context = nullptr) override;
    virtual void DrawInstanced(uint32_t instanceCount, GraphicsDeviceContext* context = nullptr) override;

    virtual VertexDefinition Definition() const override;

    uint32_t VertexCapacity() const;
    
    template<typename T>
    void Upload(std::span<const T> items, GraphicsDeviceContext* context = nullptr)
    {
      _vertexBuffer.Upload(items, context);
    }

  protected:
    VertexBuffer _vertexBuffer;
    D3D11_PRIMITIVE_TOPOLOGY _topology;
  };
}
#endif