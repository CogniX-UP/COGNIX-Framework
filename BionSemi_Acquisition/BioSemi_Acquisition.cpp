#include "BioSemi_Acquisition.h"
#include <stdio.h>
#include <string.h>
#include <sstream>

BioSemi_Acquisition::BioSemi_Acquisition(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.startStreamButton, SIGNAL(clicked()), SLOT(onStreamStart()));
    labStream = std::make_unique<LabStreamEEG>();

    interactables.push_back(ui.ex1);
    interactables.push_back(ui.ex2);
    interactables.push_back(ui.ex3);
    interactables.push_back(ui.ex4);
    interactables.push_back(ui.ex5);
    interactables.push_back(ui.ex6);
    interactables.push_back(ui.ex7);
    interactables.push_back(ui.ex8);
    interactables.push_back(ui.startStreamButton);
    interactables.push_back(ui.subsetChoice);

    //Set the log messages
    labStream->GetBiosemiInterface().SetLogCallback([this](const std::string& log, BiosemiEEG::LogType logType) {
        LogText(log.c_str(), true);
    });
}

BioSemi_Acquisition::~BioSemi_Acquisition()
{
    if (dataThread)
        dataThread->join();
}

void BioSemi_Acquisition::onStreamStart() {

    auto enableLambda = [this] {EnableInteraction(true); };
    if (!dataThread) {
        //Create the thread here
        EnableInteraction(false);
        auto thread = new std::thread([&] {
            try {
                labStream->StartStream();
                QMetaObject::invokeMethod(this, [this] {
                    ui.startStreamButton->setText("Stop");
                    ui.startStreamButton->setEnabled(true);
                });

                isStreaming = true;
                while (isStreaming) {
                
                }

                labStream->StopStream();
                QMetaObject::invokeMethod(this, enableLambda, Qt::QueuedConnection);
            }
            catch (const std::exception &e) {
                labStream->StopStream();
                isStreaming = false;
                
                QMetaObject::invokeMethod(this, [this, e] {
                    //Kill the data thread in the main thread
                    KillDataThread(true);
                    EnableInteraction(true);
                    std::ostringstream error;
                    error << "<font color =\"red\">" << e.what() << "</font>";
                    LogText(error.str().c_str());
                }, Qt::QueuedConnection);
                
            }
        });

        dataThread.reset(thread);
    }
    else 
    {
        isStreaming = false;
        KillDataThread(true);
        ui.startStreamButton->setText("Start");
    }
}

//Invoke the log in a gui thread or not
void BioSemi_Acquisition::LogText(const char* text, bool guiThread) {
    auto logText = ui.logText;
    if (guiThread) 
    {
        //We're creating a qstring and passing it to the lambda, which copies it.
        //This is necessary because the lambda runs after a small amount of time in the
        //main GUI thread, and the original string would most likely be deleted      
        QMetaObject::invokeMethod(logText, [result = QString(text), logText] {
            logText->unsetCursor();
            logText->append(result);
        }, Qt::QueuedConnection);
    }
    else 
    {;
        logText->append(text);
    }
}

void BioSemi_Acquisition::EnableInteraction(bool enable) {
    for (auto inter : interactables) {
        inter->setEnabled(enable);
    }
}

void BioSemi_Acquisition::KillDataThread(bool join) {
    if (join && dataThread->joinable())
        dataThread->join();

    auto dPoint = dataThread.release();
    if (dPoint && dPoint->joinable())
        delete dPoint;
    ui.startStreamButton->setText("Start");
}
