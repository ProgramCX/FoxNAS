#include "FileTreeView.h"
#include <qevent.h>

#include <QDir>
#include <QFileInfo>

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
    QList<QString> filesPath;
    QList<QString> pathsPath;

    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            QString path = url.toLocalFile();

            QFileInfo info(path);
            if (info.isFile()) {
                filesPath.append(path);
            } else {
                pathsPath.append(path);
            }
        }
        emit fileDirsDraggedDrop(filesPath, pathsPath);
        event->acceptProposedAction();
    }
}
