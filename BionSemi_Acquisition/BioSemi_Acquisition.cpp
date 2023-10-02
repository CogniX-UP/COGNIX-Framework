#include "BioSemi_Acquisition.h"
#include "LabStreamEEG.h"
#include <stdio.h>
#include <string.h>

BioSemi_Acquisition::BioSemi_Acquisition(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.startStreamButton, SIGNAL(clicked()), SLOT(onStreamStart()));
}

BioSemi_Acquisition::~BioSemi_Acquisition()
{
    if (dataThread.joinable())
        dataThread.join();
}

void BioSemi_Acquisition::onStreamStart() {
    

    if (!dataThread.joinable()) {
        dataThread = std::thread([&] {
            if (!isStreaming) {
                try {
                    LabStreamEEG lab;
                    auto& bioInterface = lab.GetBiosemiInterface();
                    bioInterface.SetLogCallback([&](const std::string &log) {
                        LogText(log.c_str());
                    });

                    bioInterface.ConnectAmplifier();

                    isStreaming = true;
                }
                catch (const std::exception& e) {
                    std::string result = std::string(e.what()) + '\n';
                    LogText(result.c_str());
                }
            }
            else {
                ui.logText->insertPlainText("NNOOOOO");
                isStreaming = false;
            }
            });
    }
    else {

    }
}

//Invoke the log in a gui thread or not
void BioSemi_Acquisition::LogText(const char* text, bool guiThread) {

    if (guiThread) {
        QMetaObject::invokeMethod(this, [=] {
            ui.logText->insertPlainText(text);
            ui.logText->insertPlainText("\n");
        });
    }
    else {
        ui.logText->insertPlainText(text);
        ui.logText->insertPlainText("\n");
    }
}
