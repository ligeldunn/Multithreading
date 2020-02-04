/* 
 * File:   BitMapAllocator.cpp
 * 
 * Created on August 10, 2019, 9:33 PM
 */

#include "BitMapAllocator.h"

#include <cstring>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>

using mem::Addr;
using mem::kPageSize;

BitMapAllocator::BitMapAllocator(mem::MMU &memory_) 
: memory(memory_)
{
  size_t page_frame_count = memory.get_frame_count();
  if (page_frame_count < 2 || page_frame_count > kMaxPageFrames) {
    throw std::runtime_error("page_frame_count out of range");
  }
  
  // Initialize all page frames as available except for 0 and frames beyond end
  StoreBit(0, kInUse);    // set page frame 0 unavailable 
  for (size_t i = 1; i < page_frame_count; ++i) {
    StoreBit(i, kFree);   // mark page as available
  }
  for (size_t i = page_frame_count; i < kMaxPageFrames; ++i) {
    StoreBit(i, kInUse);  // mark page frames beyond end as unavailable
  }
  
  // Write count of free page frames to memory
  uint32_t free_count = page_frame_count - 1;
  memory.movb(kFreeCount, &free_count, sizeof(uint32_t));
}

bool BitMapAllocator::GetFrames(uint32_t count, 
                                std::vector<Addr> &page_frames) {
  uint32_t free_count = get_free_count();
  
  // If enough pages available, allocate to caller
  if (count <= free_count) {  // if enough to allocate
    while (count-- > 0) {
      // Return next free frame to caller
      size_t page_frame_addr = GetFirstFree();
      page_frames.push_back(page_frame_addr);
      
      // Clear page frame to all 0
      uint64_t zero = 0;
      for (Addr i = 0; i < kPageSize; i += sizeof(zero)) {
        memory.movb(page_frame_addr + i, &zero, sizeof(zero));
      }
    }

    return true;
  } else {
    return false;  // do nothing and return error
  }
}

bool BitMapAllocator::FreeFrames(uint32_t count,
                                 std::vector<Addr> &page_frames) {
  // If enough to deallocate
  if(count <= page_frames.size()) {
    while(count-- > 0) {
      // Return next frame to bit map
      uint32_t frame_addr = page_frames.back();
      page_frames.pop_back();
      FreeFrame(frame_addr);
    }

    return true;
  } else {
    return false; // do nothing and return error
  }
}

std::string BitMapAllocator::get_bit_map_string(void) const {
  std::ostringstream out_string;
  
  uint8_t map_byte;
  for (Addr i = 0; i < kBitMapBytes; ++i) {
    memory.movb(&map_byte, kBitMapStart + i);
    out_string << " " << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<uint32_t>(map_byte);
  }
  
  return out_string.str();
}

uint32_t BitMapAllocator::get_free_count() const {
  uint32_t free_count;
  memory.movb(&free_count, kFreeCount, sizeof(uint32_t));
  return free_count;
}

void BitMapAllocator::set_free_count(uint32_t free_count) {
  memory.movb(kFreeCount, &free_count, sizeof(uint32_t));
}

uint32_t BitMapAllocator::GetBit(uint32_t frame_num) const {
  uint8_t map_byte;
  memory.movb(&map_byte, kBitMapStart + frame_num/8);  // get byte from map
  return (map_byte >> (frame_num % 8)) & 1;
}

void BitMapAllocator::StoreBit(uint32_t frame_num,
                               uint8_t value) {
  size_t index = frame_num / 8;
  size_t shift = frame_num % 8;
  uint8_t map_byte;
  memory.movb(&map_byte, kBitMapStart + index);  // read old value
  map_byte = (map_byte & ~(1 << shift)) | ((value & 1) << shift);
  memory.movb(kBitMapStart + index, &map_byte);  // rewrite updated value
}

uint32_t BitMapAllocator::GetFirstFree() {
  // Use brute-force search. This could be optimized by keeping track of the
  // last-known lowest free frame.
  
  // Find first non-zero byte in bitmap
  uint8_t map_byte = 0;
  Addr index;
  for (index = 0; index < kBitMapBytes; ++index) {
    memory.movb(&map_byte, kBitMapStart + index);
    if (map_byte != 0) break;
  }
  if (map_byte == 0) return 0;  // no available page frames
  
  // Find lowest numbered non-zero bit
  int shift = 0;
  while (((map_byte >> shift) & 1) == 0) {
    ++shift;
  }
  
  // Mark page frame as in use
  uint32_t frame_num = index * 8 + shift;
  StoreBit(frame_num, kInUse);
  return frame_num * kPageSize;  // page frame address
}

void BitMapAllocator::FreeFrame(uint32_t frame_addr) {
  uint32_t frame_num = frame_addr / kPageSize;
  StoreBit(frame_num, kFree);
  set_free_count(get_free_count() + 1);
}

