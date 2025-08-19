#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QAudioOutput>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QStringList>
#include <QTimer>
#include <QVideoWidget>
namespace Ui {
class VideoPlayer;
}

class VideoPlayer : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoPlayer(QString filePath, QWidget *parent = nullptr);
    ~VideoPlayer();

private slots:
    void on_comboBoxVideoTrack_currentIndexChanged(int index);

    void on_comboBoxSoundTrack_currentIndexChanged(int index);

    void on_comboBoxAssTrack_currentIndexChanged(int index);

    void on_horizontalSlider_sliderMoved(int position);

    void on_toolButtonStartPause_clicked();

    void on_toolButtonEnd_clicked();

    void on_toolButtonBackward_clicked();

    void on_toolButtonForward_clicked();

    void on_horizontalSlider_sliderPressed();

    void on_toolButtonFullscreen_clicked();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::VideoPlayer *ui;

    QVideoWidget *videoWidget;

    QMediaPlayer *player;
    QAudioOutput *audioOutput;

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

    bool doing = false;

private:
    void loadMedia(QString filePath);
    void iniMediaMeta();
    QString formatDuration(int durationSeconds);
    void changeButtonStateStop();
    void changeButtonStatePause();
    void changeButtonStatePlaying();
};

#endif // VIDEOPLAYER_H
