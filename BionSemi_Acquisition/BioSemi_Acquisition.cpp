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
{}

void BioSemi_Acquisition::onStreamStart() {
    try {
        LabStreamEEG lab;
        lab.SendData();
    }
    catch(const std::exception &e){
        std::string result = std::string(e.what()) + '\n';
        ui.logText->insertPlainText(result.c_str());
    }
}