#include "Global.h"

#include <atomic>
#include <filesystem>
#include <ctime>
#include <sys/stat.h>

#include "utilities/algorithm.h"
#include "utilities/atomiclock.h"
#include "utilities/lastupdate.h"

using namespace std;

atomic_flag updateLock = ATOMIC_FLAG_INIT;

void saveLastUpdate(const wstring& filepath, unordered_map<wstring, wstring>& lastUpdate)
{
    saveLastUpdate(filepath.c_str(), lastUpdate);
}

void saveLastUpdate(const wstring_view& filename, unordered_map<wstring, wstring>& lastUpdate)
{
    saveLastUpdate(nemesis::transform_to<wstring>(filename).c_str(), lastUpdate);
}

void saveLastUpdate(const string& filepath, unordered_map<wstring, wstring>& lastUpdate)
{
    saveLastUpdate(nemesis::transform_to<wstring>(filepath).c_str(), lastUpdate);
}

void saveLastUpdate(const string_view& filename, unordered_map<wstring, wstring>& lastUpdate)
{
    saveLastUpdate(nemesis::transform_to<wstring>(filename).c_str(), lastUpdate);
}

void saveLastUpdate(const wchar_t* filename, unordered_map<wstring, wstring>& lastUpdate)
{
    try
    {
        Lockless_s lock(updateLock);
        lastUpdate[filename] = GetLastModified(filename);
    }
    catch (exception& ex)
    {
        ErrorMessage(6001, ex.what());
    }
}

string GetLastModified(string filename)
{
    try
    {
        struct stat buf;
        stat(filename.data(), &buf);
        char buffer[26];
        size_t nret = std::strftime(buffer, 26, "%c", std::localtime(&buf.st_mtime));
        if(nret > 0)
        {
            return buffer;
        }
    }
    catch (...)
    {
        // ErrorMessage(2022);
    }

    return "";
}

wstring GetLastModified(wstring filename)
{
    try
    {
        struct _stat64i32 buf;
        _wstat(filename.data(), &buf);
        wchar_t buffer[26];
        size_t nret = std::wcsftime(buffer, 26, L"%c", std::localtime(&buf.st_mtime));
        if(nret > 0)
        {
            return buffer;
        }
    }
    catch (...)
    {
        // ErrorMessage(2022);
    }

    return L"";
}
