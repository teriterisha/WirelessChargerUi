#include "curve_chart.h"

Curve_chart::Curve_chart(QWidget *parent)
    : QWidget{parent}
{
    maxSize = 10; // 只存储最新的 10 个数据
    maxX = 10;
    maxY = 25;
    dataNum = 0;
    createSeries();
    QHBoxLayout *layout = new QHBoxLayout();
    QChartView* chartView = new QChartView(createChart());
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
    setLayout(layout);
}
void Curve_chart::createSeries() {
    for(int i = 0; i < seriesSize; ++i) {
        splineSeries << new QSplineSeries();
    }
}

QChart* Curve_chart::createChart()
{
    chart = new QChart();
    for(int i = 0; i < seriesSize; ++i) {
        chart->addSeries(splineSeries[i]);
    }
//    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->axisX()->setRange(0, maxX);
    chart->axisY()->setRange(0, maxY);
    return chart;
}
void Curve_chart::setCharTitle(QString s) {
    chart->setTitle(s);
}

void Curve_chart::dataReceived(const QList<int> &value) {
    for(int i = 0; i < value.size(); ++i) {
        splineSeries[i]->append(dataNum, value.at(i));
    }
    if (dataNum > maxSize) {
        for(int i = 0; i < value.size(); ++i) {
            splineSeries[i]->remove(0);
            chart->axisX()->setRange(dataNum - maxSize, dataNum);
        }
    }
    ++dataNum;
}

void Curve_chart::setLineName(QString s, int i) {
    splineSeries[i]->setName(s);
}

void Curve_chart::clearAlldate() {
    dataNum = 0;
    for(int i = 0; i < seriesSize; ++i) {
        splineSeries[i]->clear();
    }
}
