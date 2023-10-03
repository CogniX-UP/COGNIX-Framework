#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BioSemi_Acquisition.h"
#include <thread>
#include <memory>
#include "LabStreamEEG.h"
#include <vector>
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
    bool isStreaming = false;
    std::unique_ptr<LabStreamEEG> labStream;
    std::unique_ptr<std::thread> dataThread;
    std::vector<QWidget*> interactables;
private:
    void EnableInteraction(bool enable);
    void KillDataThread(bool join);
private slots:
    void onStreamStart();

};
