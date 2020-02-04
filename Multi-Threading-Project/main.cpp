/* 
 * File:   main.cpp
 *
 * Created on August 10, 2019, 7:01 PM
 */
#include "Trace.h"

#include <cstdlib>
#include <iostream>
#include <MMU.h>

using namespace std;
/*
 * 
 */
int main(int argc, char* argv[]) {
    // Use command line argument as file name
    if (argc != 2) {
        std::cerr << "usage: program2 input_file\n";
        exit(1);
    }

    // Create allocator and page table manager
    mem::MMU memory(64); // fixed memory size of 64 pages
    BitMapAllocator allocator(memory);
    ManagePageTable ptm(memory, allocator);

    // Create the process
    Trace process(argv[1], memory, ptm);

    // Run the commands
    process.RunTrace();
}

