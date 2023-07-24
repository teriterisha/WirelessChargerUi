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

    ui->pushButton->hide();
    // 标题
    ui->curve_chart1->setCharTitle("发送电压");
    ui->curve_chart2->setCharTitle("发送电流");
    ui->curve_chart3->setCharTitle("发送功率");
    ui->curve_chart4->setCharTitle("接收电压");
    ui->curve_chart5->setCharTitle("接收电流");
    ui->curve_chart6->setCharTitle("接收功率");
    ui->curve_chart7->setCharTitle("效率");
    // 图Y轴幅值
    ui->curve_chart1->setY(150.0, -150.0);
    ui->curve_chart4->setY(150.0, -150.0);

    ui->curve_chart2->setY(10.0, -10.0);
    ui->curve_chart5->setY(10.0, -10.0);

    ui->curve_chart3->setY(500.0, -500.0);
    ui->curve_chart6->setY(500.0, -500.0);

    ui->curve_chart7->setY(1.0, -1.0);

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

    connect(&rectask, &UartRecC::successSignal,this, &MainWindow::display);
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
    ui->curve_chart2->clearAlldate();
    ui->curve_chart3->clearAlldate();
    ui->curve_chart4->clearAlldate();
    ui->curve_chart5->clearAlldate();
    ui->curve_chart6->clearAlldate();
    ui->curve_chart7->clearAlldate();
}

void MainWindow::baudchange(const QString& selected)
{
    qDebug()<<selected;
    currentBaud = selected;
}

void MainWindow::readData() {
    //读取串口收到的数据
    QByteArray buffer = myPort.readAll();
    for(int i = 0; i < buffer.size(); ++i) {
        rectask.last_byte = buffer.at(i);
        rectask.uart_frame_seg();
    }
}

void MainWindow::display(const QList<short>& all_data) {
    qDebug() << all_data;
    int n = all_data.size();
    if(n == 8 && all_data[7] == 0xF1) {
        QList<float> temp[7];
        for(int i = 0; i < 7; ++i) {
            temp[i].append(all_data[i] / 100.0);
        }
        ui->curve_chart1->dataReceived(temp[0]);
        ui->curve_chart2->dataReceived(temp[1]);
        ui->curve_chart3->dataReceived(temp[2]);
        ui->curve_chart4->dataReceived(temp[3]);
        ui->curve_chart5->dataReceived(temp[4]);
        ui->curve_chart6->dataReceived(temp[5]);
        ui->curve_chart7->dataReceived(temp[6]);
    }
}
