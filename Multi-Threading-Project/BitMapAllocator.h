/* 
 * File:   BitMapAllocator.h
 *
 * Created on August 10, 2019, 9:33 PM
 */

#ifndef BITMAPALLOCATOR_H
#define BITMAPALLOCATOR_H

#include <MMU.h>

#include <cstdint>
#include <string>
#include <vector>

class BitMapAllocator {
public:
  /**
   * Constructor
   * 
   * Built bit map of free page frames.
   * 
   * @param memory MMU object
   */
  BitMapAllocator(mem::MMU &memory_);
  
  virtual ~BitMapAllocator() {}  // empty destructor
  
  // Disallow copy/move
  BitMapAllocator(const BitMapAllocator &other) = delete;
  BitMapAllocator(BitMapAllocator &&other) = delete;
  BitMapAllocator &operator=(const BitMapAllocator &other) = delete;
  BitMapAllocator &operator=(BitMapAllocator &&other) = delete;
  
  /**
   * GetFrames - allocate page frames from the free list
   * 
   * @param count number of page frames to allocate
   * @param page_frames page frame addresses allocated are pushed on back
   * @return true if success, false if insufficient page frames (no frames allocated)
   */
  bool GetFrames(uint32_t count, std::vector<mem::Addr> &page_frames);
  
  /**
   * FreeFrames - return page frames to free list
   * 
   * @param count number of page frames to free
   * @param page_frames contains page frame addresses to deallocate; addresses
   *   are popped from back of vector
   * @return true if success, false if insufficient page frames in vector
   */
  bool FreeFrames(uint32_t count, std::vector<mem::Addr> &page_frames);
  
  // Functions to return list info
  uint32_t get_free_count(void) const;
  
  /**
   * get_bit_map_string - get string representation of bit map
   * 
   * @return hex values of bit map bytes
   */
  std::string get_bit_map_string(void) const;
  
private:
  // Constants for free and available bits
  uint32_t kFree = 1;
  uint32_t kInUse = 0;
  
  // MMU for storage
  mem::MMU &memory;
  
  // Maximum number of page frames in memory
  static const mem::Addr kMaxPageFrames = 0x100;
    
  // Location to store current number of free page frames
  static const mem::Addr kFreeCount = 0;

  // Address of start of bit map in memory (just after free count)
  static const mem::Addr kBitMapStart = kFreeCount + sizeof(uint32_t);
  
  // Number of bytes in bit map
  static const mem::Addr kBitMapBytes = (kMaxPageFrames + 7) / 8;
  
  void set_free_count(uint32_t free_count);
  
  /**
   * StoreBit - store the value of a bit map bit
   * 
   * @param frame_num bit number to store
   * @param value new value, 0 or 1
   */
  void StoreBit(uint32_t frame_num, uint8_t value);
  
  /**
   * GetBit - get bit value from bit map.
   * @param frame_num page frame number
   * @return kFree or kInUse for that page frame
   */
  uint32_t GetBit(uint32_t frame_num) const;

  /**
   * GetFirstFree - mark as in-use the first free page frame
   * 
   * @return page frame address or 0 if none available
   */
  uint32_t GetFirstFree();

  /**
   * FreeFrame - free the specified frame number
   * @param frame_addr page frame address
   */
  void FreeFrame(mem::Addr frame_addr);
};

#endif /* BITMAPALLOCATOR_H */

