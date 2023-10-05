#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BioSemi_Acquisition.h"
#include <thread>
#include <memory>
#include "LabStreamEEG.h"
#include <vector>
#include <QCloseEvent>

class BioSemi_Acquisition : public QMainWindow
{
    Q_OBJECT

public:
    BioSemi_Acquisition(QWidget *parent = nullptr);
    ~BioSemi_Acquisition();
    Ui::BioSemi_AcquisitionClass& GetUI() { return ui; }
    void LogText(const char* txt, bool asError = false, bool guiThread = true);

private:
    const char* separator = "----------------------------";
    inline static std::string saveLocation = "settings.json";
    Ui::BioSemi_AcquisitionClass ui;
    bool isStreaming = false;
    std::unique_ptr<LabStreamEEG> labStream;
    std::unique_ptr<std::thread> dataThread;
    std::vector<QWidget*> interactables;
    std::vector<QCheckBox*> exgCheckboxes;
private:
    void EnableInteraction(bool enable);
    void KillDataThread(bool join);
    void LoadToUI();
    void SaveFromUI();
private slots:
    void onStreamStart();
    void clearLogText();
protected:
    void closeEvent(QCloseEvent* event) override;

};
