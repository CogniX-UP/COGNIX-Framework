#include "BioSemi_Acquisition.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <thread>
#include <chrono>
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
    for (auto p : interactables)
        exgCheckboxes.push_back(dynamic_cast<QCheckBox*>(p));

    interactables.push_back(ui.startStreamButton);
    interactables.push_back(ui.subsetChoice);
    interactables.push_back(ui.sendIntervalEdit);
    interactables.push_back(ui.streamNameEdit);

    try {
        labStream->Load(saveLocation, true);
    }
    catch (const std::exception &e) {
        LogText(e.what(), true, false);
    }

    LoadToUI();

    //End setting the UI
    //Set the log messages
    labStream->GetBiosemiInterface().SetLogCallback([this](const std::string& log, BiosemiEEG::LogType logType) {
        LogText(log.c_str(), false, true);
    });
}

BioSemi_Acquisition::~BioSemi_Acquisition()
{
    if (dataThread)
        dataThread->join();
}

void BioSemi_Acquisition::onStreamStart() {

    auto enableLambda = [this] { EnableInteraction(true); };
    if (!dataThread) {
        //Save the settings
        try {
            SaveFromUI();
        }
        catch (const std::exception& e) {
            LogText(e.what(), true, false);
        }
        //Create the thread here
        EnableInteraction(false);
        auto thread = new std::thread([&] {
            try {
                labStream->ConnectDevice();
                //Append the rest of the data here after connecting to the device
                //Connecting ensures that we have acquired metadata from the device itself, e.g sample rate

                //Start the stream
                labStream->StartStream();
                QMetaObject::invokeMethod(this, [this] {
                    ui.startStreamButton->setText("Stop");
                    ui.startStreamButton->setEnabled(true);
                });

                isStreaming = true;
                auto& streamSetting = labStream->GetStreamSetting();
                //Send the data here
                while (isStreaming) {
                    auto time = std::chrono::milliseconds(streamSetting.interval);
                    std::this_thread::sleep_for(time);
                    LogText("XD", false, true);
                    BiosemiEEG::Chunk chunk;
                    labStream->SendData(chunk);

                    LogText(std::to_string(chunk.size()).c_str(), false, true);
                }

                labStream->StopStream();
                labStream->DisconnectDevice();

                QMetaObject::invokeMethod(this, enableLambda, Qt::QueuedConnection);
            }
            catch (const std::exception &e) {
                labStream->StopStream();
                labStream->DisconnectDevice();

                isStreaming = false;
                
                QMetaObject::invokeMethod(this, [this, e] {
                    //Kill the data thread in the main thread
                    KillDataThread(true);
                    EnableInteraction(true);
                    std::ostringstream error;
                    error << "<font color =\"red\">" << e.what() << "</font>";
                    LogText(error.str().c_str(), false, true);
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
void BioSemi_Acquisition::LogText(const char* text, bool asError, bool guiThread) {
    auto logText = ui.logText;
    if (guiThread) 
    {
        //We're creating a qstring and passing it to the lambda, which copies it.
        //This is necessary because the lambda runs after a small amount of time in the
        //main GUI thread, and the original string would most likely be deleted

        std::string str(text);
        QMetaObject::invokeMethod(logText, [=] {
            logText->unsetCursor();
            QString result;
            if (asError) {
                std::ostringstream error;
                error << "<font color =\"red\">" << str << "</font>";
                result = QString(error.str().c_str());
            }
            else {
                result = QString(str.c_str());
            }
                
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

void BioSemi_Acquisition::LoadToUI() {
    auto& streamSetting = labStream->GetStreamSetting();

    auto& exgs = streamSetting.GetExgs();

    for (int i = 0; i < exgs.size(); i++) {
        exgCheckboxes[i]->setChecked(exgs[i]);
    }

    ui.streamNameEdit->setText(streamSetting.StreamName().c_str());
    ui.sendIntervalEdit->setText(std::to_string(streamSetting.interval).c_str());

    switch (streamSetting.channelCount) {
    case 32:
        ui.subsetChoice->setCurrentIndex(1);
        break;
    case 64:
        ui.subsetChoice->setCurrentIndex(2);
        break;
    case 128:
        ui.subsetChoice->setCurrentIndex(3);
        break;
    case 256:
        ui.subsetChoice->setCurrentIndex(4);
        break;
    default:
        ui.subsetChoice->setCurrentIndex(0);
        break;
    }
}

void BioSemi_Acquisition::SaveFromUI() {
    auto& streamSetting = labStream->GetStreamSetting();
    auto& exgs = streamSetting.GetExgs();
    streamSetting.SetStreamName(ui.streamNameEdit->toPlainText().toStdString());
    streamSetting.interval = std::stoi(ui.sendIntervalEdit->toPlainText().toStdString());
    for (int i = 0; i < exgs.size(); i++) {
        exgs[i] = exgCheckboxes[i]->checkState();
    }

    switch (ui.subsetChoice->currentIndex()) {
    case 0:
        streamSetting.channelCount = 0;
        break;
    case 1:
        streamSetting.channelCount = 32;
        break;
    case 2:
        streamSetting.channelCount = 64;
        break;
    case 3:
        streamSetting.channelCount = 128;
        break;
    case 4:
        streamSetting.channelCount = 256;
        break;
    }

    try {
        labStream->Save(saveLocation);
    }
    catch (const std::exception& e) {
        throw e;
    }
}
