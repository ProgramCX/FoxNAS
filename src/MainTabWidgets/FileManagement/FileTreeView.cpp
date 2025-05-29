#include "FileTreeView.h"
#include <qevent.h>

FileTreeView::FileTreeView(QWidget* parent)
    : QTreeView(parent)
{}

void FileTreeView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void FileTreeView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void FileTreeView::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            QString path = url.toLocalFile();
            qDebug() << "拖入文件：" << path;
            // 你可以在这里更新模型
        }
        event->acceptProposedAction();
    }
}
