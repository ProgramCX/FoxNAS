#ifndef REMOTEFILESYSTEMNODE_H
#define REMOTEFILESYSTEMNODE_H
#include <QList>
#include <QString>
struct RemoteFileSystemNode
{
    QString name;
    QString path;
    double size;
    QString sizeUnit;
    qint64 lastModified;
    QString type;
    RemoteFileSystemNode* parent = nullptr;
    QList<RemoteFileSystemNode*> children;
};
#endif // REMOTEFILESYSTEMNODE_H
