#include "FileTranferListItem.h"
#include "ui_FileTranferListItem.h"

#include <QDir>
#include <QFileDialog>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrlQuery>

#include <ApiUrl.h>
#include <BytesConvertorUtil.h>

#include <MemStore.h>
#include <QHttpMultiPart>
#include <QMessageBox>

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

        connect(netWorkReply,
                &QNetworkReply::metaDataChanged,
                this,
                [=]() mutable {
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

                    const QString savePath = QDir(destinationPath).filePath(fileName);

                    const QString baseName = QFileInfo(fileName).completeBaseName();
                    const QString suffix = QFileInfo(fileName).suffix();
                    QString finalName = fileName;
                    int index = 1;
                    while (QFile::exists(QDir(destinationPath).filePath(finalName))
                           && !downloadStart) {
                        finalName = QString("%1-%2.%3").arg(baseName).arg(index++).arg(suffix);
                    }

                    file = new QFile(QDir(destinationPath).filePath(finalName));

                    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
                        qDebug() << "无法创建文件：" << savePath;
                        netWorkReply->abort();
                        return;
                    }

                    ui->pushButtonCancel->setEnabled(true);
                    ui->pushButtonContinue->setEnabled(false);
                    ui->pushButtonPause->setEnabled(true);
                    ui->pushButtonRetry->setEnabled(false);

                    downloadStart = true;
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
            if (currentState != PAUSED && currentState != CANCELED) {
                int statusCode = netWorkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                                     .toInt();
                if (netWorkReply->error() || (statusCode != 200 && statusCode != 206)) {
                    dealWithError();
                } else if (currentState == TRANSFERING) {
                    currentState = FINISHED;
                    emit transferCompleted(this);
                }

            } else if (currentState == PAUSED) {
                emit transferPaused(this);
                if (file != nullptr)
                    existingBytes = file->size();
            } else if (currentState == CANCELED) {
                emit transferCanceled(this);
            }

            if (file != nullptr) {
                file->flush();
                file->close();
                file->deleteLater();
                file = nullptr;
            }
            netWorkReply->deleteLater();
            netWorkReply = nullptr;
        });

        iniSlotConnected = true;


    }else
    {
        if (destinationPath.isEmpty())
        {

           ui->labelMessage->setText("目标路径不能为空！");
            ui->pushButtonCancel->setEnabled(true);
            ui->pushButtonContinue->setEnabled(false);
            ui->pushButtonPause->setEnabled(false);
            ui->pushButtonRetry->setEnabled(false);
            currentState = FAILED;
            emit transferFailed(this);
            return;
        }
        ui->labelMessage->setText(tr("正在上传文件:"));

        ui->labelSourceFile->setText(QString("本地文件路径：%1").arg(sourceFilePath));
        ui->labelDestinationFile->setText(QString("远程保存位置：%2").arg(destinationPath));

        //打开文件
        auto *file = new QFile(sourceFilePath);
        if (!file->open(QIODevice::ReadOnly)) {
            ui->labelMessage->setText("无法打开源文件！");
            ui->pushButtonCancel->setEnabled(true);
            ui->pushButtonContinue->setEnabled(false);
            ui->pushButtonPause->setEnabled(false);
            ui->pushButtonRetry->setEnabled(false);
            currentState = FAILED;
            emit transferFailed(this);
            return;
        }

        // 创建多部分表单数据
        auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant(QString(R"(form-data; name="file"; filename="%1")")
                                        .arg(QFileInfo(sourceFilePath).fileName())));
        file->setParent(this);
        filePart.setBodyDevice(file);
        multiPart->append(filePart);

        //设置认证Bear
        netWorkRequest.setRawHeader("Authorization", QString("Bearer " + NASTOKEN).toUtf8());

        QUrlQuery query;
        query.addQueryItem("path", QUrl::toPercentEncoding(destinationPath));

        QUrl url = QUrl(getFullApiPath(FULLHOST, NASUPLOADFILEAPI));
        url.setQuery(query);
        netWorkRequest.setUrl(url);

        netWorkReply = netWorkAccessManager.post(netWorkRequest, multiPart);

        multiPart->setParent(this);

        connect(netWorkReply,&QNetworkReply::uploadProgress, this, &FileTranferListItem::handleUploadProgress);

        connect(netWorkReply, &QNetworkReply::finished, this, [=]() {
            if (currentState != CANCELED) {
                int statusCode = netWorkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                                     .toInt();
                if (netWorkReply->error() || (statusCode != 200 && statusCode != 201)) {
                    dealWithError();
                } else if (currentState == TRANSFERING) {
                    currentState = FINISHED;
                    emit transferCompleted(this);
                }
            }  else if (currentState == CANCELED) {
                emit transferCanceled(this);
            }

            file->close();
            file->deleteLater();
            netWorkReply->deleteLater();
            netWorkReply = nullptr;
        });

        ui->labelMessage->setText(tr("正在上传文件"));
        ui->pushButtonCancel->setEnabled(true);
        ui->pushButtonContinue->setVisible(false);
        ui->pushButtonPause->setVisible(false);
        ui->pushButtonRetry->setEnabled(false);

    }
    timer.start();
    currentState = TRANSFERING;
}

void FileTranferListItem::setMessageText(const QString &message) const
{
    ui->labelMessage->setText(message);
}

void FileTranferListItem::dealWithError()
{
    ui->labelMessage->setText("传输失败！");

    ui->pushButtonCancel->setEnabled(true);
    ui->pushButtonContinue->setEnabled(false);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonRetry->setEnabled(true);

    currentState = FAILED;

    emit transferFailed(this);
}

FileTranferListItem::TransferState FileTranferListItem::getCurrentState() const
{
    return currentState;
}

void FileTranferListItem::pauseTransfer()
{
    currentState = PAUSED;

    netWorkReply->abort();

    ui->labelMessage->setText(tr("已暂停"));

    ui->pushButtonCancel->setEnabled(true);
    ui->pushButtonContinue->setEnabled(true);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonRetry->setEnabled(false);
}

void FileTranferListItem::continueTransfer()
{
    ui->pushButtonContinue->setEnabled(false);
    setMessageText(tr("正在准备传输"));
    emit transferTryingContinue(this);
    // startTransfer(true);
}

void FileTranferListItem::cancelTransfer()
{
    currentState = CANCELED;
    if (netWorkReply != nullptr) {
        netWorkReply->abort();
    } else {
        emit transferCanceled(this);
    }
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
                .arg(speed.first > 0 ? speed.first : 0.0, 0, 'f', 2)
                .arg(speed.second));

        lastBytesReceived = received;
        timer.restart();
    }
}

void FileTranferListItem::handleUploadProgress(qint64 bytesSent, qint64 total)
{
    qint64 deltaBytes = bytesSent - lastBytesReceived;
    qint64 elapsedMs = timer.elapsed();

    if (elapsedMs > 500)
    {
        double speedKBps = deltaBytes / (elapsedMs / 1000.0);

        QPair<double, QString> speed = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
            speedKBps);

        QPair<double, QString> sentPair = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
            bytesSent);

        QPair<double, QString> totalPair = BytesConvertorUtil::getInstance().getReasonaleDataUnit(
            total);

        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(bytesSent * 100.0 / total);

        ui->labelProgress->setText(QString("%1%2 / %3%4")
                                       .arg(sentPair.first, 0, 'f', 2)
                                       .arg(sentPair.second)
                                       .arg(totalPair.first, 0, 'f', 2)
                                       .arg(totalPair.second));
        ui->labelSpeed->setText(QString(tr("上传速度：%1%2 /s"))
                                    .arg(speed.first > 0 ? speed.first : 0.0, 0, 'f', 2)
                                    .arg(speed.second));

        lastBytesReceived = bytesSent;
        timer.restart();
    }
}

void FileTranferListItem::on_pushButtonPause_clicked()
{
    pauseTransfer();
}

void FileTranferListItem::on_pushButtonRetry_clicked()
{
    startTransfer(false);
}

void FileTranferListItem::on_pushButtonCancel_clicked()
{
    cancelTransfer();
}

void FileTranferListItem::on_pushButtonContinue_clicked()
{
    continueTransfer();
}


