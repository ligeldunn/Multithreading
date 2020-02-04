/* 
 * File:   ManagePageTable.cpp
 * 
 * Created on August 10, 2019, 9:01 PM
 */

#include "ManagePageTable.h"
#include <iostream>

ManagePageTable::ManagePageTable(mem::MMU &memory_, BitMapAllocator &allocator_): memory(memory_), allocator(allocator_){
    //virtual memory 
    std::vector<mem::Addr> page_frames;
    mem::Addr kernel_pt_addr;
    
    //allocate kernel page table
    if((allocator.GetFrames(1, page_frames))){
        kernel_pt_addr = page_frames.at(0);
        
        // Build page table entries (map to end of existing memory)
        mem::PageTable page_table;
        mem::Addr num_pages = memory.get_frame_count();
        
        for(mem::Addr i = 0; i < num_pages; ++i){
            page_table.at(i) = (i << mem::kPageSizeBits) | mem::kPTE_PresentMask | mem::kPTE_WritableMask;
        }
        // Write page table to MMU memory, by using movb
        memory.movb(kernel_pt_addr, &page_table, mem::kPageTableSizeBytes);
        // Build PSW0 to set page table and enter virtual mode
        mem::PSW psw0 = (static_cast<mem::PSW> (kernel_pt_addr) << (mem::kPSW0_PageTableShift - mem::kPageSizeBits)) | mem::kPSW0_VModeMask;
        
        memory.load_kernel_psw0(psw0);
        
    }else {
        throw std::runtime_error("Error: could not allocate Kernel Page Table");
    }
}

void ManagePageTable::MapProcessPages(mem::PSW psw0, mem::Addr vaddr, size_t count){
    // Allocate count number of pages, use GetFrames!
    std::vector<mem::Addr> page_frames;
    if(allocator.GetFrames(count, page_frames)){
        // Map the allocated pages
        mem::Addr next_vaddr = vaddr;

        //First calculate the base:
        mem::Addr pt_base = ((psw0 >> mem::kPSW0_PageTableShift) & mem::kPSW0_PageTableMask) << mem::kPageSizeBits;
        //calculate the current index:
        while (count-- > 0) { //START WHILE LOOP

            mem::Addr pt_index = next_vaddr >> mem::kPageSizeBits;

            // Read existing page table entry
            mem::PageTableEntry pt_entry;

            //set the address to the base + the index times the size of a pagetableentry
            mem::Addr pte_addr = pt_base + (pt_index * sizeof(pt_entry));
                    //Use moveb to read from this pte address in memory to your pagetableentry object
            memory.movb(&pt_entry, pte_addr, sizeof(pt_entry));
                    // If page not mapped, create and write page table entry
            if ((pt_entry & mem::kPTE_PresentMask) == 0) {
                pt_entry = page_frames.back() | mem::kPTE_PresentMask | mem::kPTE_WritableMask;//get the last entry in the allocated vector and mask it with present and writable
                //pop to make page_frames.back() valid for next iteration
                page_frames.pop_back();
                        //Then store it into memory by using moveb
                memory.movb(pte_addr, &pt_entry, sizeof(pt_entry));
            }

            //set the next address
            next_vaddr += mem::kPageSize;
        } //END WHILE LOOP

        // Release any left over page frames (if some were already mapped)
        //If allocator vector not empty, use FreeFrames on the remaining pageframes in the vector
        if(!page_frames.empty()){
            allocator.FreeFrames(page_frames.size(), page_frames);
        }
    }else{
        throw std::runtime_error("Error: could not allocate Process Pages");
    }
    
}

mem::Addr ManagePageTable::CreateProcessPageTable(){
    std::vector<mem::Addr> page_frames;
    mem::Addr pt_page_table;
    mem::PageTable page_table;
    
    if(allocator.GetFrames(1, page_frames)){
        pt_page_table = page_frames.at(0);
        memory.movb(pt_page_table, &page_table, mem::kPageTableSizeBytes);
        return pt_page_table;
    }else{
        std::cerr << "Error: could not create process page table";
    }
}

void ManagePageTable::SetPageWritePermission(mem::PSW psw0, mem::Addr vaddr, size_t count, uint32_t writable){
    std::vector<mem::Addr> page_frames;
    
    if(allocator.GetFrames(count, page_frames)){
        
        mem::Addr pt_base = ((psw0 >> mem::kPSW0_PageTableShift) & mem::kPSW0_PageTableMask) << mem::kPageSizeBits;
        mem::Addr pt_index = vaddr >> mem::kPageSizeBits;
        mem::Addr next_vaddr = vaddr;
        
        while(count-- > 0){
            mem::PageTableEntry pt_entry;
            
            mem::Addr pt_index = next_vaddr >> mem::kPageSizeBits;
            
            mem::Addr pte_addr = pt_base + (pt_index * sizeof (pt_entry));

            memory.movb(&pt_entry, pte_addr, sizeof (pt_entry));
            
            //check to see if mapped
            if ((pt_entry & mem::kPTE_PresentMask) != 0) {
                
                if(writable != 0){
                    //enable the 5th bit of page entry
                    pt_entry = (pt_entry & ~mem::kPTE_WritableMask)  | mem::kPTE_WritableMask;
                }else {
                    //disable the 5th bit of page entry with 0
                    pt_entry = pt_entry & ~mem::kPTE_WritableMask;
                }
                
                memory.movb(pte_addr, &pt_entry, sizeof(pt_entry));
            }
            next_vaddr += mem::kPageSize;
        }
    }else {
        throw std::runtime_error("Error: could not allocate Process Pages");
    }
    
}


