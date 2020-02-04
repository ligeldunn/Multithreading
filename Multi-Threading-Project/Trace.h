/* 
 * File:   Trace.h
 *
 * Created on August 10, 2019, 7:01 PM
 */

#ifndef TRACE_H
#define TRACE_H

#include "BitMapAllocator.h"
#include "ManagePageTable.h"
#include <MMU.h>

#include <fstream>
#include <string>
#include <vector>

class Trace {
public:
  /**
   * Constructor - open trace file, initialize processing
   * 
   * @param file_name_ source of trace commands
   */
  Trace(std::string file_name_, mem::MMU &memory_, ManagePageTable &pt_manager_);
  
  /**
   * Destructor - close trace file, clean up processing
   */
  virtual ~Trace(void);

  // Other constructors, assignment: prevent copy and move
  Trace(const Trace &other) = delete;
  Trace(Trace &&other) = delete;
  Trace operator=(const Trace &other) = delete;
  Trace operator=(Trace &&other) = delete;
  
  /**
   * Run - read and process commands from trace file
   * 
   */
  void RunTrace(void);
  
private:
  // Trace file
  std::string file_name;
  std::fstream trace;
  long line_number;
  
  // physical memory
  mem::MMU &memory;
  
  //Manage Page Table
  ManagePageTable &pt_manager;
  
  //user psw
  mem:: PSW user_psw0;
  
  //fault handlers
  std::shared_ptr<mem::MMU::FaultHandler> page_fault_handler;
  std::shared_ptr<mem::MMU::FaultHandler> write_fault_handler;
    
  
  /**
   * InterpretCommand - interpret next trace file command.
   *   Aborts program if invalid trace file.
   * 
   * @param hexVals returns a vector of the command line values
   * @return true if command parsed, false if end of file
   */
  bool InterpretCommand(std::vector<uint32_t> &hexVals);
  
  /**
   * Command processors. Arguments are the same for each command.
   *   Form of the function is CmdX, where "X' is the command code.
   * @param hexVals command code and arguments
   */
  void CodeF01(const std::vector<uint32_t> &hexVals);  // allocate virtual memory
  void CodeCB1(const std::vector<uint32_t> &hexVals);  // Compare to Specified Values
  void CodeCBA(const std::vector<uint32_t> &hexVals);  // Compare Single Value to Memory Range
  void Code301(const std::vector<uint32_t> &hexVals);  // Set Bytes
  void Code30A(const std::vector<uint32_t> &hexVals);  // Set Multiple Bytes to Same Value
  void Code31D(const std::vector<uint32_t> &hexVals);  // Replicate Range of Bytes From Source to Destination
  void Code4F0(const std::vector<uint32_t> &hexVals);  // Output Bytes
  void CodeFF1(const std::vector<uint32_t> &hexVals); 
  void CodeFF0(const std::vector<uint32_t> &hexVals); 
};

#endif /* TRACE_H */





