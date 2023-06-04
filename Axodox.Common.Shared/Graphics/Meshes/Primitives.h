#pragma once
#ifdef PLATFORM_WINDOWS
#include "IndexedMesh.h"

namespace Axodox::Graphics
{
  AXODOX_COMMON_API IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreateQuad(float size = 1.f);
  AXODOX_COMMON_API IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreateCube(float size = 1.f, bool useSameTextureOnAllFaces = true);
  AXODOX_COMMON_API IndexedMeshDescription<VertexPositionNormalTexture, uint32_t> CreatePlane(float size, DirectX::XMUINT2 subdivisions);
}
#endif