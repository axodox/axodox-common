#pragma once
#ifdef PLATFORM_WINDOWS
#include "GraphicsDeviceContext.h"
#include "Infrastructure/LifetimeToken.h"

namespace Axodox::Graphics
{
  enum class GraphicsDeviceFlags : uint32_t
  {
    None = 0,
    UseDebugSdkLayers = 1,
    UseDebuggableDevice = 2,
    UseWarpDevice = 4,
    UseDirect3D11On12Device = 8
  };

  enum class GraphicsCapabilities
  {
    None = 0,
    SupportsRenderTargetArrayIndexFromVertexShader = 1,
    SupportsMinMaxFiltering = 2,
    SupportsAsyncCreate = 4,
    SupportsHalfPrecision = 8,
    SupportsAdvancedTypedUAVs = 16
  };

  struct AXODOX_COMMON_API AdapterInfo
  {
    std::wstring Name;
    LUID Id;
    uint64_t VideoMemory;
    uint32_t Index;
  };

  class AXODOX_COMMON_API GraphicsDevice
  {
  public:
    explicit GraphicsDevice(GraphicsDeviceFlags flags = GraphicsDeviceFlags::None, LUID adapterId = {});
    explicit GraphicsDevice(const winrt::com_ptr<ID3D11Device>& device);

    const winrt::com_ptr<ID3D11DeviceT>& operator*() const;
    ID3D11DeviceT* operator->() const;
    ID3D11DeviceT* get() const;

    GraphicsDeviceContext* ImmediateContext();

    static std::vector<AdapterInfo> Adapters();

    Infrastructure::lifetime_token SuppressWarnings(std::vector<D3D11_MESSAGE_ID>&& messages);

  private:
    static const D3D_FEATURE_LEVEL _featureLevels[];
    winrt::com_ptr<ID3D11DeviceT> _device;
    std::shared_ptr<GraphicsDeviceContext> _context;
    GraphicsCapabilities _capabilities;
    
    static winrt::com_ptr<IDXGIFactoryT> InitializeFactory();
    static winrt::com_ptr<IDXGIAdapterT> FindAdapter(const winrt::com_ptr<IDXGIFactoryT>& factory, LUID adapterId);
    void InitializeDevice(const winrt::com_ptr<IDXGIFactoryT>& factory, GraphicsDeviceFlags flags, winrt::com_ptr<IDXGIAdapterT> adapter);
    GraphicsCapabilities CheckCapabilties() const;
    void SetupDebugLayer();
  };
}
#endif