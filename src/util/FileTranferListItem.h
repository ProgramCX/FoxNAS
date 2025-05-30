#ifndef FILETRANFERLISTITEM_H
#define FILETRANFERLISTITEM_H

#include <QWidget>

#include <QElapsedTimer>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace Ui {
class FileTranferListItem;
}

class FileTranferListItem : public QWidget
{
    Q_OBJECT

public:
    enum TransferType { UPLOAD, DOWNLOAD };
    enum TransferState { PENDING, TRANSFERING, FAILED, PAUSED, CANCELED, FINISHED };
    explicit FileTranferListItem(QString sourceFilePath,
                                 QString destinationPath,
                                 FileTranferListItem::TransferType type,
                                 QWidget *parent = nullptr);
    ~FileTranferListItem();

public:
    void setSourceFilePath(QString path);
    void setDestinationPath(QString path);

    qint64 getTotalBytes() const;
    void setTotalBytes(qint64 newTotalBytes);

    qint64 getTransferedBytes() const;
    void setTransferedBytes(qint64 newTransferedBytes);

    TransferType getTransferType() const;
    void setTransferType(TransferType newTransferType);

    QString getSourceFilePath() const;

    QString getDestinationPath() const;

    void startTransfer(bool isContinue = false);

    void setMessageText(const QString &message) const;

    TransferState getCurrentState() const;

    bool downloadStart = 0;

    void dealWithError();

private:
    Ui::FileTranferListItem *ui;
    QString sourceFilePath;
    QString destinationPath;

    qint64 totalBytes = 0;
    qint64 transferedBytes = 0;

    bool iniSlotConnected = false;

    TransferType transferType = TransferType::DOWNLOAD;
    TransferState currentState = TransferState::PENDING;

    QNetworkAccessManager netWorkAccessManager;

    QNetworkReply *netWorkReply;
    QNetworkRequest netWorkRequest;

    qint64 lastBytesReceived = 0;
    qint64 existingBytes = 0;
    QElapsedTimer timer;

    QFile *file = nullptr;

private:
    void pauseTransfer();
    void continueTransfer();
    void cancelTransfer();

    void handleDownloadProgress(qint64 received, qint64 total);
    void handleUploadProgress(qint64 received, qint64 total);

signals:
    void transferCompleted(FileTranferListItem *item);
    void transferFailed(FileTranferListItem *item);
    void transferCanceled(FileTranferListItem *item);
    void transferPaused(FileTranferListItem *item);
    void transferTryingContinue(FileTranferListItem *item);
private slots:
    void on_pushButtonPause_clicked();
    void on_pushButtonRetry_clicked();
    void on_pushButtonCancel_clicked();
    void on_pushButtonContinue_clicked();

};

#endif // FILETRANFERLISTITEM_H
