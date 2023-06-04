#pragma once
#ifdef PLATFORM_WINDOWS
#include "pch.h"

namespace Axodox::Graphics
{
  typedef std::span<const D3D11_INPUT_ELEMENT_DESC> VertexDefition;

  struct AXODOX_COMMON_API VertexPosition
  {
    DirectX::XMFLOAT3 Position;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct AXODOX_COMMON_API VertexPositionColor
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMUBYTEN4 Color;
    
    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct AXODOX_COMMON_API VertexPositionTexture
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMUSHORTN2 Texture;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct AXODOX_COMMON_API VertexPositionNormal
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMBYTEN4 Normal;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct AXODOX_COMMON_API VertexPositionNormalColor
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMBYTEN4 Normal;
    DirectX::PackedVector::XMUBYTEN4 Color;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct AXODOX_COMMON_API VertexPositionNormalTexture
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMBYTEN4 Normal;
    DirectX::PackedVector::XMUSHORTN2 Texture;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };
}
#endif