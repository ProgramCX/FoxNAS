#ifndef FILESYSTEMREMOTEMODEL_H
#define FILESYSTEMREMOTEMODEL_H

#include <QAbstractItemModel>
#include <QObject>

#include "RemoteFileSystemNode.h"

#include <ApiRequest.h>
#include <ApiUrl.h>

class FileSystemRemoteModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit FileSystemRemoteModel(QObject *parent = nullptr);
    ~FileSystemRemoteModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setRootNode(RemoteFileSystemNode *node);

    void fetchDirectory(QString directory,
                        bool refresh = 0,
                        QString order = "asc",
                        QString sortBy = "name");

    QString getOrder() const;
    void setOrder(const QString &newOrder);

    QString getSortBy() const;
    void setSortBy(const QString &newSortBy);

    void deleteFiles(QList<QString> &pathList);
    void copyFiles(QList<QMap<QString, QString>> &filesList);
    void moveFiles(QList<QMap<QString, QString>> &filesList);
    void renameFile(QString path, QString newName);
    void createDir(QString path);

private:
    RemoteFileSystemNode *rootNode = nullptr;

    int currentPage = 0;
    int totalPage = 0;

    int from = 0;
    int to = 0;

    QString order = "asc";
    QString sortBy = "name";
    QString currentDirectory;
    void getFailedInfo(QString &rawContent,
                       qint64 &failedCount,
                       qint64 &totalCount,
                       QJsonArray &failedPathsJson);
signals:
    QString currentPathChanged(QString &path) const;
};

#endif // FILESYSTEMREMOTEMODEL_H
