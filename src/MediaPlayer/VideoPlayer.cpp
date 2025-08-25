#include "VideoPlayer.h"
#include <QAudioOutput>
#include <QUrlQuery>
#include <ApiRequest.h>
#include <ApiUrl.h>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMediaMetaData>
#include <QMessageBox>

#include "VideoWidget.h"

#include "ui_VideoPlayer.h"
#include <MemStore.h>

VideoPlayer::VideoPlayer(QString filePath, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoPlayer)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    ui->setupUi(this);
    player = new QMediaPlayer;
    videoWidget = new VideoPlayerWidget(this);
    audioOutput = new QAudioOutput;

    uiHideTimer = new QTimer(this);
    uiHideTimer->setInterval(5000);
    uiHideTimer->setSingleShot(true);

    heartBeatsTimer = new QTimer(this);
    heartBeatsTimer->setInterval(10000);

    connect(uiHideTimer, &QTimer::timeout, this, [=]() { ui->frame->setVisible(0); });
    connect(heartBeatsTimer, &QTimer::timeout, this, &VideoPlayer::sendHeartBeats);

    ui->toolButtonBackward->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    ui->toolButtonForward->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->toolButtonEnd->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

    QVBoxLayout *layout = new QVBoxLayout(ui->videoFrame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(videoWidget);

    player->setAudioOutput(audioOutput);
    player->setVideoOutput(videoWidget);

    this->filePath = filePath;

    connect(player, &QMediaPlayer::positionChanged, this, [=](qint64 position) {
        ui->labelCurrent->setText(formatDuration(position / 1000));
        ui->horizontalSlider->setValue(position / 1000);
    });
    connect(player,
            &QMediaPlayer::errorOccurred,
            this,
            [=](QMediaPlayer::Error error, const QString &errorString) {
                retryCount++;
                if (retryCount == 5) {
                    QMessageBox msgBox;
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setWindowTitle("播放错误");
                    msgBox.setText("播放失败！");
                    msgBox.setInformativeText(errorString);
                    msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
                    msgBox.setDefaultButton(QMessageBox::Retry);

                    int ret = msgBox.exec();
                    if (ret == QMessageBox::Retry) {
                        loadMedia(filePath);
                    } else {
                        close();
                    }
                }
                loadMedia(filePath);
            });
    QObject::connect(player,
                     &QMediaPlayer::mediaStatusChanged,
                     [this](QMediaPlayer::MediaStatus status) {
                         if (status == QMediaPlayer::LoadedMedia) {
                             if (!mediaReady) {
                                 mediaReady = true;
                                 iniMediaMeta();
                             }
                         }

                         if (status == QMediaPlayer::InvalidMedia) {
                             retryCount++;
                             if (retryCount == 5) {
                                 QMessageBox msgBox;
                                 msgBox.setIcon(QMessageBox::Warning);
                                 msgBox.setWindowTitle("播放错误");
                                 msgBox.setText("播放失败！");
                                 msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
                                 msgBox.setDefaultButton(QMessageBox::Retry);

                                 int ret = msgBox.exec();
                                 if (ret == QMessageBox::Retry) {
                                     loadMedia(this->filePath);
                                 } else {
                                     close();
                                 }
                             }
                             loadMedia(this->filePath);
                         }
                     });
    connect(player,
            &QMediaPlayer::playbackStateChanged,
            this,
            [=](QMediaPlayer::PlaybackState state) {
                if (state == QMediaPlayer::PlayingState) {
                    changeButtonStatePlaying();
                    retryCount = 0;
                } else if (state == QMediaPlayer::PausedState) {
                    changeButtonStatePause();
                } else {
                    changeButtonStateStop();
                }
            });

    ui->verticalLayout_2->setContentsMargins(0, 0, 0, 0);
    // 确保 Frame 无边框
    ui->frame->setFrameShape(QFrame::NoFrame);
    ui->videoFrame->setFrameShape(QFrame::NoFrame);
    ui->videoFrame->setMouseTracking(true);
    videoWidget->setAttribute(Qt::WA_OpaquePaintEvent, true);
    loadMedia(filePath);
}
void VideoPlayer::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}
VideoPlayer::~VideoPlayer()
{
    delete ui;
    delete player;
    delete audioOutput;
    delete videoWidget;
}

void VideoPlayer::showControls()
{
    uiHideTimer->start();
    ui->frame->setVisible(true);
}

void VideoPlayer::loadMedia(QString filePath)
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASVideoToken),
                                            ApiRequest::GET);
    apiRequest->addQueryParam("path", filePath);
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (!hasError && statusCode == 200) {
                    this->token = rawContent;

                    QUrl url(getFullApiPath(FULLHOST, NASVideoRangeStream));
                    QUrlQuery query;
                    query.addQueryItem("path", filePath);
                    query.addQueryItem("token", this->token);
                    url.setQuery(query);
                    player->setSource(url);
                    player->play();
                    heartBeatsTimer->start();
                }

                apiRequest->deleteLater();
            });

    apiRequest->sendRequest();

    ui->videoFrame->setMouseTracking(true);
    setMouseTracking(true);
}

void VideoPlayer::iniMediaMeta()
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASVideoMetaData),
                                            ApiRequest::GET);
    apiRequest->addQueryParam("path", filePath);
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (!hasError && statusCode == 200) {
                    QJsonParseError err;
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8(), &err);

                    QJsonObject obj = doc.object();

                    qDebug() << rawContent;
                    title = obj["title"].toString();
                    duration = obj["durationSeconds"].toInt();
                    videoCount = obj["videoTrackCount"].toInt();
                    audioCount = obj["audioTrackCount"].toInt();

                    QJsonArray tracks = obj["tracks"].toArray();
                    for (auto trackVal : tracks) {
                        QJsonObject track = trackVal.toObject();
                        int index = track["index"].toInt();
                        QString type = track["type"].toString();
                        QString ttitle = track["title"].toString();

                        if (type == "audio") {
                            audioList.append(ttitle.isEmpty()
                                                 ? tr("音频轨道 %1").arg(audioList.size())
                                                 : ttitle);
                        } else if (type == "video") {
                            videoList.append(ttitle.isEmpty()
                                                 ? tr("视频轨道 %1").arg(videoList.size())
                                                 : ttitle);
                        } else if (type == "ass") {
                            assList.append(ttitle.isEmpty() ? tr("字幕轨道 %1").arg(assList.size())
                                                            : ttitle);
                        }
                    }

                    ui->comboBoxSoundTrack->clear();
                    ui->comboBoxVideoTrack->clear();
                    ui->comboBoxAssTrack->clear();

                    ui->comboBoxSoundTrack->addItems(audioList);
                    ui->comboBoxVideoTrack->addItems(videoList);
                    ui->comboBoxAssTrack->addItems(assList);

                    this->setWindowTitle(filePath);
                    ui->labelTotal->setText(formatDuration(duration));
                    ui->labelCurrent->setText(formatDuration(0));

                    if (audioList.size() > 0) {
                        player->setActiveAudioTrack(0);
                        ui->comboBoxSoundTrack->setCurrentIndex(0);
                    }
                    if (videoList.size() > 0) {
                        player->setActiveVideoTrack(0);
                        ui->comboBoxVideoTrack->setCurrentIndex(0);
                    }
                    if (assList.size() > 0) {
                        player->setActiveSubtitleTrack(0);
                        ui->comboBoxAssTrack->setCurrentIndex(0);
                    }

                    ui->horizontalSlider->setValue(0);
                    ui->horizontalSlider->setMaximum(duration);
                }
                apiRequest->deleteLater();
            });

    apiRequest->sendRequest();
}

QString VideoPlayer::formatDuration(int durationSeconds)
{
    int ms = durationSeconds * 1000;
    QTime t = QTime::fromMSecsSinceStartOfDay(ms);

    if (durationSeconds >= 3600) {
        // 1 小时以上显示 00:00:00
        return t.toString("hh:mm:ss");
    } else {
        // 否则显示 00:00
        return t.toString("mm:ss");
    }
}

void VideoPlayer::changeButtonStateStop()
{
    ui->toolButtonStartPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void VideoPlayer::changeButtonStatePause()
{
    ui->toolButtonStartPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void VideoPlayer::changeButtonStatePlaying()
{
    ui->toolButtonStartPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
}

void VideoPlayer::sendHeartBeats()
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASVideoHeartBeats),
                                            ApiRequest::POST);
    apiRequest->addQueryParam("path", filePath);
    apiRequest->addQueryParam("token", token);
    // connect(apiRequest, &ApiRequest::responseRecieved, this, [] {});
    apiRequest->sendRequest();
}

void VideoPlayer::on_comboBoxVideoTrack_currentIndexChanged(int index)
{
    player->setActiveVideoTrack(index);
}

void VideoPlayer::on_comboBoxSoundTrack_currentIndexChanged(int index)
{
    player->setActiveAudioTrack(index);
}

void VideoPlayer::on_comboBoxAssTrack_currentIndexChanged(int index)
{
    player->setActiveSubtitleTrack(index);
}

void VideoPlayer::on_horizontalSlider_sliderMoved(int position)
{
    player->setPosition(position * 1000);
}

void VideoPlayer::on_toolButtonStartPause_clicked()
{
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
    } else {
        player->play();
    }
}

void VideoPlayer::on_toolButtonEnd_clicked()
{
    player->stop();
    ui->horizontalSlider->setValue(0);
    ui->labelCurrent->setText(formatDuration(0));
}

void VideoPlayer::on_toolButtonBackward_clicked()
{
    qint64 currentPos = player->position();
    qint64 newPos = currentPos - 5000;
    if (newPos < 0)
        newPos = 0;
    player->setPosition(newPos);
}

void VideoPlayer::on_toolButtonForward_clicked()
{
    qint64 currentPos = player->position();
    qint64 newPos = currentPos + 5000;
    if (newPos > player->duration())
        newPos = player->duration();
    player->setPosition(newPos);
}

void VideoPlayer::on_toolButtonFullscreen_clicked()
{
    if (isFullScreen()) {
        this->showNormal();
        uiHideTimer->stop();
        ui->frame->setVisible(true);
    } else {
        this->showFullScreen();
        ui->frame->setVisible(false);
        uiHideTimer->start();
    }
}
