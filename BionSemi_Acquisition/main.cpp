#include "BioSemi_Acquisition.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BioSemi_Acquisition w;
    w.show();
    return a.exec();
}
