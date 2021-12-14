#ifndef DEBUGLOG_H_
#define DEBUGLOG_H_

#include <fstream>
#include <string>
#include <vector>

// debug config
extern bool gcfg_debug_output_to_stdio;  // enable output of debug info to stdio


typedef std::vector<std::string> VecStr;
typedef std::vector<std::wstring> VecWstr;

void DebugOutput();
void DebugLogging(std::string line, bool noEndLine = true);
void DebugLogging(std::wstring line, bool noEndLine = true);
void UpdateLogReset();
void PatchLogReset();

#endif
