#include "SystemMonitorForm.h"

#include <ApiUrl.h>
#include <MemStore.h>

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QScrollArea>
#include <QSet>
#include <QUrlQuery>
#include <QVBoxLayout>

#include <algorithm>
#include <limits>

SystemMonitorForm::SystemMonitorForm(QWidget *parent)
    : QWidget(parent)
    , presetCombo(nullptr)
    , startTimeEdit(nullptr)
    , endTimeEdit(nullptr)
    , loadPresetButton(nullptr)
    , loadRangeButton(nullptr)
    , statusLabel(nullptr)
    , cpuChart(nullptr)
    , cpuSeries(nullptr)
    , cpuAxisX(nullptr)
    , cpuAxisY(nullptr)
    , memoryChart(nullptr)
    , memorySeries(nullptr)
    , memoryAxisX(nullptr)
    , memoryAxisY(nullptr)
    , networkChart(nullptr)
    , uploadSeries(nullptr)
    , downloadSeries(nullptr)
    , networkAxisX(nullptr)
    , networkAxisY(nullptr)
    , systemLoadChart(nullptr)
    , systemLoadSeries(nullptr)
    , systemLoadAxisX(nullptr)
    , systemLoadAxisY(nullptr)
    , diskChart(nullptr)
    , diskAxisX(nullptr)
    , diskAxisY(nullptr)
{
    setupUi();
    fetchByPreset();
}

SystemMonitorForm::~SystemMonitorForm() {}

void SystemMonitorForm::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(10);

    auto *controlGroup = new QGroupBox(tr("监控查询"), this);
    auto *controlLayout = new QHBoxLayout(controlGroup);

    presetCombo = new QComboBox(controlGroup);
    presetCombo->addItem(tr("最近 5 分钟"), "5-min");
    presetCombo->addItem(tr("最近 15 分钟"), "15-min");
    presetCombo->addItem(tr("最近 30 分钟"), "30-min");
    presetCombo->addItem(tr("最近 1 小时"), "1-h");
    presetCombo->addItem(tr("最近 6 小时"), "6-h");
    presetCombo->addItem(tr("最近 12 小时"), "12-h");
    presetCombo->addItem(tr("最近 1 天"), "1-d");
    presetCombo->addItem(tr("最近 7 天"), "7-d");
    presetCombo->setCurrentIndex(3);

    loadPresetButton = new QPushButton(tr("查询预设"), controlGroup);

    startTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), controlGroup);
    endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), controlGroup);
    startTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    startTimeEdit->setCalendarPopup(true);
    endTimeEdit->setCalendarPopup(true);

    loadRangeButton = new QPushButton(tr("按时间范围查询"), controlGroup);

    statusLabel = new QLabel(tr("准备就绪"), controlGroup);
    statusLabel->setMinimumWidth(220);

    controlLayout->addWidget(new QLabel(tr("预设："), controlGroup));
    controlLayout->addWidget(presetCombo);
    controlLayout->addWidget(loadPresetButton);
    controlLayout->addSpacing(10);
    controlLayout->addWidget(new QLabel(tr("开始："), controlGroup));
    controlLayout->addWidget(startTimeEdit);
    controlLayout->addWidget(new QLabel(tr("结束："), controlGroup));
    controlLayout->addWidget(endTimeEdit);
    controlLayout->addWidget(loadRangeButton);
    controlLayout->addStretch();
    controlLayout->addWidget(statusLabel);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *chartsContainer = new QWidget(scrollArea);
    auto *chartsLayout = new QGridLayout(chartsContainer);
    chartsLayout->setContentsMargins(0, 0, 0, 0);
    chartsLayout->setHorizontalSpacing(10);
    chartsLayout->setVerticalSpacing(10);

    cpuChart = new QChart;
    memoryChart = new QChart;
    networkChart = new QChart;
    systemLoadChart = new QChart;
    diskChart = new QChart;

    createCharts();

    chartsLayout->addWidget(createChartCard(tr("CPU 使用率"), cpuChart), 0, 0);
    chartsLayout->addWidget(createChartCard(tr("内存使用率"), memoryChart), 0, 1);
    chartsLayout->addWidget(createChartCard(tr("网络速度"), networkChart), 1, 0);
    chartsLayout->addWidget(createChartCard(tr("系统负载"), systemLoadChart), 1, 1);
    chartsLayout->addWidget(createChartCard(tr("磁盘使用率"), diskChart), 2, 0, 1, 2);

    scrollArea->setWidget(chartsContainer);

    rootLayout->addWidget(controlGroup);
    rootLayout->addWidget(scrollArea, 1);

    connect(loadPresetButton, &QPushButton::clicked, this, &SystemMonitorForm::fetchByPreset);
    connect(loadRangeButton, &QPushButton::clicked, this, &SystemMonitorForm::fetchByRange);
}

QWidget *SystemMonitorForm::createChartCard(const QString &title, QChart *chart)
{
    auto *card = new QGroupBox(title, this);
    auto *layout = new QVBoxLayout(card);
    auto *chartView = new QChartView(chart, card);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(280);
    layout->addWidget(chartView);
    return card;
}

void SystemMonitorForm::createCharts()
{
    setupSingleLineChart(cpuChart,
                         tr("CPU 使用率"),
                         tr("CPU"),
                         "%",
                         cpuSeries,
                         cpuAxisX,
                         cpuAxisY,
                         0,
                         100);

    setupSingleLineChart(memoryChart,
                         tr("内存使用率"),
                         tr("内存"),
                         "%",
                         memorySeries,
                         memoryAxisX,
                         memoryAxisY,
                         0,
                         100);

    networkChart->removeAllSeries();
    networkChart->setTitle(tr("网络速度"));

    networkAxisY = new QValueAxis;
    networkAxisX = new QDateTimeAxis;
    networkAxisX->setFormat("HH:mm");
    networkAxisX->setTickCount(6);
    networkAxisY->setMin(0);
    networkAxisY->setTitleText("KB/s");

    networkChart->addAxis(networkAxisX, Qt::AlignBottom);
    networkChart->addAxis(networkAxisY, Qt::AlignLeft);

    uploadSeries = new QLineSeries;
    downloadSeries = new QLineSeries;
    uploadSeries->setName(tr("上传"));
    downloadSeries->setName(tr("下载"));

    networkChart->addSeries(uploadSeries);
    networkChart->addSeries(downloadSeries);
    uploadSeries->attachAxis(networkAxisX);
    uploadSeries->attachAxis(networkAxisY);
    downloadSeries->attachAxis(networkAxisX);
    downloadSeries->attachAxis(networkAxisY);
    networkChart->legend()->setVisible(true);

    setupSingleLineChart(systemLoadChart,
                         tr("系统负载"),
                         tr("系统负载"),
                         tr("Load"),
                         systemLoadSeries,
                         systemLoadAxisX,
                         systemLoadAxisY,
                         0,
                         10);

    diskChart->removeAllSeries();
    diskChart->setTitle(tr("磁盘使用率"));
    diskAxisX = new QDateTimeAxis;
    diskAxisY = new QValueAxis;
    diskAxisX->setFormat("HH:mm");
    diskAxisX->setTickCount(6);
    diskAxisY->setRange(0, 100);
    diskAxisY->setTitleText("%");
    diskChart->addAxis(diskAxisX, Qt::AlignBottom);
    diskChart->addAxis(diskAxisY, Qt::AlignLeft);
    diskChart->legend()->setVisible(true);
}

void SystemMonitorForm::setupSingleLineChart(QChart *chart,
                                             const QString &title,
                                             const QString &lineName,
                                             const QString &yTitle,
                                             QLineSeries *&series,
                                             QDateTimeAxis *&axisX,
                                             QValueAxis *&axisY,
                                             double yMin,
                                             double yMax)
{
    chart->removeAllSeries();
    chart->setTitle(title);

    series = new QLineSeries;
    series->setName(lineName);
    chart->addSeries(series);

    axisX = new QDateTimeAxis;
    axisX->setFormat("HH:mm");
    axisX->setTickCount(6);

    axisY = new QValueAxis;
    axisY->setRange(yMin, yMax);
    axisY->setTitleText(yTitle);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);
}

void SystemMonitorForm::fetchByPreset()
{
    const QString preset = presetCombo->currentData().toString();
    const QStringList parts = preset.split("-");
    if (parts.size() != 2) {
        QMessageBox::warning(this, tr("参数错误"), tr("预设参数解析失败"));
        return;
    }

    QMap<QString, QString> params;
    params.insert("number", parts[0]);
    params.insert("unit", parts[1]);
    requestedDurationMs = unitToDurationMs(parts[0].toInt(), parts[1]);

    fetchMetrics(NASMONITORRECENT, params);
}

void SystemMonitorForm::fetchByRange()
{
    if (startTimeEdit->dateTime() >= endTimeEdit->dateTime()) {
        QMessageBox::warning(this, tr("参数错误"), tr("开始时间必须小于结束时间"));
        return;
    }

    QMap<QString, QString> params;
    params.insert("startMills", QString::number(startTimeEdit->dateTime().toMSecsSinceEpoch()));
    params.insert("endMills", QString::number(endTimeEdit->dateTime().toMSecsSinceEpoch()));
    requestedDurationMs = endTimeEdit->dateTime().toMSecsSinceEpoch()
                          - startTimeEdit->dateTime().toMSecsSinceEpoch();

    fetchMetrics(NASMONITORBYRANGE, params);
}

void SystemMonitorForm::fetchMetrics(const QString &apiPath, const QMap<QString, QString> &params)
{
    statusLabel->setText(tr("加载中..."));
    loadPresetButton->setEnabled(false);
    loadRangeButton->setEnabled(false);

    QString fullApi = getFullApiPath(FULLHOST, apiPath);
    auto *request = new ApiRequest(fullApi, ApiRequest::GET, this);

    QUrl url(fullApi);
    QUrlQuery query;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.addQueryItem(it.key(), it.value());
    }
    url.setQuery(query);

    QUrlQuery encodedQuery(url);
    request->setUrlQuery(encodedQuery);

    connect(request, &ApiRequest::responseRecieved, this, [this, request](QString &rawContent, bool hasError, qint16 statusCode) {
        onMetricsResponse(rawContent, hasError, statusCode);
        request->deleteLater();
    });

    request->sendRequest();
}

void SystemMonitorForm::onMetricsResponse(QString &rawContent, bool hasError, qint16 statusCode)
{
    loadPresetButton->setEnabled(true);
    loadRangeButton->setEnabled(true);

    if (hasError || statusCode != 200) {
        statusLabel->setText(tr("加载失败"));
        QMessageBox::warning(this,
                             tr("请求失败"),
                             tr("监控数据请求失败，状态码：%1").arg(statusCode));
        return;
    }

    bool ok = false;
    const QList<MonitorMetricPoint> parsed = parseMetrics(rawContent, ok);
    if (!ok) {
        statusLabel->setText(tr("数据解析失败"));
        QMessageBox::warning(this, tr("数据错误"), tr("监控数据解析失败"));
        return;
    }

    metrics = parsed;
    statusLabel->setText(tr("共 %1 条记录").arg(metrics.size()));
    updateAllCharts();
}

QList<MonitorMetricPoint> SystemMonitorForm::parseMetrics(const QString &rawContent, bool &ok) const
{
    ok = false;
    QList<MonitorMetricPoint> result;

    const QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
    QJsonArray list;

    if (doc.isArray()) {
        list = doc.array();
    } else if (doc.isObject()) {
        const QJsonObject rootObj = doc.object();
        if (rootObj.contains("data") && rootObj.value("data").isArray()) {
            list = rootObj.value("data").toArray();
        } else {
            return result;
        }
    } else {
        return result;
    }

    for (const QJsonValue &value : list) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject obj = value.toObject();
        MonitorMetricPoint point;
        point.timestamp = obj.value("timestamp").toString();
        point.timestampMs = parseTimestampMs(point.timestamp);
        point.cpu = obj.value("cpu").toDouble();
        point.totalMemory = static_cast<qint64>(obj.value("totalMemory").toDouble());
        point.usedMemory = static_cast<qint64>(obj.value("usedMemory").toDouble());
        point.uploadSpeed = static_cast<qint64>(obj.value("uploadSpeed").toDouble());
        point.downloadSpeed = static_cast<qint64>(obj.value("downloadSpeed").toDouble());
        point.sysLoad = obj.value("sysLoad").toDouble();

        const QJsonArray diskList = obj.value("diskMetricsList").toArray();
        for (const QJsonValue &diskVal : diskList) {
            if (!diskVal.isObject()) {
                continue;
            }
            const QJsonObject diskObj = diskVal.toObject();
            const QString diskName = diskObj.value("diskName").toString();
            const double totalSpace = diskObj.value("totalSpace").toDouble();
            const double usedSpace = diskObj.value("usedSpace").toDouble();
            if (!diskName.isEmpty()) {
                point.diskUsagePercent.insert(diskName, safePercent(usedSpace, totalSpace));
            }
        }
        result.append(point);
    }

    ok = true;
    return result;
}

void SystemMonitorForm::updateAllCharts()
{
    updateTimeAxisFormat();
    updateCpuChart();
    updateMemoryChart();
    updateNetworkChart();
    updateSystemLoadChart();
    updateDiskChart();
}

void SystemMonitorForm::updateCpuChart()
{
    QList<QPointF> points;
    points.reserve(metrics.size());

    qint64 minTime = std::numeric_limits<qint64>::max();
    qint64 maxTime = std::numeric_limits<qint64>::min();

    for (int index = 0; index < metrics.size(); ++index) {
        const qint64 x = metrics[index].timestampMs > 0 ? metrics[index].timestampMs : index;
        points.append(QPointF(static_cast<qreal>(x), metrics[index].cpu));
        minTime = qMin(minTime, x);
        maxTime = qMax(maxTime, x);
    }

    cpuSeries->replace(points);
    if (!points.isEmpty()) {
        applyTimeRange(cpuAxisX, minTime, maxTime);
    }
}

void SystemMonitorForm::updateMemoryChart()
{
    QList<QPointF> points;
    points.reserve(metrics.size());

    qint64 minTime = std::numeric_limits<qint64>::max();
    qint64 maxTime = std::numeric_limits<qint64>::min();

    for (int index = 0; index < metrics.size(); ++index) {
        const auto &item = metrics[index];
        const qint64 x = item.timestampMs > 0 ? item.timestampMs : index;
        points.append(QPointF(static_cast<qreal>(x), safePercent(item.usedMemory, item.totalMemory)));
        minTime = qMin(minTime, x);
        maxTime = qMax(maxTime, x);
    }

    memorySeries->replace(points);
    if (!points.isEmpty()) {
        applyTimeRange(memoryAxisX, minTime, maxTime);
    }
}

void SystemMonitorForm::updateNetworkChart()
{
    QList<QPointF> uploadPoints;
    QList<QPointF> downloadPoints;
    uploadPoints.reserve(metrics.size());
    downloadPoints.reserve(metrics.size());

    qint64 minTime = std::numeric_limits<qint64>::max();
    qint64 maxTime = std::numeric_limits<qint64>::min();

    double maxSpeed = 1.0;
    for (int index = 0; index < metrics.size(); ++index) {
        const auto &item = metrics[index];
        const double upload = bytesToKB(item.uploadSpeed);
        const double download = bytesToKB(item.downloadSpeed);
        const qint64 x = item.timestampMs > 0 ? item.timestampMs : index;
        uploadPoints.append(QPointF(static_cast<qreal>(x), upload));
        downloadPoints.append(QPointF(static_cast<qreal>(x), download));
        maxSpeed = qMax(maxSpeed, qMax(upload, download));
        minTime = qMin(minTime, x);
        maxTime = qMax(maxTime, x);
    }

    uploadSeries->replace(uploadPoints);
    downloadSeries->replace(downloadPoints);

    if (!uploadPoints.isEmpty()) {
        applyTimeRange(networkAxisX, minTime, maxTime);
    }
    networkAxisY->setRange(0, maxSpeed * 1.1);
}

void SystemMonitorForm::updateSystemLoadChart()
{
    QList<QPointF> points;
    points.reserve(metrics.size());

    qint64 minTime = std::numeric_limits<qint64>::max();
    qint64 maxTime = std::numeric_limits<qint64>::min();

    double maxLoad = 1.0;
    for (int index = 0; index < metrics.size(); ++index) {
        const double load = metrics[index].sysLoad;
        const qint64 x = metrics[index].timestampMs > 0 ? metrics[index].timestampMs : index;
        points.append(QPointF(static_cast<qreal>(x), load));
        maxLoad = qMax(maxLoad, load);
        minTime = qMin(minTime, x);
        maxTime = qMax(maxTime, x);
    }

    systemLoadSeries->replace(points);
    if (!points.isEmpty()) {
        applyTimeRange(systemLoadAxisX, minTime, maxTime);
    }
    systemLoadAxisY->setRange(0, maxLoad * 1.2);
}

void SystemMonitorForm::updateDiskChart()
{
    QSet<QString> diskNames;
    for (const auto &item : metrics) {
        for (auto it = item.diskUsagePercent.constBegin(); it != item.diskUsagePercent.constEnd(); ++it) {
            diskNames.insert(it.key());
        }
    }

    for (auto it = diskSeriesMap.begin(); it != diskSeriesMap.end();) {
        if (!diskNames.contains(it.key())) {
            diskChart->removeSeries(it.value());
            delete it.value();
            it = diskSeriesMap.erase(it);
            continue;
        }
        ++it;
    }

    for (const QString &diskName : diskNames) {
        if (diskSeriesMap.contains(diskName)) {
            continue;
        }

        auto *series = new QLineSeries;
        series->setName(diskName);
        diskChart->addSeries(series);
        series->attachAxis(diskAxisX);
        series->attachAxis(diskAxisY);
        diskSeriesMap.insert(diskName, series);
    }

    for (auto it = diskSeriesMap.begin(); it != diskSeriesMap.end(); ++it) {
        QList<QPointF> points;
        points.reserve(metrics.size());

        for (int index = 0; index < metrics.size(); ++index) {
            const auto &item = metrics[index];
            double usage = 0;
            if (item.diskUsagePercent.contains(it.key())) {
                usage = item.diskUsagePercent.value(it.key());
            }
            const qint64 x = item.timestampMs > 0 ? item.timestampMs : index;
            points.append(QPointF(static_cast<qreal>(x), usage));
        }
        it.value()->replace(points);
    }

    if (!metrics.isEmpty()) {
        qint64 minTime = std::numeric_limits<qint64>::max();
        qint64 maxTime = std::numeric_limits<qint64>::min();
        for (int index = 0; index < metrics.size(); ++index) {
            const qint64 x = metrics[index].timestampMs > 0 ? metrics[index].timestampMs : index;
            minTime = qMin(minTime, x);
            maxTime = qMax(maxTime, x);
        }
        applyTimeRange(diskAxisX, minTime, maxTime);
    }
}

void SystemMonitorForm::updateTimeAxisFormat()
{
    if (metrics.isEmpty() && requestedDurationMs <= 0) {
        return;
    }

    qint64 minTime = std::numeric_limits<qint64>::max();
    qint64 maxTime = std::numeric_limits<qint64>::min();
    for (const auto &item : metrics) {
        const qint64 ts = item.timestampMs;
        if (ts <= 0) {
            continue;
        }
        minTime = qMin(minTime, ts);
        maxTime = qMax(maxTime, ts);
    }

    qint64 dataDurationMs = 0;
    if (!(minTime == std::numeric_limits<qint64>::max() || maxTime < minTime)) {
        dataDurationMs = maxTime - minTime;
    }

    const qint64 effectiveDurationMs = qMax(dataDurationMs, requestedDurationMs);
    if (effectiveDurationMs <= 0) {
        return;
    }

    const QString format = smartTimeAxisFormat(effectiveDurationMs);
    const int tickCount = qBound(3, metrics.size() <= 1 ? 3 : metrics.size(), 8);

    auto applyAxisStyle = [format, tickCount](QDateTimeAxis *axis) {
        if (!axis) {
            return;
        }
        axis->setFormat(format);
        axis->setTickCount(tickCount);
    };

    applyAxisStyle(cpuAxisX);
    applyAxisStyle(memoryAxisX);
    applyAxisStyle(networkAxisX);
    applyAxisStyle(systemLoadAxisX);
    applyAxisStyle(diskAxisX);
}

double SystemMonitorForm::safePercent(double used, double total)
{
    if (total <= 0.0) {
        return 0.0;
    }
    return used / total * 100.0;
}

double SystemMonitorForm::bytesToKB(double bytesValue)
{
    return bytesValue / 1024.0;
}

void SystemMonitorForm::applyTimeRange(QDateTimeAxis *axis, qint64 minTime, qint64 maxTime)
{
    if (!axis) {
        return;
    }

    if (maxTime <= minTime) {
        maxTime = minTime + 60 * 1000;
    }

    axis->setRange(QDateTime::fromMSecsSinceEpoch(minTime), QDateTime::fromMSecsSinceEpoch(maxTime));
}

qint64 SystemMonitorForm::parseTimestampMs(const QString &timestamp)
{
    QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODateWithMs);
    if (!dt.isValid()) {
        dt = QDateTime::fromString(timestamp, Qt::ISODate);
    }
    if (!dt.isValid()) {
        dt = QDateTime::fromString(timestamp, "yyyy-MM-dd HH:mm:ss");
    }
    if (!dt.isValid()) {
        dt = QDateTime::fromString(timestamp, "yyyy-MM-dd HH:mm:ss.zzz");
    }
    if (!dt.isValid()) {
        dt = QDateTime::fromString(timestamp, "yyyy-MM-ddTHH:mm:ss");
    }
    if (!dt.isValid()) {
        dt = QDateTime::fromString(timestamp, "yyyy-MM-ddTHH:mm:ss.zzz");
    }
    if (!dt.isValid()) {
        return 0;
    }
    return dt.toMSecsSinceEpoch();
}

QString SystemMonitorForm::smartTimeAxisFormat(qint64 durationMs)
{
    const qint64 dayMs = 24LL * 60LL * 60LL * 1000LL;
    if (durationMs <= dayMs) {
        return "HH:mm";
    }
    if (durationMs <= 7LL * dayMs) {
        return "MM-dd HH:mm";
    }
    if (durationMs <= 365LL * dayMs) {
        return "MM-dd";
    }
    return "yy-MM";
}

qint64 SystemMonitorForm::unitToDurationMs(int number, const QString &unit)
{
    if (number <= 0) {
        return 0;
    }

    const qint64 minuteMs = 60LL * 1000LL;
    const qint64 hourMs = 60LL * minuteMs;
    const qint64 dayMs = 24LL * hourMs;

    if (unit == "min") {
        return number * minuteMs;
    }
    if (unit == "h") {
        return number * hourMs;
    }
    if (unit == "d") {
        return number * dayMs;
    }
    if (unit == "m") {
        return number * 30LL * dayMs;
    }
    if (unit == "y") {
        return number * 365LL * dayMs;
    }
    return 0;
}
