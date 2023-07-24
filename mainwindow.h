#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include<QLabel>
#include <QSerialPort>        //Qt串口功能接口类
#include <QSerialPortInfo> //提供设备现有串行端口的信息

#include "uart.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void timerEvent(QTimerEvent *event);
    int timerId;
    QString currentCom;
    QString currentBaud;

    UartRecC rectask;
    QString saveFile = "F:/Data/1.txt";
    void startSaveData(QString fileName, char* saveData);
    QSerialPort myPort;

    void comchange(const QString& selected);
    void baudchange(const QString& selected);
    void readData();
    void display(const QList<short>& all_data);
};
#endif // MAINWINDOW_H
