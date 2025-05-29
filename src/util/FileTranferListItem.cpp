#include "FileTranferListItem.h"
#include "ui_FileTranferListItem.h"

#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrlQuery.h>
#include <ApiUrl.h>
#include <BytesConvertorUtil.h>

#include <MemStore.h>

FileTranferListItem::FileTranferListItem(QString sourceFilePath,
                                         QString destinationPath,
                                         FileTranferListItem::TransferType type,
                                         QWidget *parent)
    : QWidget(parent)
    , sourceFilePath(sourceFilePath)
    , destinationPath(destinationPath)
    , transferType(type)
    , ui(new Ui::FileTranferListItem)
{
    ui->setupUi(this);
    ui->labelSourceFile->setText(QString("远程路径：%1").arg(sourceFilePath));
    ui->labelDestinationFile->setText(QString("本地保存位置：%2").arg(destinationPath));
}

FileTranferListItem::~FileTranferListItem()
{
    delete ui;
}

qint64 FileTranferListItem::getTotalBytes() const
{
    return totalBytes;
}

void FileTranferListItem::setTotalBytes(qint64 newTotalBytes)
{
    totalBytes = newTotalBytes;
}

qint64 FileTranferListItem::getTransferedBytes() const
{
    return transferedBytes;
}

void FileTranferListItem::setTransferedBytes(qint64 newTransferedBytes)
{
    transferedBytes = newTransferedBytes;
}

FileTranferListItem::TransferType FileTranferListItem::getTransferType() const
{
    return transferType;
}

void FileTranferListItem::setTransferType(TransferType newTransferType)
{
    transferType = newTransferType;
}

QString FileTranferListItem::getSourceFilePath() const
{
    return sourceFilePath;
}

QString FileTranferListItem::getDestinationPath() const
{
    return destinationPath;
}

void FileTranferListItem::startTransfer(bool isContinue)
{
    ui->pushButtonCancel->setEnabled(true);
    ui->pushButtonPause->setEnabled(true);

    if (transferType == TransferType::DOWNLOAD) {
        if (!QDir(destinationPath).exists()) {
            QDir dir;
            if (!dir.mkpath(destinationPath)) {
                ui->labelMessage->setText("传输失败！");
                emit transferFailed(this);
            }
        }

        ui->labelMessage->setText(tr("正在下载文件:"));

        ui->labelSourceFile->setText(QString("远程路径：%1").arg(sourceFilePath));
        ui->labelDestinationFile->setText(QString("本地保存位置：%2").arg(destinationPath));

        QString fullAPiPath = getFullApiPath(FULLHOST, NASDOWNLOADFILEAPI);
        QUrl url(fullAPiPath);

        QUrlQuery query;
        QByteArray encodedPath = QUrl::toPercentEncoding(sourceFilePath);
        query.addQueryItem("path", encodedPath);
        url.setQuery(query);

        netWorkRequest.setUrl(url);

        netWorkRequest.setRawHeader("Authorization", QString("Bearer " + NASTOKEN).toUtf8());
        if (isContinue) {
            netWorkRequest.setRawHeader("Range", "bytes=" + QByteArray::number(existingBytes) + "-");
        }
        netWorkReply = netWorkAccessManager.get(netWorkRequest);

        if (!iniSlotConnected) {
            connect(netWorkReply, &QNetworkReply::metaDataChanged, this, [=]() mutable {
                if (!file) {
                    QString contentDisposition = netWorkReply->rawHeader("Content-Disposition");
                    QRegularExpression re(
                        "filename\\*?=[^']*'(?:[^']*)'([^\";]+)|filename=\"?([^\";]+)\"?");
                    QRegularExpressionMatch match = re.match(contentDisposition);

                    QString fileName;
                    if (match.hasMatch()) {
                        fileName = match.captured(1).isEmpty() ? match.captured(2)
                                                               : match.captured(1);
                        fileName = QUrl::fromPercentEncoding(fileName.toUtf8());
                    } else {
                        // 从 URL 获取
                        fileName = QFileInfo(QUrlQuery(netWorkReply->url()).queryItemValue("path"))
                                       .fileName();
                    }

                    if (fileName.isEmpty())
                        fileName = "default_download.txt";

                    QString savePath = QDir(destinationPath).filePath(fileName);

                    QString baseName = QFileInfo(fileName).completeBaseName();
                    QString suffix = QFileInfo(fileName).suffix();
                    QString finalName = fileName;
                    int index = 1;
                    while (QFile::exists(QDir(destinationPath).filePath(finalName))) {
                        finalName = QString("%1-%2.%3").arg(baseName).arg(index++).arg(suffix);
                    }

                    file = new QFile(QDir(destinationPath).filePath(finalName));

                    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
                        qDebug() << "无法创建文件：" << savePath;
                        netWorkReply->abort();
                        return;
                    }
                }
            });

            connect(netWorkReply, &QNetworkReply::readyRead, this, [=]() {
                if (file && file->isOpen()) {
                    file->write(netWorkReply->readAll());
                }
            });

            connect(netWorkReply,
                    &QNetworkReply::downloadProgress,
                    this,
                    &FileTranferListItem::handleDownloadProgress);
            connect(netWorkReply, &QNetworkReply::finished, this, [=]() {
                if (!isPause) {
                    emit transferCompleted(this);
                } else {
                    if (file)
                        existingBytes = file->size();
                }

                int statusCode = netWorkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                                     .toInt();
                if (netWorkReply->error() || (statusCode != 200 && statusCode != 206)) {
                    qDebug() << "状态码：" << statusCode;
                    ui->labelMessage->setText("传输失败！");
                    emit transferFailed(this);
                }

                if (file) {
                    file->flush();
                    file->close();
                    file->deleteLater();
                }
                netWorkReply->deleteLater();
            });

            iniSlotConnected = true;
        }

        timer.start();
    }
}

void FileTranferListItem::setMessageText(const QString &message) const
{
    ui->labelMessage->setText(message);
}

void FileTranferListItem::continueTransfer()
{
    isPause = true; //表示再次启动是因为之前暂停
    startTransfer(true);
}

void FileTranferListItem::cancelTransfer()
{
    netWorkReply->abort();
}

void FileTranferListItem::handleDownloadProgress(qint64 received, qint64 total)
{
    qint64 deltaBytes = received - lastBytesReceived;
    qint64 elapsedMs = timer.elapsed();

    if (elapsedMs > 500) { // 每0.5秒计算一次
        double speedKBps = deltaBytes / (elapsedMs / 1000.0);

        QPair<double, QString> speed = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
            speedKBps);

        QPair<double, QString> recievedPair = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
            received + existingBytes);

        QPair<double, QString> totalPair = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
            total + existingBytes);

        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue((received + existingBytes) * 100.0 / (total + existingBytes));

        ui->labelProgress->setText(QString("%1%2 / %3%4")
                                       .arg(recievedPair.first, 0, 'f', 2)
                                       .arg(recievedPair.second)
                                       .arg(totalPair.first, 0, 'f', 2)
                                       .arg(totalPair.second));
        ui->labelSpeed->setText(
            QString(tr("%1速度：%2%3 /s"))
                .arg(transferType == TransferType::DOWNLOAD ? tr("下载") : tr("上传"))
                .arg(speed.first, 0, 'f', 2)
                .arg(speed.second));

        lastBytesReceived = received;
        timer.restart();
    }
}
