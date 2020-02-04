/* 
 * File:   ManagePageTable.h
 *
 * Created on August 10, 2019, 9:01 PM
 */

#include "BitMapAllocator.h"

#include <MMU.h>

class ManagePageTable {
public:
/**
* Constructor - build kernel page table and enter virtual mode
* 
* Must be in physical memory mode on entry. Creates kernel page table in MMU, 
* enters virtual mode.
* 
* @param memory_ MMU class object to use for memory
* @param allocator_ page frame allocator object
* @throws std::runtime_error if unable to allocate memory for page table
*/
ManagePageTable(mem::MMU &memory_, BitMapAllocator &allocator_);

virtual ~ManagePageTable() {
} // empty destructor

// Disallow copy/move
ManagePageTable(const ManagePageTable &other) = delete;
ManagePageTable(ManagePageTable &&other) = delete;
ManagePageTable &operator=(const ManagePageTable &other) = delete;
ManagePageTable &operator=(ManagePageTable &&other) = delete;

/**
* CreateProcessPageTable - create empty page table for process
* 
* All entries in the page table will be 0 (not present). Must be called
* in kernel mode.
* 
* @return kernel address of new page table
* @throws std::runtime_error if unable to allocate memory for page table
*/
mem::Addr CreateProcessPageTable(void);

/**
* MapProcessPages - map pages into the memory of specified process
* 
* The requested pages are allocated and mapped into the page table of
* the process whose PMCB is specified. Any pages
* already mapped are ignored. Must be called in kernel mode.
* 
* @param psw0 PSW0 of process to modify
* @param vaddr starting virtual address
* @param count number of pages to map
* @throws std::runtime_error if unable to allocate memory for pages
*/
void MapProcessPages(mem::PSW psw0, mem::Addr vaddr, size_t count);

/**
* SetPageWritePermission - change writable bit for page(s)
* 
* Must be called in kernel mode.
* 
* @param psw0 PSW0 of process to modify
* @param vaddr starting virtual address
* @param count number of pages to change
* @param writable non-zero to set writable bit, 0 to clear it
*/
void SetPageWritePermission(
mem::PSW psw0, mem::Addr vaddr, size_t count, uint32_t writable);

private:
// Save references to memory and allocator
mem::MMU &memory;
BitMapAllocator &allocator;
};

