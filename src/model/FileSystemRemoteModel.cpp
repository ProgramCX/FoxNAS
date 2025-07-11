#include "FileSystemRemoteModel.h"
#include <QDateTime>
#include <QDebug>
#include <QFileIconProvider>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QThread>
#include <QUrlQuery>

#include <FileOperationFailedDialog.h>

#include <MemStore.h>

#include "../LoadingDialog.h"

FileSystemRemoteModel::FileSystemRemoteModel(QObject *parent)
    : QAbstractItemModel{parent}
{}

FileSystemRemoteModel::~FileSystemRemoteModel()
{
    delete rootNode;
}

QModelIndex FileSystemRemoteModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!rootNode || row < 0 || column < 0)
        return QModelIndex();

    RemoteFileSystemNode *parentNode = parent.isValid() ? static_cast<RemoteFileSystemNode *>(
                                                              parent.internalPointer())
                                                        : rootNode;

    if (row >= parentNode->children.size())
        return QModelIndex();

    return createIndex(row, column, parentNode->children[row]);
}

QModelIndex FileSystemRemoteModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    RemoteFileSystemNode *node = static_cast<RemoteFileSystemNode *>(index.internalPointer());
    if (!node || !node->parent || node->parent == rootNode)
        return QModelIndex();

    RemoteFileSystemNode *parentNode = node->parent;
    RemoteFileSystemNode *grandParent = parentNode->parent;
    int row = grandParent ? grandParent->children.indexOf(parentNode) : 0;

    return createIndex(row, 0, parentNode);
}

int FileSystemRemoteModel::rowCount(const QModelIndex &parent) const
{
    RemoteFileSystemNode *parentNode = parent.isValid() ? static_cast<RemoteFileSystemNode *>(
                                                              parent.internalPointer())
                                                        : rootNode;

    return parentNode ? parentNode->children.size() : 0;
}

int FileSystemRemoteModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant FileSystemRemoteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    RemoteFileSystemNode *node = static_cast<RemoteFileSystemNode *>(index.internalPointer());

    if (!node)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return node->name;
            break;
        case 1: {
            QString time = QDateTime::fromMSecsSinceEpoch(node->lastModified)
                               .toLocalTime()
                               .toString("yyyy/MM/dd hh:mm:ss.zzz");
            return time;
            break;
        }
        case 2: {
            return node->type == "directory" ? tr("文件夹") : tr("文件");
            break;
        }
        case 3: {
            if (node->size < 1024) {
                return QString::number(node->size) + " B";
            } else {
                return QString::number(node->size / 1024.0, 'f', 2) + " KB";
            }
            break;
        }
        default:
            return QVariant();
        }
    }

    if (role == Qt::DecorationRole && index.column() == 0) {
        QFileIconProvider iconProvider;
        if (node->type == "file") {
#ifdef Q_OS_WIN
            return QIcon(":/file/resource/favicon/file_text.svg");
#else
            QFileInfo fileInfo(node->name);
            return iconProvider.icon(fileInfo);
#endif

        } else if (node->type == "directory") {
#ifdef Q_OS_WIN
            return QIcon(":/file/resource/favicon/folder.svg");
#else
            return iconProvider.icon(QFileIconProvider::Folder);
#endif
        } else {
            return QIcon();
        }
    }

    return QVariant();
}

Qt::ItemFlags FileSystemRemoteModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant FileSystemRemoteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case 0:
        return tr("名称");
        break;
    case 1:
        return tr("修改时间");
        break;
    case 2:
        return tr("类型");
        break;
    case 3:
        return tr("大小");
        break;
    default:
        return QVariant();
    }
}

void FileSystemRemoteModel::setRootNode(RemoteFileSystemNode *node)
{
    beginResetModel();
    delete rootNode;
    rootNode = node;
    endResetModel();
}

void FileSystemRemoteModel::fetchDirectory(QString directory,
                                           bool refresh,
                                           QString order,
                                           QString sortBy)
{
    if (isFetchingDir)
        return;

    if (directory == currentDirectory && totalPage == currentPage && !refresh) {
        return;
    }

    QString fullApiPath = getFullApiPath(FULLHOST, NASFILEDIRLISTAPI);

    ApiRequest *request = new ApiRequest(fullApiPath, ApiRequest::GET, this);

    request->addQueryParam("path", directory);
    request->addQueryParam("order", order);
    request->addQueryParam("sortBy", sortBy);
    bool isCurrent = directory == currentDirectory;

    if (!isCurrent || refresh) {
        totalPage = 0;
        currentPage = 0;
        from = 0;
        to = 0;
    } else {
        request->addQueryParam("page", QString::number(++currentPage));
    }
    if (!rootNode) {
        return;
    }
    isFetchingDir = true;
    request->sendRequest();

    connect(request,
            &ApiRequest::responseRecieved,
            this,
            [this, request, isCurrent, directory, refresh](QString &rawContent,
                                                           bool hasError,
                                                           qint16 statusCode) {
                if (statusCode == 200) {
                    beginResetModel();

                    if (!isCurrent || refresh) {
                        qDeleteAll(rootNode->children);
                        rootNode->children.clear();
                        currentDirectory = directory;
                    }

                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    QJsonObject obj = doc.object();
                    totalPage = obj["totalPage"].toInt();
                    currentPage = obj["page"].toInt();

                    from = obj["from"].toInt();
                    to = obj["to"].toInt();

                    QJsonArray arr = obj["list"].toArray();
                    QList<QVariant> variantList = arr.toVariantList();
                    for (const QVariant &var : variantList) {
                        QMap<QString, QVariant> varMap = var.toMap();
                        RemoteFileSystemNode *newNode = new RemoteFileSystemNode;
                        newNode->name = varMap["name"].toString();
                        newNode->path = varMap["path"].toString();
                        newNode->size = varMap["size"].toDouble();
                        newNode->type = varMap["type"].toString();
                        newNode->lastModified = varMap["lastModified"].toLongLong();
                        newNode->parent = rootNode;

                        rootNode->children.append(newNode);
                    }
                    endResetModel();
                    emit currentPathChanged(currentDirectory);
                } else if (statusCode == 403) {
                    QMessageBox::critical(nullptr,
                                          "失败",
                                          "你可能没有权限访问该目录！请联系管理员！",
                                          tr("确定"));
                } else {
                    QMessageBox::critical(nullptr,
                                          "失败",
                                          "获取文件列表失败：" + rawContent,
                                          tr("确定"));
                }
                request->deleteLater();
                isFetchingDir = false;
            });
}

QString FileSystemRemoteModel::getOrder() const
{
    return order;
}

void FileSystemRemoteModel::setOrder(const QString &newOrder)
{
    order = newOrder;
}

QString FileSystemRemoteModel::getSortBy() const
{
    return sortBy;
}

void FileSystemRemoteModel::setSortBy(const QString &newSortBy)
{
    sortBy = newSortBy;
}

void FileSystemRemoteModel::deleteFiles(QList<QString> &pathList)
{
    if (pathList.isEmpty()) {
        return;
    }

    //构造JSON请求体
    QJsonDocument requestJson;
    QJsonArray jsonArray = QJsonArray::fromStringList(pathList);
    requestJson.setArray(jsonArray);

    ApiRequest *apiRequest = new ApiRequest(nullptr, ApiRequest::DELETE, requestJson, this);
    QString baseFullApi = getFullApiPath(FULLHOST, NASDIELETEFILEAPI);
    apiRequest->setApi(baseFullApi);

    LoadingDialog *loadingDialog = new LoadingDialog("正在删除文件...");

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [loadingDialog, this](QString &rawContent, bool hasError, qint16 statusCode) {
                loadingDialog->close();
                loadingDialog->deleteLater();
                if (statusCode == 200) {
                    fetchDirectory(currentDirectory, true);
                } else if (statusCode == 403) {
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    QMessageBox::critical(nullptr, "权限不足！", doc.object()["message"].toString());
                } else {
                    qint64 totalCount, failedCount = 0;
                    QJsonArray failedPathsJson;
                    getFailedInfo(rawContent, failedCount, totalCount, failedPathsJson);

                    QList<QString> failedPaths;

                    for (const auto &path : failedPathsJson) {
                        QJsonObject singleFailedPathObj = path.toObject();
                        failedPaths.append(singleFailedPathObj["path"].toString());
                    }

                    FileOperationFailedDialog *fileOpFiDialog
                        = new FileOperationFailedDialog(totalCount, failedCount, "删除文件失败");
                    fileOpFiDialog->addFileLists(failedPaths);
                    if (fileOpFiDialog->exec() == QDialog::Accepted) {
                        QList<QString> list = fileOpFiDialog->getResult();
                        deleteFiles(list);
                    }
                    fileOpFiDialog->deleteLater();
                }
            });

    apiRequest->sendRequest();
    loadingDialog->exec();
}

void FileSystemRemoteModel::copyFiles(QList<QMap<QString, QString>> &filesList)
{
    if (filesList.isEmpty()) {
        return;
    }

    QJsonDocument requestJson;
    QJsonArray requestArray;

    for (const QMap<QString, QString> &aFileCopyMap : filesList) {
        QVariantMap variantMap;
        for (auto it = aFileCopyMap.begin(); it != aFileCopyMap.end(); ++it) {
            variantMap.insert(it.key(), it.value());
        }
        QJsonObject requestPathObject = QJsonObject::fromVariantMap(variantMap);
        requestArray.append(requestPathObject);
    }

    requestJson.setArray(requestArray);

    ApiRequest *apiRequest = new ApiRequest(nullptr, ApiRequest::POST, requestJson, this);
    QString baseFullApi = getFullApiPath(FULLHOST, NASCOPYAPI);
    apiRequest->setApi(baseFullApi);

    LoadingDialog *loadingDialog = new LoadingDialog("正在复制文件...");

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [loadingDialog, this, apiRequest](QString &rawContent, bool hasError, qint16 statusCode) {
                loadingDialog->close();
                loadingDialog->deleteLater();
                if (statusCode == 200) {
                    fetchDirectory(currentDirectory, true);
                } else if (statusCode == 403) {
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    QMessageBox::critical(nullptr, "权限不足！", doc.object()["message"].toString());
                } else {
                    qint64 totalCount, failedCount = 0;
                    QJsonArray failedPathsJson;
                    getFailedInfo(rawContent, failedCount, totalCount, failedPathsJson);

                    QList<QMap<QString, QString>> failedPaths;

                    for (const auto &path : failedPathsJson) {
                        QJsonObject singleFailedPathObj = path.toObject();

                        QJsonArray array = singleFailedPathObj["path"].toArray();
                        QMap<QString, QString> pathMap;
                        if (array.size() == 2) {
                            pathMap.insert("oldPath", array[0].toString());
                            pathMap.insert("newPath", array[1].toString());
                        }
                        failedPaths.append(pathMap);
                    }

                    FileOperationFailedDialog *fileOpFiDialog
                        = new FileOperationFailedDialog(totalCount, failedCount, "复制文件失败");

                    fileOpFiDialog->addFileLists(failedPaths);
                    if (fileOpFiDialog->exec() == QDialog::Accepted) {
                        QList<QMap<QString, QString>> list = fileOpFiDialog->getMapResult();
                        copyFiles(list);
                    }
                    fileOpFiDialog->deleteLater();
                }
                apiRequest->deleteLater();
            });

    apiRequest->sendRequest();
    loadingDialog->exec();
}

void FileSystemRemoteModel::moveFiles(QList<QMap<QString, QString>> &filesList)
{
    if (filesList.isEmpty()) {
        return;
    }

    QJsonDocument requestJson;
    QJsonArray requestArray;

    for (const QMap<QString, QString> &aFileCopyMap : filesList) {
        QVariantMap variantMap;
        for (auto it = aFileCopyMap.begin(); it != aFileCopyMap.end(); ++it) {
            variantMap.insert(it.key(), it.value());
        }
        QJsonObject requestPathObject = QJsonObject::fromVariantMap(variantMap);
        requestArray.append(requestPathObject);
    }

    requestJson.setArray(requestArray);

    ApiRequest *apiRequest = new ApiRequest(nullptr, ApiRequest::PUT, requestJson, this);
    QString baseFullApi = getFullApiPath(FULLHOST, NASMOVEAPI);
    apiRequest->setApi(baseFullApi);

    LoadingDialog *loadingDialog = new LoadingDialog("正在移动文件...");

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [loadingDialog, this, apiRequest](QString &rawContent, bool hasError, qint16 statusCode) {
                loadingDialog->close();
                loadingDialog->deleteLater();
                if (statusCode == 200) {
                    fetchDirectory(currentDirectory, true);
                } else if (statusCode == 403) {
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    QMessageBox::critical(nullptr, "权限不足！", doc.object()["message"].toString());
                } else {
                    qint64 totalCount, failedCount = 0;
                    QJsonArray failedPathsJson;
                    getFailedInfo(rawContent, failedCount, totalCount, failedPathsJson);

                    QList<QMap<QString, QString>> failedPaths;

                    for (const auto &path : failedPathsJson) {
                        QJsonObject singleFailedPathObj = path.toObject();

                        QJsonArray array = singleFailedPathObj["path"].toArray();
                        QMap<QString, QString> pathMap;
                        if (array.size() == 2) {
                            pathMap.insert("oldPath", array[0].toString());
                            pathMap.insert("newPath", array[1].toString());
                        }
                        failedPaths.append(pathMap);
                    }

                    FileOperationFailedDialog *fileOpFiDialog
                        = new FileOperationFailedDialog(totalCount, failedCount, "移动文件失败");

                    fileOpFiDialog->addFileLists(failedPaths);
                    if (fileOpFiDialog->exec() == QDialog::Accepted) {
                        QList<QMap<QString, QString>> list = fileOpFiDialog->getMapResult();
                        moveFiles(list);
                    }
                    fileOpFiDialog->deleteLater();
                }
                apiRequest->deleteLater();
            });

    apiRequest->sendRequest();
    loadingDialog->exec();
}

void FileSystemRemoteModel::renameFile(QString path, QString newName)
{
    QString apiRequestPath = getFullApiPath(FULLHOST, NASRENAMEFILEAPI);

    ApiRequest *apiRequest = new ApiRequest(apiRequestPath, ApiRequest::PUT, this);
    apiRequest->addQueryParam("path", path);
    apiRequest->addQueryParam("newName", newName);

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, path, apiRequest](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    fetchDirectory(currentDirectory, true);
                } else if (statusCode == 403) {
                    QMessageBox::information(nullptr,
                                             "重命名失败",
                                             "暂无权限修改 " + path + " 名称！请联系管理员。",
                                             tr("确定"));
                } else if (statusCode == 500) {
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    if (doc.isObject()) {
                        QJsonObject obj = doc.object();
                        QMessageBox::information(nullptr,
                                                 "重命名失败",
                                                 QString("无法将 %1 重命名为 %2!")
                                                     .arg(obj["path"].toString(),
                                                          obj["renameTo"].toString()),
                                                 tr("确定"));

                    } else {
                        QMessageBox::information(nullptr,
                                                 "重命名失败",
                                                 "无法重命名文件！",
                                                 tr("确定"));
                    }

                } else {
                    QMessageBox::information(nullptr, "重命名失败", "无法重命名文件！", tr("确定"));
                }
                apiRequest->deleteLater();
            });
    apiRequest->sendRequest();
}

void FileSystemRemoteModel::createDir(QString path)
{
    QString apiCreateDir = getFullApiPath(FULLHOST, NASCREATEDIRAPI);
    auto *apiRequest = new ApiRequest(apiCreateDir, ApiRequest::POST, this);
    apiRequest->addQueryParam("path", path);

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, apiRequest, path](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    fetchDirectory(currentDirectory, true);
                } else if (statusCode == 403) {
                    QMessageBox::information(nullptr,
                                             "创建目录失败",
                                             "暂无权限创建文件夹 " + path + " ！请联系管理员。",
                                             tr("确定"));
                } else if (statusCode == 500) {
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    if (doc.isObject()) {
                        QJsonObject obj = doc.object();
                        QMessageBox::information(nullptr,
                                                 "创建目录失败",
                                                 QString(" %1：%2").arg(obj["path"].toString(),
                                                                        obj["message"].toString()),
                                                 tr("确定"));

                    } else {
                        QMessageBox::information(nullptr,
                                                 "创建目录失败",
                                                 "无法创建文件夹！",
                                                 tr("确定"));
                    }

                } else {
                    QMessageBox::information(nullptr,
                                             "创建目录失败",
                                             "无法创建文件夹！",
                                             tr("确定"));
                }
                apiRequest->deleteLater();
            });
    apiRequest->sendRequest();
}

void FileSystemRemoteModel::getFailedInfo(QString &rawContent,
                                          qint64 &failedCount,
                                          qint64 &totalCount,
                                          QJsonArray &failedPathsJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());

    QJsonObject obj = doc.object();

    totalCount = obj["totalCount"].toInt();
    failedCount = obj["failedCount"].toInt();

    failedPathsJson = obj["failedPaths"].toArray();
}
