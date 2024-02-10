#pragma once
#include "pch.h"

namespace Axodox::Infrastructure
{
  struct AXODOX_COMMON_API buffer_segment
  {
    uint64_t Start = 0, Size = 0;

    explicit operator bool() const noexcept;
  };

  AXODOX_COMMON_API uint64_t align_memory_offset(uint64_t offset, uint64_t alignment);

  class AXODOX_COMMON_API buffer_allocator
  {
  public:
    buffer_allocator(uint64_t size);

    uint64_t size() const;

    buffer_segment try_allocate(uint64_t size, uint64_t alignment = 0);
    void deallocate(buffer_segment segment);

  private:
    uint64_t _size;
    std::vector<buffer_segment> _freeSpace;
  };
}