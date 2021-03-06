#include <locale>
#include "Global.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

#include <QtGui/QIcon>
#include <QtWidgets/QApplication>


#include "ui/CmdLaunch.h"
#include "ui/ErrorMsgBox.h"
#include "ui/MessageHandler.h"
#include "ui/MultiInstanceCheck.h"
#include "ui/NemesisEngine.h"

#include <windows.h>

extern std::wstring stagePath;

int main(int argc, char* argv[])
{
    // set locale for libstdc++ to ".UTF-8" as first thing. This is needed to support
    // non ASCII characters in file paths and <iostream> functions!.
    char *lc_ctype = std::setlocale(LC_CTYPE, ".UTF-8");

    bool generate = false;
    bool update   = false;
    VecStr modlist;
    std::string logfile = "CriticalLog.txt";

    #ifdef _WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        
        // console is active - we can output dbg info to stdio
        gcfg_debug_output_to_stdio = true;
        std::cout << "Reattached to Console\n";

        // TODO: display huge warning in main window when unicode page was not set.
        // After all when not running from console, warning will not be issued.
        if(lc_ctype != nullptr)
            std::cout << "LC_CTYPE set to: \"" << lc_ctype << "\"" << std::endl;
        else
            std::cout << "ERROR setting LC_CTYPE set to \".UTF-8\" !!!" << std::endl;
    }
    #endif

    try
    {
        if (isFileExist(logfile) && !std::filesystem::is_directory(logfile)) std::filesystem::remove(logfile);
    }
    catch (const std::exception&)
    {
        // empty
    }

    QApplication a(argc, argv);

    try
    {
        if (argc > 1)
        {
            for (uint i = 1; i < argc; ++i)
            {
                if (nemesis::iequals(argv[i], "-update"))
                {
                    if (generate)
                    {
                        std::cout << "Invalid arguments. \"update\" argument and \"generate\" arugment "
                                     "cannot be used simultaneously";
                        std::cout << "Failed to generate behavior";
                        return 1;
                    }

                    update = true;
                }
                if (nemesis::iequals(argv[i], "-generate"))
                {
                    if (update)
                    {
                        std::cout << "Invalid arguments. \"update\" argument and \"generate\" arugment "
                                     "cannot be used simultaneously";
                        std::cout << "Failed to generate behavior";
                        return 1;
                    }

                    generate = true;
                }
                else if (std::string_view(argv[i]).find("-stage=") == 0)
                {
                    stagePath = nemesis::transform_to<std::wstring>(std::string(argv[i] + 7));

                    if (stagePath.size() > 0 && stagePath[0] == L'\"') stagePath = stagePath.substr(1);

                    if (stagePath.size() > 0 && stagePath.back() == L'\"') stagePath.pop_back();
                }
                else
                {
                    modlist.push_back(argv[i]);
                }
            }
        }

        if (generate)
        {
            if (!isFileExist("languages"))
            {
                CEMsgBox* msg = new CEMsgBox;
                msg->setWindowTitle("ERROR");
                msg->setText("Error: \"languages\" folder not found. Please reinstall Nemesis");
                msg->show();
            }
            else if (!isFileExist("languages\\english.txt"))
            {
                CEMsgBox* msg = new CEMsgBox;
                msg->setWindowTitle("ERROR");
                msg->setText(
                    "Error: \"english.txt\" file not found in language folder. Please reinstall Nemesis");
                msg->show();
            }
            else
            {
                NewDebugMessage(*new DebugMsg(L"english"));
                NemesisInfo* nemesisInfo = new NemesisInfo;
                CmdGenerateInitialize(modlist, nemesisInfo);
                delete nemesisInfo;
            }
        }
        else if (update)
        {
            if (!isFileExist("languages"))
            {
                CEMsgBox* msg = new CEMsgBox;
                msg->setWindowTitle("ERROR");
                msg->setText("Error: \"languages\" folder not found. Please reinstall Nemesis");
                msg->show();
            }
            else if (!isFileExist("languages\\english.txt"))
            {
                CEMsgBox* msg = new CEMsgBox;
                msg->setWindowTitle("ERROR");
                msg->setText(
                    "Error: \"english.txt\" file not found in language folder. Please reinstall Nemesis");
                msg->show();
            }
            else
            {
                NewDebugMessage(*new DebugMsg(L"english"));
                NemesisInfo* nemesisInfo = new NemesisInfo;
                CmdUpdateInitialize(nemesisInfo);
                delete nemesisInfo;
            }
        }
        else if (programInitiateCheck(&a))
        {
            NemesisEngine w;
            w.setWindowIcon(QIcon(":/icon/title icon.png"));
            w.show();
            return a.exec();
        }

        return a.exec();
    }
    catch (const std::exception& ex)
    {
        QMessageBox::information(nullptr, "Exception Caught", ex.what(), QMessageBox::Ok);
    }

    return 0;
}
