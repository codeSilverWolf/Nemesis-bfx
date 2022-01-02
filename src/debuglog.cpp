#include "Global.h"
#include "debuglog.h"

#include "utilities/algorithm.h"
#include "utilities/atomiclock.h"
#include <algorithm>
#include <exception>
#include <iostream>
#include <stdexcept>

bool gcfg_debug_output_to_stdio = false;

using namespace std;

VecStr updatelog;
VecStr patchlog;
std::atomic_flag atomlock{};
;
string filename = "CriticalLog.txt";


std::string currentTime()
{
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    return buffer;
}

std::wstring currentTimeW()
{
    return nemesis::transform_to<wstring>(currentTime());
}

void DebugOutput()
{
    //filename.clear();
}

void DebugLogging(string line, bool noEndLine)
{
    int64_t size = count(line.begin(), line.end(), '\n');

    if (noEndLine)
    {
        for (int64_t i = 0; i < size; ++i)
        {
            line.replace(line.find("\n"), 1, " | ");
        }
    }

    try
    {
        Lockless_s lock(atomlock);

        if(gcfg_debug_output_to_stdio)
            std::cout << "DEBUG:[" + currentTime() + "] " + line << std::endl;

        ofstream relog(filename, ios_base::app);
        relog << "[" + currentTime() + "] " + line + "\n";
        relog.close();
    }
    catch (std::exception &e)
    {
        if(gcfg_debug_output_to_stdio)
            std::cout << "DEBUG: exception generated in DebugLogging!!! log files may be incomplete. Exception: " << e.what() << std::endl;
        throw(std::runtime_error(e.what()));
    }

}

void DebugLogging(wstring line, bool noEndLine)
{
    int64_t size = count(line.begin(), line.end(), L'\n');

    if (noEndLine)
    {
        for (int64_t i = 0; i < size; ++i)
        {
            line.replace(line.find(L"\n"), 1, L" | ");
        }
    }

    try
    {
        Lockless_s lock(atomlock);

        if(gcfg_debug_output_to_stdio)
            std::wcout << L"DEBUG:[" + currentTimeW() + L"] " + line << std::endl;

        wofstream relog(filename, ios_base::app);
        relog << L"[" + currentTimeW() + L"] " + line + L"\n";
        relog.close();
    }
    catch (std::exception &e)
    {
        if(gcfg_debug_output_to_stdio)
            std::cout << "DEBUG: exception generated in DebugLogging!!! log files may be incomplete. Exception: " << e.what() << std::endl;
        throw(std::runtime_error(e.what()));
    }

}

void UpdateLogReset()
{
    filename = "UpdateLog.txt";
    ofstream log(filename);
    log.close();
}

void PatchLogReset()
{
    filename = "PatchLog.txt";
    ofstream log(filename);
    log.close();
}
