#ifndef OVERVIEWFORM_H
#define OVERVIEWFORM_H

#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QQueue>
#include <QWidget>
#include <QtWebSockets/QtWebSockets>

namespace Ui {
class OverViewForm;
}

class OverViewForm : public QWidget
{
    Q_OBJECT

public:
    explicit OverViewForm(QWidget *parent = nullptr);
    ~OverViewForm();

private:
    Ui::OverViewForm *ui;
    QWebSocket *socket;
    QList<QMap<QString, QString>> diskList;
    QList<QMap<QString, QString>> netList;
    QMap<QString, double> memoryInfo;
    double cpuUsage;

    QChart *cpuChart;
    QLineSeries *cpuSeries;
    QQueue<double> cpuStatisticsQueue;

    QChart *memChart;
    QLineSeries *memSeries;
    QQueue<double> memStatisticsQueue;

    QChart *netChart;
    QList<QLineSeries *> netSentSerieses;
    QList<QLineSeries *> netRecieveSerieses;
    QQueue<QList<double>> netSentStatisticsQueue;
    QQueue<QList<double>> netRecvStatisticsQueue;

    bool received = false;

    qint16 reconnectTime = 0;

private:
    void startGetResourceStatus();
    void stopGetResourceStatus();

    void updateChart();
    void updateCPUChart();
    void updateMemoryChart();
    void updateNetworkChart();
    void updateDiskList();

    void parseJSONString(const QString &jsonString);
    void onRecieveMessage(const QString &message);

    void createCharts();
    void createCPUChart();
    void createMemoryChart();
    void extracted();
    void createNetworkChart();

    void handleError();

    void handleDisconnected();
    QString getFullWebsocketUrl();
    QPair<double, QString> getReasonaleDataUnit(double bytes);
    double getDataAccordingUnit(double bytes, QString unit);
};

#endif // OVERVIEWFORM_H
