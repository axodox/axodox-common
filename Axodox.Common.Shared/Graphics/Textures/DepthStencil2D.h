#pragma once
#ifdef PLATFORM_WINDOWS
#include "Texture2D.h"

namespace Axodox::Graphics
{
  class AXODOX_COMMON_API DepthStencil2D : public Texture2D
  {
  public:
    DepthStencil2D(const GraphicsDevice& device, const Texture2DDefinition& definition);
    DepthStencil2D(const GraphicsDevice& device, const winrt::com_ptr<ID3D11Texture2D>& texture, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);

    void BindDepthStencilView(GraphicsDeviceContext* context = nullptr);
    void UnbindDepthStencilView(GraphicsDeviceContext* context = nullptr);

    virtual void Unbind(GraphicsDeviceContext* context = nullptr) override;

    void Clear(float depth = 1.f, GraphicsDeviceContext* context = nullptr);
    void Discard(GraphicsDeviceContext* context = nullptr);

    const winrt::com_ptr<ID3D11DepthStencilView>& operator*() const;
    ID3D11DepthStencilView* operator->() const;
    ID3D11DepthStencilView* get() const;

  private:
    winrt::com_ptr<ID3D11DepthStencilView> _depthStencilView;
    D3D11_VIEWPORT _defaultViewport;

    static Texture2DDefinition PrepareDefinition(Texture2DDefinition definition);
    winrt::com_ptr<ID3D11DepthStencilView> CreateView(DXGI_FORMAT format) const;
    D3D11_VIEWPORT GetDefaultViewport() const;
  };
}
#endif