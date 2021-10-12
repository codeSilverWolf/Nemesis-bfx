#ifndef DEBUGMSG_H_
#define DEBUGMSG_H_

#include <filesystem>
#include <mutex>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "debuglog.h"

#include "utilities/algorithm.h"

#include "ui/MessageHandler.h"

extern bool error; // get error warning
extern bool isPatch;

extern VecWstr warningMsges;
extern std::mutex err_Mutex;

namespace nemesis
{
    class exception
    {};
} // namespace nemesis

// debugging
struct DebugMsg
{
    std::unordered_map<int, std::wstring> errorlist;
    std::unordered_map<int, std::wstring> warninglist;
    std::unordered_map<int, std::wstring> uilist;
    std::unordered_map<int, std::wstring> textlist;

    DebugMsg()
    {}
    DebugMsg(std::string language);
    DebugMsg(std::wstring language);

private:
    void setup(const std::wstring& language);
};

// add new language pack
void NewDebugMessage(DebugMsg NewLog);

std::wstring DMLogError(int errorcode);
std::wstring DMLogWarning(int warningcode);

std::string EngLogError(int errorcode);
std::string EngLogWarning(int warningcode);

int sameWordCount(std::string, std::string);
int sameWordCount(std::wstring, std::wstring);


// end of recursion case
inline void AdditionalInput(std::string& message, int counter)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    int ref                 = sameWordCount(message, newInput);

    if (ref == 0)
    {
        // all ok
        return;
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Not enough parameters for AdditionalInput. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename type>
inline void AdditionalInput(std::string& message, int counter, type input)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    // According to standard operator << on std::ostringstream returns std::basic_ostream which DO NOT have .str() member function,
    // so code like:  std::string replacement = (std::ostringstream() << input).str();
    // won't work. We need to use variable. (or static_cast???)
    std::ostringstream tmpostr {};
    tmpostr << input;
    std::string replacement = tmpostr.str();
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

inline void AdditionalInput(std::string& message, int counter, const std::wstring& input)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = nemesis::transform_to<std::string>(input);
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

// my addition
template <>
inline void AdditionalInput(std::string& message, int counter, const std::string& input)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = input;
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <>
inline void AdditionalInput(std::string& message, int counter, const std::wstring_view& input)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = nemesis::transform_to<std::string>(input);
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <>
inline void AdditionalInput(std::string& message, int counter, const std::filesystem::path& input)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = input.string();
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

//my addition
template <>
inline void AdditionalInput(std::string& message, int counter, const unsigned int& input)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = std::to_string(input);
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

// forward declarations
template <typename... other>
inline void AdditionalInput(std::string& message, int counter, const unsigned int& input, other... rest);
template <typename... other>
inline void AdditionalInput(std::string& message, int counter, const std::filesystem::path& input, other... rest);

// my addition
template <typename... other>
inline void AdditionalInput(std::string& message, int counter, const char* input, other... rest)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = input;
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}
// my addition
template <typename... other>
inline void AdditionalInput(std::string& message, int counter, const std::string& input, other... rest)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = input;
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}


// my addition
template <typename... other>
inline void AdditionalInput(std::string& message, int counter, const unsigned int& input, other... rest)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = std::to_string(input);
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename... other>
inline void AdditionalInput(std::string& message, int counter, const std::wstring& input, other... rest)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = nemesis::transform_to<std::string>(input);
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename... other>
inline void AdditionalInput(std::string& message, int counter, const std::wstring_view& input, other... rest)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = nemesis::transform_to<std::string>(input);
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename... other>
inline void AdditionalInput(std::string& message, int counter, const std::filesystem::path& input, other... rest)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::string replacement = input.string();
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename type, typename... other>
inline void AdditionalInput(std::string& message, int counter, type input, other... rest)
{
    std::string newInput    = "<" + std::to_string(counter) + ">";
    std::ostringstream tmpostr {};
    tmpostr << input;
    std::string replacement = tmpostr.str();
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

// Here starts AdditionalInput for message of type std::wstring

// my addition - recurence ending function
inline void AdditionalInput(std::wstring& message, int counter)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    int ref                  = sameWordCount(message, newInput);

    if (ref == 0)
    {
        // end recursion
        return;
    }
    else
    {
        std::wstring msg = L"CRITICAL ERROR: Wrong error input. To few parameters wstring ver??? Please re-install Nemesis";
        interMsg(msg + L"\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

inline void AdditionalInput(std::wstring& message, int counter, const std::string& input)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wstring replacement = nemesis::transform_to<std::wstring>(input);
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::wstring msg = L"CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + L"\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

inline void AdditionalInput(std::wstring& message, int counter, const std::string_view& input)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wstring replacement = nemesis::transform_to<std::wstring>(input);
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::wstring msg = L"CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + L"\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

inline void AdditionalInput(std::wstring& message, int counter, const std::filesystem::path& input)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wstring replacement = input.wstring();
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::wstring msg = L"CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + L"\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

// wchar AdditionalInput forward declations
template <typename... other>
inline void AdditionalInput(std::wstring& message, int counter, const std::filesystem::path& input, other... rest);


// my addition
inline void AdditionalInput(std::wstring& message, int counter, const int& input)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wstring replacement = std::to_wstring(input);
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::wstring msg = L"CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + L"\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename type>
inline void AdditionalInput(std::wstring& message, int counter, type input)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wostringstream tmpwostr {};
    tmpwostr << input;
    std::wstring replacement = tmpwostr.str();
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }
    }
    else
    {
        std::wstring msg = L"CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + L"\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

// my addition
template <typename... other>
inline void AdditionalInput(std::wstring& message, int counter, const int& input, other... rest)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wstring replacement = std::to_wstring(input);
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

// my addition
template <typename... other>
inline void AdditionalInput(std::wstring& message, int counter, const char* input, other... rest)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::string tmpstr{input};
    std::wstring replacement = nemesis::transform_to<std::wstring>(tmpstr);
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}


template <typename... other>
inline void AdditionalInput(std::wstring& message, int counter, const std::string& input, other... rest)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wstring replacement = nemesis::transform_to<std::wstring>(input);
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename... other>
inline void AdditionalInput(std::wstring& message, int counter, const std::string_view& input, other... rest)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wstring replacement = nemesis::transform_to<std::wstring>(input);
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename... other>
inline void AdditionalInput(std::wstring& message, int counter, const std::filesystem::path& input, other... rest)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wstring replacement = input.wstring();
    int ref                  = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

template <typename type, typename... other>
inline void AdditionalInput(std::wstring& message, int counter, type input, other... rest)
{
    std::wstring newInput    = L"<" + std::to_wstring(counter) + L">";
    std::wostringstream tmpwostr {};
    tmpwostr << input;
    std::wstring replacement = tmpwostr.str();
    int ref                 = sameWordCount(message, newInput);

    if (ref != 0)
    {
        for (int i = 0; i < ref; ++i)
        {
            message.replace(message.find(newInput), newInput.size(), replacement);
        }

        AdditionalInput(message, counter + 1, rest...);
    }
    else
    {
        std::string msg = "CRITICAL ERROR: Wrong error input. Please re-install Nemesis";
        interMsg(msg + "\n");
        DebugLogging(msg);
        error = true;
        return;
    }
}

// error
void ErrorMessage(int errorcode);

template <typename... other>
inline void ErrorMessage(int errorcode, other... rest)
{
    std::scoped_lock<std::mutex> err_Lock(err_Mutex);

    if (error) throw nemesis::exception();

    error                 = true;
    std::wstring errormsg = L"ERROR(" + std::to_wstring(errorcode) + L"): " + DMLogError(errorcode);
    std::string englog    = "ERROR(" + std::to_string(errorcode) + "): " + EngLogError(errorcode);

    if (DMLogError(errorcode).length() == 0)
    {
        errormsg
            = L"CRITICAL ERROR: Error code not found. Unable to diagnose problem. Please re-install Nemesis";
        interMsg(errormsg + L"\n");
        return;
    }

    AdditionalInput(errormsg, 1, rest...);
    AdditionalInput(englog, 1, rest...);
    interMsg(errormsg + L"\n");
    DebugLogging(englog);
    throw nemesis::exception();
}

// warning
void WarningMessage(int warningcode);

template <typename... other>
inline void WarningMessage(int warningcode, other... rest)
{
    std::scoped_lock<std::mutex> err_Lock(err_Mutex);

    if (error) throw nemesis::exception();

    std::wstring warninmsg = L"WARNING(" + std::to_wstring(warningcode) + L"): " + DMLogWarning(warningcode);
    std::string englog    = "WARNING(" + std::to_string(warningcode) + "): " + EngLogWarning(warningcode);

    if (DMLogWarning(warningcode).length() == 0)
    {
        warninmsg
            = L"CRITICAL ERROR: Error code not found. Unable to diagnose problem. Please re-install Nemesis";
        interMsg(warninmsg + L"\n");
        error = true;
        return;
    }

    AdditionalInput(warninmsg, 1, rest...);
    AdditionalInput(englog, 1, rest...);
    warningMsges.push_back(warninmsg + L"\n");
    DebugLogging(englog);
}

// TextBox
std::wstring TextBoxMessage(int textcode);
std::string EngTextBoxMessage(int textcode);

// UI
std::wstring UIMessage(int uicode);

#endif
