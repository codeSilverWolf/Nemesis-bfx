#include "Global.h"

#include "ui/ErrorMsgBox.h"
#include "ui/MultiInstanceCheck.h"
#include <iostream>

#include <QSharedMemory>
#include <qsharedmemory.h>


bool isProgramAlreadyRunning(QObject *parent);

bool programInitiateCheck(QObject *parent)
{
    if (isProgramAlreadyRunning(parent))
    {
        CEMsgBox* errorMsg = new CEMsgBox;
        errorMsg->setWindowTitle("Nemesis Unlimited Behavior Engine");
        errorMsg->setText("An instance of Nemesis is already running");
        errorMsg->show();
        return false;
    }

    return true;
}

bool isProgramAlreadyRunning(QObject *parent)
{
    QString appName = "Nemesis-Ultimate=Behavior+Engine";
    QSharedMemory *m_sm = new QSharedMemory(appName,parent);

    assert(m_sm);

    if(!m_sm->create(1024) && m_sm->error() == QSharedMemory::AlreadyExists)
    {
        std::cerr << "QSharedMemory exists - another instance running!!!\n";
        return true;
    }

    return false;
}
