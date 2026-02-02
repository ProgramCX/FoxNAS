#include "OverViewForm.h"
#include "ui_OverViewForm.h"

#include <QDebug>
#include <QMessageBox>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineSeries>
#include <QToolTip>
#include <QValueAxis>

#include "OverviewDiskItem.h"

OverViewForm::OverViewForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OverViewForm)
{
    ui->setupUi(this);
    socket = new QWebSocket;

    startGetResourceStatus();
    qDebug() << "正在连接WebSocket...\n";
}

OverViewForm::~OverViewForm()
{
    delete ui;
}

void OverViewForm::startGetResourceStatus()
{
    //测试
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    socket->setProxy(proxy);
    // connect(socket, &QWebSocket::connected, this, [] {
    //     QMessageBox::information(nullptr, "连接", "链接成功");
    // });
    connect(socket,
            &QWebSocket::textMessageReceived,
            this,
            &OverViewForm::onRecieveMessage,
            Qt::UniqueConnection);

    connect(socket,
            &QWebSocket::errorOccurred,
            this,
            &OverViewForm::handleError,
            Qt::UniqueConnection);

    connect(socket,
            &QWebSocket::disconnected,
            this,
            &OverViewForm::handleDisconnected,
            Qt::UniqueConnection);

    socket->open(QUrl(getFullWebsocketUrl()));
}

void OverViewForm::stopGetResourceStatus() {}

void OverViewForm::updateChart()
{
    if (!cpuSeries || !cpuChart || !netChart)
        return;

    updateCPUChart();
    updateMemoryChart();
    updateNetworkChart();

    updateDiskList();
}

void OverViewForm::updateCPUChart()
{
    if (cpuStatisticsQueue.length() >= 50) {
        cpuStatisticsQueue.pop_front();
    }
    cpuStatisticsQueue.enqueue(cpuUsage);

    QList<QPointF> cpuPoints;
    cpuPoints.reserve(cpuStatisticsQueue.size());
    int cpuX = 0;
    for (double yVal : cpuStatisticsQueue) {
        cpuPoints.append(QPointF(cpuX++, yVal));
    }
    cpuSeries->replace(cpuPoints);

    auto *cpuAxisX = qobject_cast<QValueAxis *>(cpuChart->axisX());
    if (cpuAxisX) {
        int maxX = qMax(49, cpuPoints.count() - 1);
        cpuAxisX->setRange(maxX - 49, maxX);
    }

    ui->labelCpuRate->setText(QString::number(cpuUsage, 'f', 2) + " %");
}

void OverViewForm::updateMemoryChart()
{
    if (memStatisticsQueue.length() >= 50) {
        memStatisticsQueue.pop_front();
    }
    QPair<double, QString> memTotal = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
        memoryInfo["total"]);
    auto *memAxisY = qobject_cast<QValueAxis *>(memChart->axisY());

    if (memAxisY) {
        memAxisY->setMax(memTotal.first);
        memAxisY->setTitleText(memTotal.second);
    }

    double memFree = BytesConvertorUtil::getInstance()
                         .getDataAccordingUnit(memoryInfo["total"] - memoryInfo["used"],
                                               memTotal.second);

    memStatisticsQueue.enqueue(memFree);

    QList<QPointF> points;
    points.reserve(memStatisticsQueue.size());
    int memX = 0;
    for (double yVal : memStatisticsQueue) {
        points.append(QPointF(memX++, yVal));
    }

    memSeries->replace(points);

    auto *memAxisX = qobject_cast<QValueAxis *>(memChart->axisX());
    if (memAxisX) {
        int maxX = qMax(49, points.count() - 1);
        memAxisX->setRange(maxX - 49, maxX);
    }

    ui->labelMemoryRate->setText(
        QString::number(memoryInfo["used"] / memoryInfo["total"] * 100, 'f', 2) + " %");

    QPair<double, QString> memUsed = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
        memoryInfo["used"]);
    ui->labelMemCondition->setText(QString("%1 %2 / %3 %4")
                                       .arg(QString::number(memUsed.first, 'f', 2),
                                            memUsed.second,
                                            QString::number(memTotal.first, 'f', 2),
                                            memTotal.second));
}
void OverViewForm::updateNetworkChart()
{
    double totalDownload = 0;
    double totalUpload = 0;
    double max = 0;
    double min = 0;
    if (netSentStatisticsQueue.length() >= 50) {
        netSentStatisticsQueue.pop_front();
    }

    if (netRecvStatisticsQueue.length() >= 50) {
        netRecvStatisticsQueue.pop_front();
    }

    QList<double> sentSpeedList;
    QList<double> recvSpeedList;

    int netCardCount = netList.size(); // 网卡数量

    // 初始化 sentPointsList 和 recvPointsList，确保每个都有 netCardCount 个 QList<QPointF>
    QList<QList<QPointF>> sentPointsList(netCardCount);
    QList<QList<QPointF>> recvPointsList(netCardCount);

    for (const QMap<QString, QString> &map : netList) {
        double sent = map["sentSpeed"].toDouble();
        double recv = map["recvSpeed"].toDouble();

        sentSpeedList.append(BytesConvertorUtil::getInstance().getDataAccordingUnit(sent, "KB"));
        recvSpeedList.append(BytesConvertorUtil::getInstance().getDataAccordingUnit(recv, "KB"));

        totalUpload += sent;
        totalDownload += recv;
    }

    netSentStatisticsQueue.enqueue(sentSpeedList);
    netRecvStatisticsQueue.enqueue(recvSpeedList);

    auto *netAxisX = qobject_cast<QValueAxis *>(netChart->axisX());

    // 组装折线图坐标点：横坐标为时间（队列下标），纵坐标为速度值
    for (int i = 0; i < netSentStatisticsQueue.length(); i++) {
        for (int j = 0; j < netSentStatisticsQueue[i].length(); j++) {
            sentPointsList[j].append(QPointF(i, netSentStatisticsQueue[i][j]));
            max = netSentStatisticsQueue[i][j] > max ? netSentStatisticsQueue[i][j] : max;
            min = netSentStatisticsQueue[i][j] < min ? netSentStatisticsQueue[i][j] : min;
        }
    }

    for (int i = 0; i < netRecvStatisticsQueue.length(); i++) {
        for (int j = 0; j < netRecvStatisticsQueue[i].length(); j++) {
            recvPointsList[j].append(QPointF(i, netRecvStatisticsQueue[i][j]));
            max = netRecvStatisticsQueue[i][j] > max ? netRecvStatisticsQueue[i][j] : max;
            min = netRecvStatisticsQueue[i][j] < min ? netRecvStatisticsQueue[i][j] : min;
        }
    }

    auto *netAxisY = qobject_cast<QValueAxis *>(netChart->axisY());
    netAxisY->setMax(max);
    netAxisY->setMin(min);
    // 更新图表
    for (int i = 0; i < sentPointsList.size(); i++) {
        netSentSerieses[i]->replace(sentPointsList[i]);
    }

    for (int i = 0; i < recvPointsList.size(); i++) {
        netRecieveSerieses[i]->replace(recvPointsList[i]);
    }

    // 滚动窗口 X 轴
    int maxX = qMax(49, netSentStatisticsQueue.size() - 1);
    netAxisX->setRange(maxX - 49, maxX);

    QPair<double, QString> totalDownloadSpeed = BytesConvertorUtil::getInstance()
                                                    .getReasonaleDataUnit(totalDownload);
    QPair<double, QString> totalUploadSpeed = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
        totalUpload);

    ui->labelDownloadSpeed->setText(QString("%1 %2/s")
                                        .arg(QString::number(totalDownloadSpeed.first, 'f', 2))
                                        .arg(totalDownloadSpeed.second));
    ui->labelUploadSpeed->setText(QString("%1 %2/s")
                                      .arg(QString::number(totalUploadSpeed.first, 'f', 2))
                                      .arg(totalUploadSpeed.second));
}

void OverViewForm::updateDiskList()
{
    double totalTotalSpace = 0;
    double totalTotalUsed = 0;

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QWidget *widget = ui->listWidget->itemWidget(item);

        if (widget) {
            delete widget;
        }

        delete item;
    }
    ui->listWidget->clear();

    for (QMap<QString, QString> &map : diskList) {
        OverviewDiskItem *overView = new OverviewDiskItem;

        QPair<double, QString> totalSpace = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
            map["total"].toDouble());
        QPair<double, QString> usedSpace = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
            map["used"].toDouble());

        totalTotalSpace += map["total"].toDouble();
        totalTotalUsed += map["used"].toDouble();

        overView->setDiskName(map["name"]);
        overView->setProgressbar(map["used"].toDouble() / map["total"].toDouble() * 100.0, 100);

        overView->setDiskCondition(
            QString("%1 %2").arg(QString::number(usedSpace.first, 'f', 2)).arg(usedSpace.second),
            QString("%1 %2").arg(QString::number(totalSpace.first, 'f', 2)).arg(totalSpace.second));

        QListWidgetItem *item = new QListWidgetItem;
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setSizeHint(overView->sizeHint());
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, overView);
    }

    QPair<double, QString> totalTotalSpacePair = BytesConvertorUtil::getInstance()
                                                     .getReasonaleDataUnit(totalTotalSpace);
    QPair<double, QString> totalUsedSpacePair = BytesConvertorUtil::getInstance()
                                                    .getReasonaleDataUnit(totalTotalUsed);

    ui->labelDiskRate->setText(
        QString("%1 %").arg(QString::number(totalTotalUsed / totalTotalSpace * 100.0, 'f', 2)));
    ui->labelCondition->setText(QString("%1 %2/%3 %4")
                                    .arg(QString::number(totalUsedSpacePair.first, 'f', 2))
                                    .arg(totalUsedSpacePair.second)
                                    .arg(QString::number(totalTotalSpacePair.first, 'f', 2))
                                    .arg(totalTotalSpacePair.second));
}

void OverViewForm::parseJSONString(const QString &jsonString)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "JSON 解析失败！";
        return;
    }

    QJsonObject root = doc.object();

    diskList.clear();
    netList.clear();

    // 读取磁盘信息
    QJsonArray diskArray = root["disk"].toArray();
    for (const QJsonValue &diskVal : diskArray) {
        QJsonObject disk = diskVal.toObject();
        QMap<QString, QString> map;
        map.insert("name", disk["name"].toString());
        map.insert("total", QString::number(disk["total"].toDouble(), 'f', 2));
        map.insert("used", QString::number(disk["used"].toDouble(), 'f', 2));
        map.insert("free", QString::number(disk["free"].toDouble(), 'f', 2));
        diskList.append(map);

        // qDebug() << "磁盘名称:" << disk["name"].toString();
        // qDebug() << "总大小:" << disk["total"].toDouble();
        // qDebug() << "已用:" << disk["used"].toDouble();
        // qDebug() << "可用:" << disk["free"].toDouble();
        // qDebug() << "------------------------";
    }

    // 读取内存信息
    QJsonObject memory = root["memory"].toObject();
    memoryInfo.insert("total", memory["total"].toDouble());
    memoryInfo.insert("used", memory["used"].toDouble());

    // qDebug() << "内存总量:" << memory["total"].toDouble();
    // qDebug() << "已用内存:" << memory["used"].toDouble();

    // 读取CPU使用率
    cpuUsage = root["cpu"].toDouble();
    // qDebug() << "CPU 使用率:" << cpuUsage;

    // 读取网络信息
    QJsonArray networkArray = root["network"].toArray();
    for (const QJsonValue &netVal : networkArray) {
        QJsonObject net = netVal.toObject();
        QMap<QString, QString> map;
        map.insert("name", net["name"].toString());
        map.insert("ipv4", net["ipv4"].toArray().first().toString());
        map.insert("recvSpeed", QString::number(net["recvSpeed"].toDouble(), 'f', 2));
        map.insert("sentSpeed", QString::number(net["sentSpeed"].toDouble(), 'f', 2));
        netList.append(map);
        // qDebug() << "origin:" << net["recvSpeed"].toString();
        // qDebug() << "网卡名称:" << net["name"].toString();
        // qDebug() << "IPv4 地址:" << net["ipv4"].toArray().first().toString();
        // qDebug() << "接收速率:" << net["recvSpeed"].toDouble();
        // qDebug() << "发送速率:" << net["sentSpeed"].toDouble();
        // qDebug() << "------------------------";
    }
}

void OverViewForm::onRecieveMessage(const QString &message)
{
    reconnectTime = 0;
    parseJSONString(message);
    if (!received) {
        received = true;
        createCharts();
        return;
    }
    updateChart();
}

void OverViewForm::createCharts()
{
    //CPU 图表
    createCPUChart();
    //内存图表
    createMemoryChart();
    //网络图表
    createNetworkChart();
}

void OverViewForm::createMemoryChart()
{
    memSeries = new QLineSeries;
    memSeries->setName(tr("剩余容量"));

    memChart = new QChart;
    memChart->addSeries(memSeries);
    memChart->setTitle(tr("内存剩余情况(GB)"));

    auto *memAxisX = new QValueAxis;
    memAxisX->setRange(0, 49);
    memAxisX->setLabelsVisible(false);
    memChart->addAxis(memAxisX, Qt::AlignBottom);
    memSeries->attachAxis(memAxisX);

    auto *memAxisY = new QValueAxis;
    // memAxisY->setRange(0, 100);
    memAxisY->setTitleText("GB");
    memChart->addAxis(memAxisY, Qt::AlignLeft);
    memSeries->attachAxis(memAxisY);

    memSeries->setUseOpenGL(false);

    QChartView *memChartView = new QChartView(memChart);
    memChartView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *memLayout = new QVBoxLayout(ui->memoryWidget);
    memLayout->addWidget(memChartView);

    connect(memSeries, &QLineSeries::hovered, this, [=](const QPointF &point, bool state) {
        if (state) {
            QToolTip::showText(QCursor::pos(),
                               QString("内存剩余: %1 %2")
                                   .arg(point.y(), 0, 'f', 2)
                                   .arg(BytesConvertorUtil::getInstance()
                                            .getReasonaleDataUnit(memoryInfo["total"])
                                            .second));
        } else {
            QToolTip::hideText();
        }
    });
}

void OverViewForm::createNetworkChart()
{
    netChart = new QChart;
    netChart->setTitle(tr("网络传输速率(KB)"));
    auto *netAxisX = new QValueAxis;
    netAxisX->setRange(0, 49);
    netAxisX->setLabelsVisible(false);
    netChart->addAxis(netAxisX, Qt::AlignBottom);

    auto *netAxisY = new QValueAxis;
    netAxisY->setTitleText("KB");
    netChart->addAxis(netAxisY, Qt::AlignLeft);

    for (const QMap<QString, QString> &map : netList) {
        QLineSeries *sentSeries = new QLineSeries;
        sentSeries->setName(QString("%1 (%2-%3)").arg(tr("上传速度"), map["name"], map["ipv4"]));
        netChart->addSeries(sentSeries);
        sentSeries->attachAxis(netAxisX);
        sentSeries->attachAxis(netAxisY);

        QLineSeries *receiveSeries = new QLineSeries;
        receiveSeries->setName(QString("%1 (%2-%3)").arg(tr("下载速度"), map["name"], map["ipv4"]));
        netChart->addSeries(receiveSeries);
        receiveSeries->attachAxis(netAxisX);
        receiveSeries->attachAxis(netAxisY);

        connect(sentSeries, &QLineSeries::hovered, this, [=](const QPointF &point, bool state) {
            if (state) {
                QToolTip::showText(QCursor::pos(),
                                   QString(tr("上传速度: %1 KB/s")).arg(point.y(), 0, 'f', 2));
            } else {
                QToolTip::hideText();
            }
        });

        connect(receiveSeries, &QLineSeries::hovered, this, [=](const QPointF &point, bool state) {
            if (state) {
                QToolTip::showText(QCursor::pos(),
                                   QString(tr("下载速度: %1 KB/s")).arg(point.y(), 0, 'f', 2));
            } else {
                QToolTip::hideText();
            }
        });

        netSentSerieses.append(sentSeries);
        netRecieveSerieses.append(receiveSeries);

        QLegend *legend = netChart->legend();
        legend->setVisible(true);
        legend->setAlignment(Qt::AlignRight);
    }

    QChartView *netChartView = new QChartView(netChart);
    netChartView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *netLayout = new QVBoxLayout(ui->networkWidget);
    netLayout->addWidget(netChartView);
}

void OverViewForm::handleError()
{
    disconnect(socket, &QWebSocket::textMessageReceived, this, &OverViewForm::onRecieveMessage);
    disconnect(socket, &QWebSocket::errorOccurred, this, &OverViewForm::handleError);
    disconnect(socket, &QWebSocket::disconnected, this, &OverViewForm::handleDisconnected);

    reconnectTime++;
    qDebug() << "连接失败:" << socket->errorString();

    if (reconnectTime >= 10) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("重连失败"),
                                      tr("十次重连失败！是否继续重连？"),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            reconnectTime = 0;
            qDebug() << "继续重连...";
            startGetResourceStatus();
        } else {
            QMessageBox::critical(this, tr("失败"), tr("请检查你的网络！"), tr("确定"));
            disconnect(socket,
                       &QWebSocket::textMessageReceived,
                       this,
                       &OverViewForm::onRecieveMessage);
            disconnect(socket, &QWebSocket::errorOccurred, this, &OverViewForm::handleError);
            disconnect(socket, &QWebSocket::disconnected, this, &OverViewForm::handleDisconnected);
        }
    } else {
        qDebug() << "正在进行第" << reconnectTime << "次重连...";
        startGetResourceStatus();
    }
}

void OverViewForm::handleDisconnected()
{
    handleError();
}

void OverViewForm::createCPUChart()
{
    cpuSeries = new QLineSeries;
    cpuSeries->setName(tr("CPU使用率"));

    cpuChart = new QChart;
    cpuChart->addSeries(cpuSeries);
    cpuChart->setTitle(tr("CPU使用率(%)"));

    auto *cpuAxisX = new QValueAxis;
    cpuAxisX->setRange(0, 49);
    cpuAxisX->setLabelsVisible(false);
    cpuChart->addAxis(cpuAxisX, Qt::AlignBottom);
    cpuSeries->attachAxis(cpuAxisX);

    auto *cpuAxisY = new QValueAxis;
    cpuAxisY->setRange(0, 100);
    cpuAxisY->setTitleText("%");
    cpuChart->addAxis(cpuAxisY, Qt::AlignLeft);
    cpuSeries->attachAxis(cpuAxisY);

    cpuSeries->setUseOpenGL(false);

    QChartView *cpuChartView = new QChartView(cpuChart);
    cpuChartView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *cpuLayout = new QVBoxLayout(ui->CPUWidget);
    cpuLayout->addWidget(cpuChartView);

    connect(cpuSeries, &QLineSeries::hovered, this, [=](const QPointF &point, bool state) {
        if (state) {
            QToolTip::showText(QCursor::pos(),
                               QString(tr("CPU使用率: %1%")).arg(point.y(), 0, 'f', 2));
        } else {
            QToolTip::hideText();
        }
    });
}

QString OverViewForm::getFullWebsocketUrl()
{
    const QString prefix = FULLHOST.contains(":443") ? "wss://" : "ws://";
    QString fullUrl = prefix + FULLHOST + NASRESOURCESTATUSSOCKET + "?token=" + NASTOKEN;
    qDebug() << fullUrl;
    return fullUrl;
}

