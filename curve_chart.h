#ifndef CURVE_CHART_H
#define CURVE_CHART_H

#include <QWidget>

#include <QWidget>
#include <QList>

#include <QSplineSeries>
#include <QScatterSeries>
#include <QChart>
#include <QChartView>
#include <QHBoxLayout>

using namespace QtCharts;

class Curve_chart : public QWidget
{
    Q_OBJECT
public:
    explicit Curve_chart(QWidget *parent = nullptr);
    void dataReceived(const QList<float> &value);
    void setCharTitle(QString s);
    void setLineName(QString s, int i);
    void clearAlldate();
    void setY(float maxY, float minY);

protected:
    QChart* createChart();
    void createSeries();

private:
    int seriesSize = 1;//定义曲线个数最大为3
    int maxSize = 10;
    float maxX = 10;
    float maxY = 25;
    float minY = -25;
    double dataNum;
    QHBoxLayout *layout;
    QList<QSplineSeries*> splineSeries;
    QChart* chart;

signals:

};

#endif // CURVE_CHART_H
