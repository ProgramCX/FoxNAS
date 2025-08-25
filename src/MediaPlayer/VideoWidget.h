#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QObject>
#include <QVideoWidget>
#include "VideoPlayer.h"

class VideoPlayerWidget : public QVideoWidget
{
    Q_OBJECT

public:
    explicit VideoPlayerWidget(VideoPlayer *parent = nullptr)
        : QVideoWidget(parent)
        , parent(parent)
    {
        setMouseTracking(true);
        installEventFilter(this); // 安装事件过滤器
    }

    ~VideoPlayerWidget() {}

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == this && event->type() == QEvent::MouseMove) {
            // 鼠标移动事件触发
            qDebug() << "Mouse moved on VideoPlayerWidget";
            if (parent && parent->isFullScreen()) {
                parent->showControls(); // 显示控制面板
            }
            return true;
        }

        // 确保事件继续传递给父类，避免被阻断
        return QVideoWidget::eventFilter(watched, event);
    }

    // 直接在 VideoPlayerWidget 中重写 mouseMoveEvent
    void mouseMoveEvent(QMouseEvent *event) override
    {
        qDebug() << "Mouse move detected in VideoPlayerWidget";
        if (parent && parent->isFullScreen()) {
            parent->showControls();
        }
        QVideoWidget::mouseMoveEvent(event); // 确保继续传递给父类
    }

private:
    VideoPlayer *parent;
};

#endif // VIDEOWIDGET_H
