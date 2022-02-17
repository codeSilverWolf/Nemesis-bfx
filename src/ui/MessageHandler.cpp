#include "Global.h"

#include "connector.h"

#include "debuglog.h"

#include "ui/ErrorMsgBox.h"
#include "ui/MessageHandler.h"

#include "update/updateprocess.h"

#include "generate/behaviorprocess.h"

UpdateFilesStart* process1;
BehaviorStart* process2;
DummyLog* process3;

void interMsg(std::string input)
{
    if (process1) 
    { 
        process1->message(input);
    }
    else if (process2)
    {
        process2->message(input);
    }
    else if (process3)
    {
        process3->message(input);
    }
    else
    {
        DebugLogging("Non-captured message: " + input);
        CEMsgBox* msgbox = new CEMsgBox;
        msgbox->setWindowTitle("CRITICAL ERROR");
        msgbox->setText(QString("interMsg can not deliver message. Running process not found. Report to Nemesis' author immediately.\n"
                                "Message: ")
                        .append(QString::fromStdString(input)));
        msgbox->show();
        error = true;
    }
}

void interMsg(std::wstring input)
{
    if (process1)
    {
        process1->message(input);
    }
    else if (process2)
    {
        process2->message(input);
    }
    else if (process3)
    {
        process3->message(input);
    }
    else
    {
        DebugLogging(L"Non-captured message: " + input);
        CEMsgBox* msgbox = new CEMsgBox;
        msgbox->setWindowTitle("CRITICAL ERROR");
        msgbox->setText(QString("interMsg can not deliver message. Running process not found. Report to Nemesis' author immediately.\n"
                                "Message: ")
                        .append(QString::fromStdWString(input)));
        msgbox->show();
        error = true;
    }
}

void connectProcess(UpdateFilesStart* newProcess)
{
    process1 = newProcess;
}

void connectProcess(BehaviorStart* newProcess)
{
    process2 = newProcess;
}

void connectProcess(DummyLog* newProcess)
{
    process3 = newProcess;
}

void disconnectProcess()
{
    DebugLogging("Standard log disconnected");
    process1 = nullptr;
    process2 = nullptr;
    process3 = nullptr;
}

void disconnectProcess(DummyLog* newProcess)
{
    if(process3 != nullptr)
    {
        if(process3 != newProcess)
            DebugLogging("Warning: disconnectProcess for DummyLog that was not connected!");
        process3 = nullptr;
    }
    else DebugLogging("Error: disconnectProcess is trying to disconnect DummyLog but 'process3' ptr is already null!");
}
