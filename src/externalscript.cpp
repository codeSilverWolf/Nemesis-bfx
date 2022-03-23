#include "Global.h"

//#undef pyslots
//#define MS_NO_COREDLL
#include <Python.h>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
//#define pyslots

#include <QtCore/QProcess>

#include "externalscript.h"

#include "generate/generator_utility.h"
#include "generate/alternateanimation.h"

#include "pylifecycle.h"
#include "utilities/algorithm.h"

using namespace std;

bool dummyScript = false;

void BatchScriptThread(const wstring& filename, const filesystem::path& filepath, bool hidden);
void PythonScriptThread(const wstring& filename, const wchar_t* filepath);

void RunScript(const filesystem::path& directory, bool& hasScript)
{
    if (!isFileExist(directory))
    {
        FolderCreate(directory);
        return;
    }

    bool warning = false;
    VecWstr scriptlist;
    read_directory(directory, scriptlist);

    for (auto& file : scriptlist)
    {
        wstring scriptpath = directory.wstring() + file;
        std::filesystem::path scriptfile(scriptpath);

        // hidden scripts
        if (!std::filesystem::is_directory(scriptfile))
        {
            // bat script
            if (nemesis::iequals(scriptfile.extension().wstring(), L".bat"))
            {
                hasScript = true;
                BatchScriptThread(scriptfile.filename().wstring(), scriptpath, false);
            }
            // python script
            else if (nemesis::iequals(scriptfile.extension().wstring(), L".py"))
            {
                hasScript = true;
                PythonScriptThread(scriptfile.filename().wstring(), scriptpath.c_str());
            }
        }
        // visible scripts
        else if (nemesis::iequals(file, L"show") && std::filesystem::is_directory(scriptfile))
        {
            VecWstr shownscriptlist;
            read_directory(scriptpath, shownscriptlist);

            for (auto& shown : shownscriptlist)
            {
                wstring shownpath = scriptpath + L"\\" + shown;
                std::filesystem::path shownscript(shownpath);

                if (!std::filesystem::is_directory(shownscript))
                {
                    // bat script
                    if (nemesis::iequals(shownscript.extension().wstring(), L".bat"))
                    {
                        hasScript = true;
                        BatchScriptThread(shownscript.filename().wstring(), shownpath, true);
                    }
                    // python script
                    else if (nemesis::iequals(shownscript.extension().wstring(), L".py"))
                    {
                        hasScript = true;
                        PythonScriptThread(shownscript.filename().wstring(), shownpath.c_str());
                    }
                }
            }
        }
    }

    if (hasScript) interMsg("");
}

void BatchScriptThread(const wstring& filename, const filesystem::path& filepath, bool hidden)
{
    try
    {
        wstring msg = TextBoxMessage(1016) + L": " + filename;
        interMsg(msg);
        DebugLogging(msg);

        if (hidden)
        {
            QProcess* p = new QProcess();
            p->start(QString::fromStdWString(filepath));
            p->waitForFinished();
            delete p;
        }
        else
        {
            if (QProcess::execute(QString::fromStdWString(filepath)) != 0) WarningMessage(1023, filepath);
        }
    }
    catch (const exception& ex)
    {
        ErrorMessage(6008, filename, ex.what());
    }
    catch (...)
    {
        ErrorMessage(6008, filename, "Unknown exception");
    }
}

void PythonScriptThread(const wstring& filename, const wchar_t* filepath)
{
    try
    {
        FILE* f;
        wstring msg = TextBoxMessage(1016) + L": " + filename;
        interMsg(msg);
        DebugLogging(msg);
        _wfopen_s(&f, filepath, L"rb");
        // from python documentation:
        // On Windows, fp should be opened as binary mode (e.g. fopen(filename, "rb")). Otherwise, Python may not handle script file with LF line ending correctly.

        if (f)
        {
            try
            {
                // Setup python program name and embedded library paths before calling Py_Initialize().
                // Zip archive containing Python libraries have to be in location 'python_libs/python39.zip'
                // Libraries from lib-dynload have to be copied to 'python_libs/lib-dynload' and left there unpacked!!!
                // Otherwise python can't unpack archive containing rest of libraries.
                // Program name has to be set to nemesis main executable name.

                #ifndef PYTHON_LIBS_ARCH_VER
                    #error PYTHON_LIBS_ARCH_VER should be defined as (python_ver_major)(python_ver_minor) ex. 39 for python 3.9.x
                #endif

                std::filesystem::path current_path = std::filesystem::current_path();
                std::filesystem::path python_lib_path = current_path / (std::wstring(L"python_libs\\python") + std::to_wstring(PYTHON_LIBS_ARCH_VER) + L".zip");
                std::filesystem::path python_dynlib_path = current_path / L"python_libs\\lib-dynload";

                std::wstring python_lib_paths = std::wstring(python_lib_path) + L";" + std::wstring(python_dynlib_path);

                std::error_code ec;
                if (!std::filesystem::exists(python_lib_path, ec))
                {
                    throw std::runtime_error("Python library archive missing! It is NOT a script Error!!! Try reinstalling Nemesis-Bfx. Missing file: " + python_lib_path.u8string());
                }

                Py_SetProgramName(L"Nemesis Unlimited Behavior Engine.exe");
                Py_SetPath(python_lib_paths.c_str());

                Py_Initialize();
                PyRun_SimpleFile(f, nemesis::transform_to<string>(wstring(filepath)).c_str());
                Py_Finalize();
            }
            catch (const exception& ex)
            {
                ErrorMessage(6008, filename, ex.what());
            }
            catch (...)
            {
                ErrorMessage(6008, filename, "Unknown exception");
            }

            fclose(f);
        }
    }
    catch (const exception& ex)
    {
        ErrorMessage(6008, filename, ex.what());
    }
    catch (...)
    {
        ErrorMessage(6008, filename, "Unknown exception");
    }
}
