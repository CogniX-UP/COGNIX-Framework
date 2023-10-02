#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BioSemi_Acquisition.h"
#include <thread>

class BioSemi_Acquisition : public QMainWindow
{
    Q_OBJECT

public:
    BioSemi_Acquisition(QWidget *parent = nullptr);
    ~BioSemi_Acquisition();
    Ui::BioSemi_AcquisitionClass& GetUI() { return ui; }
    void LogText(const char* txt, bool guiThread = true);

private:
    Ui::BioSemi_AcquisitionClass ui;

    std::thread dataThread;
    bool isStreaming = false;

private slots:
    void onStreamStart();

};
