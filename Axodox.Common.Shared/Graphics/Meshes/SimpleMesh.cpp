#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "SimpleMesh.h"

namespace Axodox::Graphics
{
  void SimpleMesh::Draw(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    _vertexBuffer.Bind(0u, context);
    (*context)->IASetPrimitiveTopology(_topology);
    (*context)->Draw(_vertexBuffer.Size(), 0);
  }

  void SimpleMesh::DrawInstanced(uint32_t instanceCount, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    _vertexBuffer.Bind(0u, context);
    (*context)->IASetPrimitiveTopology(_topology);
    (*context)->DrawInstanced(_vertexBuffer.Size(), instanceCount, 0, 0);
  }

  VertexDefinition SimpleMesh::Definition() const
  {
    return _vertexBuffer.Definition();
  }
  
  uint32_t SimpleMesh::VertexCapacity() const
  {
    return _vertexBuffer.Capacity();
  }
}
#endif