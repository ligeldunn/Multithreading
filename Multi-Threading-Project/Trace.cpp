/* 
 * File:   Trace.cpp
 * 
 * Created on August 10, 2019, 7:01 PM
 */

#include "Trace.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <memory>


using std::cerr;
using std::cin;
using std::copy;
using std::cout;
using std::dec;
using std::getline;
using std::hex;
using std::istringstream;
using std::setfill;
using std::setw;
using std::vector;

namespace {
  // Define command code for comment or blank line
  const uint32_t kComment = 0xFFFFFFFF;
  
  // Define memory block size
  const uint32_t kBlockSize = 0x400;
}

class PageFaultHandler : public mem::MMU::FaultHandler{
public:

    PageFaultHandler() : fault_count(0) {
    }
    
    virtual bool Run(mem::PSW psw0) {
        last_psw0 = psw0;
        ++fault_count;
        
        //type of fault
        uint32_t fault_type = (psw0 >> mem::kPSW0_OpStateShift) & mem::kPSW0_OpStateMask;
        
              
        mem::Addr next_vaddr = (psw0 >> mem::kPSW0_NextAddrShift) & mem::kPSW0_NextAddrMask;
        if(fault_type == mem::kPSW0_OpRead){
            std::cout << "Read";
        }else {
            std::cout << "Write";
        }
        std::cout << "Page Fault at " << hex << setfill('0') << setw(8) << next_vaddr << "\n";
        
        return false;
    }

    int get_fault_count() const {
        return fault_count;
    }

    void reset_fault_count() {
        fault_count = 0;
    }

    void get_last_psw0(mem::PSW &psw0) const {
        psw0 = last_psw0;
    }
private:
    // Count of number of times handler was called
    int fault_count;

    // PSW0 and 1 from last fault handled
    mem::PSW last_psw0;
};

class WriteFaultHandler : public mem::MMU::FaultHandler {
public:

    WriteFaultHandler() : fault_count(0) {
    }
    
    virtual bool Run(mem::PSW psw0) {
        last_psw0 = psw0;
        ++fault_count;
        
        mem::Addr next_vaddr = (psw0 >> mem::kPSW0_NextAddrShift) & mem::kPSW0_NextAddrMask;
        
        std::cout << "Write Permission Fault at " << hex << setfill('0') << setw(8) << next_vaddr << "\n";
        
        return false;
    }

    int get_fault_count() const {
        return fault_count;
    }

    void reset_fault_count() {
        fault_count = 0;
    }
private:
    // Count of number of times handler was called
    int fault_count;

    // PSW0 from last fault handled
    mem::PSW last_psw0;
};


Trace::Trace(std::string file_name_, mem::MMU &memory_, ManagePageTable &pt_manager_) 
: file_name(file_name_), line_number(0), memory(memory_), pt_manager(pt_manager_) { 
  // Open the trace file.  Abort program if can't open.
  trace.open(file_name, std::ios_base::in);
  if (!trace.is_open()) {
    cerr << "ERROR: failed to open trace file: " << file_name << "\n";
    exit(2);
  }
  
  // Set up user page table
    memory.set_kernel_mode();
    mem::Addr pt_base = pt_manager.CreateProcessPageTable();
    user_psw0 = (static_cast<mem::PSW> (pt_base) << (mem::kPSW0_PageTableShift - mem::kPageSizeBits))
            | (mem::kPSW0_UModeMask << mem::kPSW0_UModeShift)
            | (mem::kPSW0_VModeMask << mem::kPSW0_VModeShift);

    // Create fault handlers
    page_fault_handler = std::make_shared<PageFaultHandler>();
    write_fault_handler = std::make_shared<WriteFaultHandler>();
}

Trace::~Trace() {
  trace.close();
}

void Trace::RunTrace(void) {
    
  //user psw0
  memory.load_user_psw0(user_psw0);
  
  //fault handlers
  memory.SetPageFaultHandler(page_fault_handler);
  memory.SetWritePermissionFaultHandler(write_fault_handler);
  
  // Read and process commands
  std::string line; // text line read
  std::string cmd; // command from line
  vector<uint32_t> hexVals; // arguments from line
  
  // Select the command to execute
  while (InterpretCommand(hexVals)) {
    switch (hexVals[0]) {
      case 0xF01:
        CodeF01(hexVals); // allocate virtual memory
        break;
      case 0xCB1:
        CodeCB1(hexVals); // Compare to Specified Values
        break;
      case 0xCBA:
        CodeCBA(hexVals); // Compare Single Value to Memory Range
        break;
      case 0x301:
        Code301(hexVals); // Set Bytes
        break;
      case 0x30A:
        Code30A(hexVals); // Set Multiple Bytes to Same Value
        break;
      case 0x31D:
        Code31D(hexVals); // Replicate Range of Bytes From Source to Destination
        break;
      case 0x4F0:
        Code4F0(hexVals); // Output Bytes
        break;
      case 0xFF1:
        CodeFF1(hexVals);
        break;
      case 0xFF0:
        CodeFF0(hexVals);
        break;
      case kComment:
        break;
      default:
        cerr << "ERROR: invalid command\n";
        exit(2);
    }
  }
}

bool Trace::InterpretCommand(vector<uint32_t> &hexVals) {
  hexVals.clear();
  std::string textLine;
  
  // Read next textLine
  if (getline(trace, textLine)) {
    ++line_number;
    cout << dec << line_number << ":" << textLine << "\n";
    
    // No further processing if comment
    if (textLine.empty() || textLine[0] == '*') {
      hexVals.push_back(kComment);
      return true;
    }
    
    // Make a string stream from command line
    istringstream lineStream(textLine);
    
    // Read the hex values from the line
    uint32_t hVal;
    while (lineStream >> hex >> hVal) {
      hexVals.push_back(hVal);
    }
    
    // If no values read, set as comment
    if (hexVals.empty()) hexVals.push_back(kComment);
    
    return true;
  }
  
  // Check for eof or error
  if (trace.eof()) {
    return false;
  } else {
    cerr << "ERROR: getline failed on trace file: " << file_name 
            << " at line " << line_number << "\n";
    exit(2);
  }
}

void Trace::CodeF01(const vector<uint32_t> &hexVals) {
  if (hexVals.size() == 3) {
      uint32_t count = hexVals.at(1);
      mem::Addr vaddr = hexVals.at(2);
      
      //check to see if vaddr is a multiple of 0x400
      if(vaddr % 1024 == 0) {
          memory.set_kernel_mode();
          pt_manager.MapProcessPages(user_psw0, vaddr, count);
          memory.load_user_psw0(user_psw0);
      }else {
          cerr << "ERROR: virtual address is not a multiple of page size";
      }
      
  } else {
       cerr << "ERROR: badly formatted command\n";
       exit(2);
  }
    
}

void Trace::CodeCB1(const vector<uint32_t> &hexVals) {
  // Compare to Specified Values
  mem::Addr addr = hexVals.at(1);
  uint8_t byte_at_addr; 
  // Compare specified byte values
  for (int i = 2; i < hexVals.size(); ++i) {
    memory.movb(&byte_at_addr, addr);
    if(byte_at_addr != hexVals.at(i)) {
      cout << "compare error at address " << hex << setw(8) << setfill('0')
              << addr
              << ", expected " << setw(2) << static_cast<uint32_t>(hexVals.at(i))
              << ", actual is " << setw(2) << static_cast<uint32_t>(byte_at_addr) << "\n";
    }
    ++addr;
  }
}

void Trace::CodeCBA(const vector<uint32_t> &hexVals) {
  // Compare Single Value to Memory Range
  uint32_t count = hexVals.at(1);
  mem::Addr addr = hexVals.at(2);
  uint32_t val = hexVals.at(3);
  uint8_t byte_at_addr;
  
  // Compare specified byte values
  for (uint32_t i = 0; i < count; ++i) {
    memory.movb(&byte_at_addr, addr + i);
    if(byte_at_addr != val) {
      cout << "compare error at address " << hex << setw(8) << setfill('0')
              << addr+i
              << ", expected " << setw(2) << val
              << ", actual is " << setw(2) << static_cast<uint32_t>(byte_at_addr) << "\n";
    }
  }
}

void Trace::Code301(const vector<uint32_t> &hexVals) {
  // Store multiple bytes starting at specified address
  mem::Addr addr = hexVals.at(1);
  for (int i = 2; i < hexVals.size(); ++i) {
      mem::Addr new_addr = addr++;
      uint8_t value = hexVals.at(i);
     memory.movb(new_addr, &value);
  }
}

void Trace::Code31D(const vector<uint32_t> &hexVals) {
  // Replicate Range of Bytes From Source to Destination
    uint32_t nextTemp = hexVals.at(1);
    mem::Addr nextTemp1 = hexVals.at(2);
    mem::Addr nextTemp2 = hexVals.at(3); 
    uint8_t byte_at_addr;
    
    for (uint32_t i = 0; i < nextTemp; i++) {
        memory.movb(&byte_at_addr, nextTemp2);
        memory.movb(nextTemp1, &byte_at_addr);

        nextTemp1++;
        nextTemp2++;

    }
}

void Trace::Code30A(const vector<uint32_t> &hexVals) {
  // Set Multiple Bytes to Same Value
  uint8_t value = hexVals.at(3);
  uint32_t count = hexVals.at(1);
  uint32_t addr = hexVals.at(2);
  for (int i = 0; i < count; ++i) {
    memory.movb(addr++, &value);
  }
}

void Trace::Code4F0(const vector<uint32_t> &hexVals) {
  // Output bytes
  mem::Addr addr = hexVals.at(2);
  uint32_t count = hexVals.at(1);
  uint8_t byte_at_addr;

  // Output the specified number of bytes starting at the address
  for (int i = 0; i < count; ++i) {
    if ((i % 16) == 0) { // Write new line with address every 16 bytes
      if (i > 0) cout << "\n";  // not before first line
      cout << hex << setw(8) << setfill('0') << addr << ": ";
    } else {
      cout << ",";
    }
    mem::Addr new_addr = addr++;
    
    memory.movb(&byte_at_addr, new_addr);
    
    cout << setfill('0') << setw(2)
            << static_cast<uint32_t> (byte_at_addr);
  }
  cout << "\n";
}

void Trace::CodeFF0(const std::vector<uint32_t>& hexVals){
    if (hexVals.size() == 3) {
        uint32_t count = hexVals.at(1);
        mem::Addr vaddr = hexVals.at(2);

        //check to see if vaddr is a multiple of 0x400
        if (vaddr % 1024 == 0) {
            memory.set_kernel_mode();
            pt_manager.SetPageWritePermission(user_psw0, vaddr, count, 0);
            memory.load_user_psw0(user_psw0);
        } else {
            cerr << "ERROR: virtual address is not a multiple of page size";
        }

    } else {
        cerr << "ERROR: badly formatted command\n";
        exit(2);
    }
}

void Trace::CodeFF1(const std::vector<uint32_t>& hexVals){
    if (hexVals.size() == 3) {
        uint32_t count = hexVals.at(1);
        mem::Addr vaddr = hexVals.at(2);

        //check to see if vaddr is a multiple of 0x400
        if (vaddr % 1024 == 0) {
            memory.set_kernel_mode();
            pt_manager.SetPageWritePermission(user_psw0, vaddr, count, 1);
            memory.load_user_psw0(user_psw0);
        } else {
            cerr << "ERROR: virtual address is not a multiple of page size";
        }

    } else {
        cerr << "ERROR: badly formatted command\n";
        exit(2);
    }
}