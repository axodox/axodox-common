#pragma once
#include "../includes.h"

#include "Graphics/Math/Point.h"
#include "Graphics/Math/Rect.h"
#include "Graphics/Math/Size.h"

#if defined(USE_DIRECTX) && defined(PLATFORM_WINDOWS)
#include "Graphics/Buffers/ConstantBuffer.h"
#include "Graphics/Buffers/GraphicsBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/StructuredBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"

#include "Graphics/Devices/DrawingController.h"
#include "Graphics/Devices/GraphicsDevice.h"
#include "Graphics/Devices/GraphicsDeviceContext.h"
#include "Graphics/Devices/GraphicsResource.h"
#include "Graphics/Devices/GraphicsTypes.h"

#include "Graphics/Meshes/IndexedMesh.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/Primitives.h"
#include "Graphics/Meshes/SimpleMesh.h"
#include "Graphics/Meshes/VertexDefinitions.h"

#include "Graphics/Shaders/ComputeShader.h"
#include "Graphics/Shaders/DomainShader.h"
#include "Graphics/Shaders/GeometryShader.h"
#include "Graphics/Shaders/HullShader.h"
#include "Graphics/Shaders/PixelShader.h"
#include "Graphics/Shaders/VertexShader.h"

#include "Graphics/States/BlendState.h"
#include "Graphics/States/DepthStencilState.h"
#include "Graphics/States/RasterizerState.h"
#include "Graphics/States/SamplerState.h"

#include "Graphics/Swap Chains/CoreSwapChain.h"
#include "Graphics/Swap Chains/HwndSwapChain.h"
#include "Graphics/Swap Chains/SwapChain.h"
#include "Graphics/Swap Chains/XamlSwapChain.h"

#include "Graphics/Textures/DepthStencil2D.h"
#include "Graphics/Textures/DrawingTarget2D.h"
#include "Graphics/Textures/RenderTarget2D.h"
#include "Graphics/Textures/RenderTargetWithDepthStencil2D.h"
#include "Graphics/Textures/StagingTexture2D.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/Texture2DDefinition.h"
#include "Graphics/Textures/TextureData.h"

#include "Graphics/Helpers.h"
#endif