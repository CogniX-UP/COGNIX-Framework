#pragma once
#include <QThread>

class DataThread : public QThread
{
    Q_OBJECT
        void run() override = 0;
signals:
    void resultReady(const QString& s);
    
};