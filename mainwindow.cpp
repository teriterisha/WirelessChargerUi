#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timerId = startTimer(1000);

    ui->label_zjupic->setScaledContents(true);

    ui->curve_chart1->setCharTitle("三相电压");
    connect(ui->pushButton, &QPushButton::clicked, [=](){
        saveFile = QFileDialog::getOpenFileName(this, tr("selected"), "F:/Data", tr("txt文件(*.txt)"));
    });

    //检测系统现有串口，显示到comboBox控件
    foreach (const QSerialPortInfo &qspinfo, QSerialPortInfo::availablePorts())
    {
        myPort.setPort(qspinfo);
        if(myPort.open(QIODevice::ReadWrite))
        {
            ui->portCombox->addItem(qspinfo.portName());
            myPort.close();
        }
    }
    currentCom = ui->portCombox->currentText();
    connect(ui->portCombox,static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated),this,&MainWindow::comchange);
    ui->stop_recieve_btn->setEnabled(false);
    //设置波特率
    QStringList baudList;
    baudList <<"1200"<<"2400"<<"4800"<<"9600"<<"14400"<<"19200"<<"38400"<<"56000"<<"57600"<<"115200";
    ui->baud_rate_select->addItems(baudList);
    ui->baud_rate_select->setCurrentIndex(3);
    currentBaud = ui->baud_rate_select->currentText();

    connect(ui->baud_rate_select,static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated),this,&MainWindow::baudchange);

    connect(&myPort, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(ui->start_recieve_btn, &QPushButton::clicked, [=](){
        ui->start_recieve_btn->setEnabled(false);
        ui->stop_recieve_btn->setEnabled(true);
        ui->portCombox->setEnabled(false);
        ui->baud_rate_select->setEnabled(false);
        myPort.setBaudRate(currentBaud.toInt());
        myPort.setPortName(currentCom);
        myPort.setDataBits(QSerialPort::Data8);
        myPort.setParity(QSerialPort::NoParity);
        myPort.setFlowControl(QSerialPort::NoFlowControl);
        if(myPort.open(QIODevice::ReadWrite)==false)
        {
            qDebug() <<"提示, 串口打开失败！";
            return;
        }
    });
    connect(ui->stop_recieve_btn, &QPushButton::clicked, [=](){
        ui->start_recieve_btn->setEnabled(true);
        ui->stop_recieve_btn->setEnabled(false);
        ui->portCombox->setEnabled(true);
        ui->baud_rate_select->setEnabled(true);
        myPort.close();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (event->timerId() == timerId)
    {
        QDateTime CurrentTime = QDateTime::currentDateTime();
        QString strTime=CurrentTime.toString(" yyyy年MM月dd日 hh:mm:ss \n");
        ui->label_time->setText(strTime);

        char*  ch;
        QByteArray ba = strTime.toLocal8Bit();  // toLocal8Bit 支持中文
        ch=ba.data();
//        startSaveData(saveFile, ch);
    }
}

void MainWindow::startSaveData(QString fileName, char* saveData){
    QFile f(fileName);
    f.open(QIODevice::Append);
    f.write(saveData);
    f.close();
}

void MainWindow::comchange(const QString& selected)
{
    qDebug()<<selected;
    currentCom = selected;
    ui->curve_chart1->clearAlldate();
    ui->curve_chart1_2->clearAlldate();
    ui->curve_chart1_3->clearAlldate();
    ui->curve_chart1_4->clearAlldate();
    ui->curve_chart1_5->clearAlldate();
    ui->curve_chart1_6->clearAlldate();
}

void MainWindow::baudchange(const QString& selected)
{
    qDebug()<<selected;
    currentBaud = selected;
}

void MainWindow::readData() {
    //读取串口收到的数据
    QByteArray buffer = myPort.readAll();
    QString receive = QString(buffer);
    QStringList  strs=  receive.split(" ");
    QList<QList<int>> tem;
    //在接受窗口显示收到的数据
    for(int i = 1; i < strs.size(); i = i + 3){
        QList<int> tem1;
        for(int j = 0; j < 3; ++j){
            tem1 << strs[i + j].toInt();
        }
        tem << tem1;
    }
    ui->curve_chart1->dataReceived(tem[0]);
    ui->curve_chart1_2->dataReceived(tem[1]);
    ui->curve_chart1_3->dataReceived(tem[2]);
    ui->curve_chart1_4->dataReceived(tem[3]);
    ui->curve_chart1_5->dataReceived(tem[4]);
    ui->curve_chart1_6->dataReceived(tem[5]);
}
