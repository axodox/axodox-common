#pragma once
#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "Collections/AlignedAllocator.h"
#include "Graphics/Math/Rect.h"

namespace Axodox::Graphics
{
  enum class TextureImageFormat
  {
    Unknown,
    Rgba8,
    Gray8
  };

  struct AXODOX_COMMON_API TextureData
  {
    uint32_t Width, Height, Stride;
    DXGI_FORMAT Format;
    Collections::aligned_vector<uint8_t> Buffer;

    TextureData();
    TextureData(uint32_t width, uint32_t height, uint32_t stride, DXGI_FORMAT format, Collections::aligned_vector<uint8_t>&& buffer);
    TextureData(uint32_t width, uint32_t height, DXGI_FORMAT format);

    explicit TextureData(const TextureData&) = default;

    TextureData(TextureData&&) noexcept;
    TextureData& operator=(TextureData&&) noexcept;

    uint32_t MainDimension() const;
    uint32_t PixelCount() const;
    uint32_t ByteCount() const;
    bool IsValid() const;

    explicit operator bool() const;
    bool operator==(const TextureData&) const = default;
    bool operator!=(const TextureData&) const = default;

    static TextureData FromBuffer(std::span<const uint8_t> buffer, TextureImageFormat format = TextureImageFormat::Rgba8, std::string* metadata = nullptr);
    std::vector<uint8_t> ToBuffer(std::string_view metadata = "") const;

    static TextureData FromRawBuffer(std::span<const uint8_t> buffer);
    std::vector<uint8_t> ToRawBuffer() const;
    
#ifdef __wincodec_h__
    static TextureData FromWicBitmap(const winrt::com_ptr<IWICBitmapSource>& wicBitmap);
    winrt::com_ptr<IWICBitmap> ToWicBitmap() const;
#endif    

#ifdef WINRT_Windows_Graphics_Imaging_H
    static TextureData FromSoftwareBitmap(const winrt::Windows::Graphics::Imaging::SoftwareBitmap& softwareBitmap);
    winrt::Windows::Graphics::Imaging::SoftwareBitmap ToSoftwareBitmap() const;
#endif

    TextureData Resize(uint32_t width, uint32_t height) const;
    TextureData UniformResize(uint32_t width, uint32_t height, Rect* sourceRect = nullptr) const;
    TextureData ToFormat(DXGI_FORMAT format) const;

    template<typename T>
    T* Row(uint32_t row)
    {
      return reinterpret_cast<T*>(Buffer.data() + row * Stride);
    }

    template<typename T>
    const T* Row(uint32_t row) const
    {
      return reinterpret_cast<const T*>(Buffer.data() + row * Stride);
    }

    template<typename T>
    T* Pixel(uint32_t column, uint32_t row)
    {
      return reinterpret_cast<T*>(Buffer.data() + row * Stride + column * sizeof(T));
    }

    template<typename T>
    const T* Pixel(uint32_t column, uint32_t row) const
    {
      return reinterpret_cast<const T*>(Buffer.data() + row * Stride + column * sizeof(T));
    }

    template<typename T>
    std::span<T> Cast()
    {
      if (Stride != Width * sizeof(T)) throw std::bad_cast();
      return std::span{ reinterpret_cast<T*>(Buffer.data()), Buffer.size() / sizeof(T) };
    }

    TextureData ExtendHorizontally(uint32_t width) const;
    TextureData ExtendVertically(uint32_t height) const;

    TextureData TruncateHorizontally(uint32_t width) const;
    TextureData TruncateVertically(uint32_t height) const;

    Size Size() const;
    Rect FindNonZeroRect() const;
    TextureData GetTexture(Rect rect) const;
    TextureData MergeTexture(const TextureData& texture, Point position);

    static TextureData AlphaBlend(const TextureData& target, const TextureData& source, const TextureData& alpha);
  };
}
#endif