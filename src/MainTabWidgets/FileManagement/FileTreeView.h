#ifndef FILETREEVIEW_H
#define FILETREEVIEW_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTreeView>
class FileTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit FileTreeView(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

signals:
    void fileDirsDraggedDrop(const QList<QString>& filesPath, const QList<QString> dirsPath);
};

#endif // FILETREEVIEW_H
