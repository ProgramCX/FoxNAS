#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QAudioOutput>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QStringList>
#include <QTimer>
#include <QVideoWidget>

class VideoPlayerWidget;

namespace Ui {
class VideoPlayer;
}

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QString filePath, QWidget *parent = nullptr);
    ~VideoPlayer();

    void showControls();

private slots:
    void on_comboBoxVideoTrack_currentIndexChanged(int index);

    void on_comboBoxSoundTrack_currentIndexChanged(int index);

    void on_comboBoxAssTrack_currentIndexChanged(int index);

    void on_horizontalSlider_sliderMoved(int position);

    void on_toolButtonStartPause_clicked();

    void on_toolButtonEnd_clicked();

    void on_toolButtonBackward_clicked();

    void on_toolButtonForward_clicked();

    void on_toolButtonFullscreen_clicked();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    //     bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::VideoPlayer *ui;

    VideoPlayerWidget *videoWidget;

    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QTimer *heartBeatsTimer;

    QString filePath;
    QString token;

    QString title;
    int duration;
    int videoCount;
    int audioCount;

    QStringList audioList;
    QStringList videoList;
    QStringList assList;

    bool mediaReady = false;

    int retryCount = 0;
    QTimer *uiHideTimer;

    qint64 videoPosition = 0;

private:
    void loadMedia(QString filePath);
    void iniMediaMeta();
    QString formatDuration(int durationSeconds);
    void changeButtonStateStop();
    void changeButtonStatePause();
    void changeButtonStatePlaying();

    void sendHeartBeats();
};

#endif // VIDEOPLAYER_H
