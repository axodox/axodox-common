#pragma once
#ifdef PLATFORM_WINDOWS
#include "Texture2D.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API StagingTexture2D : public Texture2D
  {
  public:
    StagingTexture2D(const GraphicsDevice& device, const Texture2DDefinition& definition);

    TextureData Download(uint32_t textureIndex = 0u, GraphicsDeviceContext* context = nullptr);

  private:
    static Texture2DDefinition PrepareDefinition(Texture2DDefinition definition);
  };
}
#endif