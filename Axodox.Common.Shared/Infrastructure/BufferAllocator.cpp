#include "common_includes.h"
#include "BufferAllocator.h"

using namespace std;

namespace Axodox::Infrastructure
{
  buffer_allocator::buffer_allocator(uint64_t size) :
    _size(size)
  {
    _freeSpace.push_back({ 0, _size });
  }

  uint64_t buffer_allocator::size() const
  {
    return _size;
  }

  buffer_segment buffer_allocator::try_allocate(uint64_t size, uint64_t alignment)
  {
    //If we do not have empty space return
    if (_freeSpace.empty()) return {};

    //We cannot allocate more space than the total buffer size
    if (size > _size) throw logic_error("Cannot allocate more space than the entire length of the buffer.");

    //Otherwise find first empty slot
    buffer_segment* selectedSlot = nullptr;
    uint64_t alignedStart = 0;
    for (auto& slot : _freeSpace)
    {
      //If slot is smaller than the alignment ignore it
      if (slot.Size < alignment) continue;

      //Calculate space after alignment
      alignedStart = align_memory_offset(slot.Start, alignment);
      auto remainingSpace = slot.Start + slot.Size - alignedStart;

      //If we have space then select the slot
      if (remainingSpace >= size)
      {
        selectedSlot = &slot;
        break;
      }
    }

    //If we found no slot return
    if (!selectedSlot) return {};

    //Decrease free space
    auto alignmentGap = alignedStart - selectedSlot->Start;
    selectedSlot->Start = alignedStart + size;
    selectedSlot->Size -= size + alignmentGap;

    //Remove empty slot if needed
    if (selectedSlot->Size == 0)
    {
      swap(_freeSpace.back(), *selectedSlot);
      _freeSpace.pop_back();
    }

    //Keep alignment gap as free space
    if (alignmentGap > 0u)
    {
      _freeSpace.push_back(buffer_segment(alignedStart - alignmentGap, alignmentGap));
    }

    //Return allocated segment
    return { alignedStart, size };
  }

  void buffer_allocator::deallocate(buffer_segment segment)
  {
    //Find neighboring slots
    buffer_segment* previousSlot = nullptr;
    buffer_segment* nextSlot = nullptr;

    auto segmentEnd = segment.Start + segment.Size;
    for (auto& slot : _freeSpace)
    {
      if (segmentEnd == slot.Start) nextSlot = &slot;

      auto currentEnd = slot.Start + slot.Size;
      if (currentEnd == segment.Start) previousSlot = &slot;
    }

    //Add neighboring segments to current one
    if (previousSlot)
    {
      segment.Start = previousSlot->Start;
      segment.Size += previousSlot->Size;
      swap(_freeSpace.back(), *previousSlot);
      _freeSpace.pop_back();
    }

    if (nextSlot)
    {
      segment.Size += nextSlot->Size;
      swap(_freeSpace.back(), *nextSlot);
      _freeSpace.pop_back();
    }

    //Add reclaimed segment
    _freeSpace.push_back(segment);
  }

  buffer_segment::operator bool() const noexcept
  {
    return Size > 0u;
  }

  uint64_t align_memory_offset(uint64_t offset, uint64_t alignment)
  {
    auto alignmentRemainder = offset % alignment;
    if (alignmentRemainder != 0) offset += alignment - alignmentRemainder;
    return offset;
  }
}