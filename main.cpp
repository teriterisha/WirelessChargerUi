#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
//    w.setAutoFillBackground(true);
//    w.setPalette(QPalette(QColor(0,63,165)));
    w.show();
    return a.exec();
}
