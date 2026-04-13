#ifndef SYSTEMMONITORFORM_H
#define SYSTEMMONITORFORM_H

#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDateTimeAxis>
#include <QDateTimeEdit>
#include <QLineSeries>
#include <QMap>
#include <QPushButton>
#include <QValueAxis>
#include <QWidget>

#include <ApiRequest.h>

class QGridLayout;
class QLabel;

struct MonitorMetricPoint
{
    QString timestamp;
    qint64 timestampMs = 0;
    double cpu = 0.0;
    qint64 totalMemory = 0;
    qint64 usedMemory = 0;
    qint64 uploadSpeed = 0;
    qint64 downloadSpeed = 0;
    double sysLoad = 0.0;
    QMap<QString, double> diskUsagePercent;
};

class SystemMonitorForm : public QWidget
{
    Q_OBJECT

public:
    explicit SystemMonitorForm(QWidget *parent = nullptr);
    ~SystemMonitorForm();

private:
    QComboBox *presetCombo;
    QDateTimeEdit *startTimeEdit;
    QDateTimeEdit *endTimeEdit;
    QPushButton *loadPresetButton;
    QPushButton *loadRangeButton;

    QLabel *statusLabel;

    QChart *cpuChart;
    QLineSeries *cpuSeries;
    QDateTimeAxis *cpuAxisX;
    QValueAxis *cpuAxisY;

    QChart *memoryChart;
    QLineSeries *memorySeries;
    QDateTimeAxis *memoryAxisX;
    QValueAxis *memoryAxisY;

    QChart *networkChart;
    QLineSeries *uploadSeries;
    QLineSeries *downloadSeries;
    QDateTimeAxis *networkAxisX;
    QValueAxis *networkAxisY;

    QChart *systemLoadChart;
    QLineSeries *systemLoadSeries;
    QDateTimeAxis *systemLoadAxisX;
    QValueAxis *systemLoadAxisY;

    QChart *diskChart;
    QDateTimeAxis *diskAxisX;
    QValueAxis *diskAxisY;
    QMap<QString, QLineSeries *> diskSeriesMap;

    QList<MonitorMetricPoint> metrics;
    qint64 requestedDurationMs = 0;

private:
    void setupUi();
    QWidget *createChartCard(const QString &title, QChart *chart);
    void createCharts();

    void setupSingleLineChart(QChart *chart,
                              const QString &title,
                              const QString &lineName,
                              const QString &yTitle,
                              QLineSeries *&series,
                              QDateTimeAxis *&axisX,
                              QValueAxis *&axisY,
                              double yMin,
                              double yMax);

    void fetchByPreset();
    void fetchByRange();
    void fetchMetrics(const QString &apiPath, const QMap<QString, QString> &params);
    void onMetricsResponse(QString &rawContent, bool hasError, qint16 statusCode);

    QList<MonitorMetricPoint> parseMetrics(const QString &rawContent, bool &ok) const;
    void updateAllCharts();
    void updateCpuChart();
    void updateMemoryChart();
    void updateNetworkChart();
    void updateSystemLoadChart();
    void updateDiskChart();
    void updateTimeAxisFormat();
    static void applyTimeRange(QDateTimeAxis *axis, qint64 minTime, qint64 maxTime);
    static qint64 unitToDurationMs(int number, const QString &unit);

    static double safePercent(double used, double total);
    static double bytesToKB(double bytesValue);
    static qint64 parseTimestampMs(const QString &timestamp);
    static QString smartTimeAxisFormat(qint64 durationMs);
};

#endif // SYSTEMMONITORFORM_H
