#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BioSemi_Acquisition.h"

class BioSemi_Acquisition : public QMainWindow
{
    Q_OBJECT

public:
    BioSemi_Acquisition(QWidget *parent = nullptr);
    ~BioSemi_Acquisition();
    Ui::BioSemi_AcquisitionClass& GetUI() { return ui; }

private:
    Ui::BioSemi_AcquisitionClass ui;

private slots:
    void onStreamStart();
};
