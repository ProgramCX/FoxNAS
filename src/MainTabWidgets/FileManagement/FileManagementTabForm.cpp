#include "FileManagementTabForm.h"
#include <ClipboardManager.h>
#include <DirsAuthedSelectDialog.h>

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMutex>
#include <QScrollBar>
#include <QTreeView>

#include "ui_FileManagementTabForm.h"
FileManagementTabForm::FileManagementTabForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileManagementTabForm)
{
    ui->setupUi(this);

    iniTreeView();
    connectSlots();
}

FileManagementTabForm::~FileManagementTabForm()
{
    delete ui;
}

void FileManagementTabForm::showSelectDirDialog()
{
    DirsAuthedSelectDialog *dialog = new DirsAuthedSelectDialog;
    if (dialog->exec() == QDialog::Accepted) {
        model->fetchDirectory(dialog->getSelectedPath());
    }
    dialog->deleteLater();
}

void FileManagementTabForm::onScrollBarValueChanged(int value)
{
    QScrollBar *scrollBar = ui->treeView->verticalScrollBar();
    int maxValue = scrollBar->maximum();

    if (value == maxValue) {
        model->fetchDirectory(currentPath);
    }
}

void FileManagementTabForm::onSelectionChanged(const QItemSelection &selected,
                                               const QItemSelection &deselected)
{
    if (selected.indexes().size() == 0) {
        ui->pushButtonDelete->setEnabled(false);
        ui->pushButtonCopy->setEnabled(false);
        ui->pushButtonCut->setEnabled(false);
        ui->pushButtonOpen->setEnabled(false);
        ui->pushButtonProperty->setEnabled(false);
        ui->pushButtonRename->setEnabled(false);
        return;
    }
    ui->pushButtonDelete->setEnabled(true);
    ui->pushButtonCopy->setEnabled(true);
    ui->pushButtonCut->setEnabled(true);
    ui->pushButtonOpen->setEnabled(true);
    ui->pushButtonProperty->setEnabled(true);
    ui->pushButtonRename->setEnabled(true);

    QModelIndex index = ui->treeView->currentIndex();
    RemoteFileSystemNode *node = static_cast<RemoteFileSystemNode *>(index.internalPointer());

    if (node) {
        ui->pushButtonEdit->setEnabled(node->type == "file");
        ui->pushButtonDownload->setEnabled(node->type == "file");
        ui->pushButtonDownload->setEnabled(node->type == "file");
    }
}

void FileManagementTabForm::iniTreeView()
{
    RemoteFileSystemNode *root = new RemoteFileSystemNode;
    root->name = "/";
    root->type = "directory";
    root->lastModified = QDateTime::currentMSecsSinceEpoch();
    root->size = 0;
    root->path = "/";

    model = new FileSystemRemoteModel(this);
    model->setRootNode(root);

    // model->fetchDirectory("C:/Windows/System32");
    ui->treeView->setModel(model);

    connect(ui->treeView->verticalScrollBar(),
            &QScrollBar::valueChanged,
            this,
            &FileManagementTabForm::onScrollBarValueChanged);

    connect(model, &FileSystemRemoteModel::currentPathChanged, [this](const QString &path) {
        if (!currentPath.isEmpty() && !back && !forward && currentPath != path) {
            backHistoryStack.push(currentPath);
        }

        back = 0;
        forward = 0;
        qDebug() << backHistoryStack;
        currentPath = path;
        ui->comboBoxPath->setCurrentText(path);
        updateNavButtonState();
    });

    ui->treeView->setColumnWidth(0, 300);
    ui->treeView->setColumnWidth(1, 200);
    ui->treeView->setColumnWidth(2, 100);
    ui->treeView->setColumnWidth(3, 150);

    if (ClipboardManager::instance().getClipboardMode() == ClipboardManager::COPY) {
        bool hasContent = !ClipboardManager::instance().getCopiedFiles().isEmpty();
        ui->pushButtonPaste->setEnabled(hasContent);
    } else if (ClipboardManager::instance().getClipboardMode() == ClipboardManager::CUT) {
        bool hasContent = !ClipboardManager::instance().getCutFiles().isEmpty();
        ui->pushButtonPaste->setEnabled(hasContent);
    }

    ui->treeView->setAcceptDrops(true);
    ui->treeView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->treeView->setDefaultDropAction(Qt::CopyAction);
}

void FileManagementTabForm::connectSlots()
{
    connect(ui->treeView,
            &QTreeView::doubleClicked,
            this,
            &FileManagementTabForm::handleItemDoubleClicked);
    connect(ui->toolButtonNext,
            &QToolButton::clicked,
            this,
            &FileManagementTabForm::forwardHistoryDir);
    connect(ui->toolButtonPrevious,
            &QToolButton::clicked,
            this,
            &FileManagementTabForm::backHistoryDir);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &FileManagementTabForm::backTopDir);
    connect(ui->toolButtonRefresh, &QToolButton::clicked, this, &FileManagementTabForm::refresh);
    connect(ui->pushButtonDelete, &QPushButton::clicked, this, &FileManagementTabForm::deleteFiles);

    connect(ui->pushButtonCopy, &QPushButton::clicked, this, &FileManagementTabForm::copyFiles);
    connect(ui->pushButtonCut, &QPushButton::clicked, this, &FileManagementTabForm::cutFiles);
    connect(ui->pushButtonPaste, &QPushButton::clicked, this, &FileManagementTabForm::pasteFiles);
    connect(ui->pushButtonRename, &QPushButton::clicked, this, &FileManagementTabForm::renameFile);

    connect(&ClipboardManager::instance(), &ClipboardManager::clipboardCopyChanged, this, [this] {
        if (ClipboardManager::instance().getClipboardMode() == ClipboardManager::COPY) {
            bool hasContent = !ClipboardManager::instance().getCopiedFiles().isEmpty();
            ui->pushButtonPaste->setEnabled(hasContent);
        }
    });

    connect(&ClipboardManager::instance(), &ClipboardManager::clipboardCutChanged, this, [this] {
        if (ClipboardManager::instance().getClipboardMode() == ClipboardManager::CUT) {
            bool hasContent = !ClipboardManager::instance().getCutFiles().isEmpty();
            ui->pushButtonPaste->setEnabled(hasContent);
        }
    });

    connect(ui->treeView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &FileManagementTabForm::onSelectionChanged);
}

void FileManagementTabForm::handleItemDoubleClicked(const QModelIndex &index)
{
    // 获取模型项指针
    FileSystemRemoteModel *model = qobject_cast<FileSystemRemoteModel *>(ui->treeView->model());
    if (!model)
        return;

    RemoteFileSystemNode *item = static_cast<RemoteFileSystemNode *>(index.internalPointer());

    if (item) {
        if (item->type == "directory") {
            model->fetchDirectory(item->path);
            onSelectionChanged(QItemSelection(), QItemSelection());
        } else if (item->type == "file") {
        }
    }
}

void FileManagementTabForm::updateNavButtonState()
{
    ui->toolButtonNext->setEnabled(forwardHistoryStack.size() > 0);
    ui->toolButtonPrevious->setEnabled(backHistoryStack.size() > 0);

    QDir dir(currentPath);

    ui->toolButtonUp->setEnabled(!dir.isRoot());
    ui->toolButtonRefresh->setEnabled(!currentPath.isEmpty());

    ui->pushButtonDeselectAll->setEnabled(!currentPath.isEmpty());
    ui->pushButtonSelectAll->setEnabled(!currentPath.isEmpty());

    ui->pushButtonUpload->setEnabled(!currentPath.isEmpty());
    ui->comboBoxSort->setEnabled(!currentPath.isEmpty());
}

bool FileManagementTabForm::hasSection()
{
    QItemSelectionModel *selection = ui->treeView->selectionModel();
    return selection->hasSelection();
}

void FileManagementTabForm::backHistoryDir()
{
    if (backHistoryStack.isEmpty())
        return;

    back = 1;

    QString lastDir = backHistoryStack.pop();
    model->fetchDirectory(lastDir);

    forwardHistoryStack.push(currentPath);
    updateNavButtonState();
}

void FileManagementTabForm::forwardHistoryDir()
{
    if (forwardHistoryStack.isEmpty())
        return;

    forward = 1;

    QString forDir = forwardHistoryStack.pop();
    model->fetchDirectory(forDir);

    backHistoryStack.push(currentPath);
    updateNavButtonState();
}

void FileManagementTabForm::backTopDir()
{
    QDir dir(currentPath);
    if (!dir.isRoot() && dir.cdUp()) {
        model->fetchDirectory(dir.absolutePath());
    }
    updateNavButtonState();
}

void FileManagementTabForm::refresh()
{
    if (currentPath.isEmpty())
        return;
    model->fetchDirectory(currentPath, true);
}

void FileManagementTabForm::deleteFiles()
{
    QList<QString> selectedItemPaths = getSelectedFiles();

    if (QMessageBox::question(
            this,
            "是否删除",
            QString("你将删除%1个文件和目录，是否继续？删除过程中无法取消操作，此操作不可逆！")
                .arg(selectedItemPaths.size()))
        == QMessageBox::Yes) {
        model->deleteFiles(selectedItemPaths);
    }
}

void FileManagementTabForm::copyFiles()
{
    QList<QString> selectedItemPaths = getSelectedFiles();
    ClipboardManager::instance().setCopiedFiles(selectedItemPaths);
}

void FileManagementTabForm::pasteFiles()
{
    QList<QMap<QString, QString>> pathList;
    QList<QString> sourceFilePathList;
    if (ClipboardManager::instance().getClipboardMode() == ClipboardManager::COPY) {
        sourceFilePathList = ClipboardManager::instance().getCopiedFiles();
    } else if (ClipboardManager::instance().getClipboardMode() == ClipboardManager::CUT) {
        sourceFilePathList = ClipboardManager::instance().getCutFiles();
    }

    if (sourceFilePathList.isEmpty()) {
        return;
    }

    for (const QString sourceFilePath : sourceFilePathList) {
        QMap<QString, QString> copyMap;
        QFileInfo sourceFile(sourceFilePath);
        QString fileName = sourceFile.fileName();
        copyMap.insert("oldPath", sourceFilePath);
        copyMap.insert("newPath", QDir(currentPath).filePath(fileName));
        pathList.append(copyMap);
    }

    if (ClipboardManager::instance().getClipboardMode() == ClipboardManager::COPY) {
        model->copyFiles(pathList);
    } else if (ClipboardManager::instance().getClipboardMode() == ClipboardManager::CUT) {
        model->moveFiles(pathList);
    }

    ClipboardManager::instance().clearClipboard();
}

void FileManagementTabForm::cutFiles()
{
    QList<QString> selectedItemPaths = getSelectedFiles();
    ClipboardManager::instance().setCutFiles(selectedItemPaths);
}

void FileManagementTabForm::renameFile()
{
    QModelIndex index = ui->treeView->currentIndex();
    RemoteFileSystemNode *node = static_cast<RemoteFileSystemNode *>(index.internalPointer());

    if (node) {
        QString newName = QInputDialog::getText(
            this,
            QString("重命名%1").arg(node->type == "file" ? tr("文件") : tr("文件夹")),
            QString("请输入 %1 %2的新名称：")
                .arg(node->name, node->type == "file" ? tr("文件") : tr("文件夹")));
        if (!newName.trimmed().isEmpty()) {
            model->renameFile(node->path, newName);
        }
    }
}

void FileManagementTabForm::removeItemFromTransferList(FileTranferListItem *item)
{
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *listItem = ui->listWidget->item(i);
        if (ui->listWidget->itemWidget(listItem) == item) {
            ui->listWidget->removeItemWidget(listItem);
            delete listItem;
            break;
        }
    }
}

void FileManagementTabForm::downloadFile(QList<QString> filesToDownload, QString savePath)
{
    for (const QString &file : filesToDownload) {
        FileTranferListItem *widget = new FileTranferListItem(file,
                                                              savePath,
                                                              FileTranferListItem::DOWNLOAD,
                                                              this);
        taskQueue.append(widget);
        waitingQueue.append(widget);

        connect(widget,
                &FileTranferListItem::transferCompleted,
                this,
                [this](FileTranferListItem *item) {
                    taskQueue.removeOne(item);
                    removeItemFromTransferList(item);
                    item->deleteLater();

                    {
                        QMutexLocker locker(&inTaskMutex);
                        inTaskFiles--;
                    }

                    tryStartDownloadNext();
                });

        connect(widget,
                &FileTranferListItem::transferFailed,
                this,
                [this](FileTranferListItem *item) {
                    {
                        QMutexLocker locker(&inTaskMutex);
                        inTaskFiles--;
                    }
                    item->setMessageText("下载失败！");
                    tryStartDownloadNext();
                });

        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(widget->sizeHint());
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, widget);

        tryStartDownloadNext();
    }
}

void FileManagementTabForm::tryStartDownloadNext()
{
    QMutexLocker locker(&inTaskMutex);
    if (isStartingDownload) {
        return;
    }
    isStartingDownload = true;

    if (inTaskFiles < getMaxiumInTaskCount() && !waitingQueue.isEmpty()) {
        FileTranferListItem *next = waitingQueue.dequeue(); //从等待队列拿
        inTaskFiles++;
        next->startTransfer();
        qDebug() << "正在下载的" << inTaskFiles;
    }
    qDebug() << "正在下载的（可能最后）" << inTaskFiles;
    isStartingDownload = false;
}

QList<QString> FileManagementTabForm::getSelectedFiles(bool hasDir)
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedRows(0);
    QList<QString> selectedItemPaths;
    for (const QModelIndex &index : selectedIndexes) {
        RemoteFileSystemNode *node = static_cast<RemoteFileSystemNode *>(index.internalPointer());
        if (node) {
            if (hasDir || node->type == "file") {
                selectedItemPaths.append(node->path);
            }
        }
    }
    return selectedItemPaths;
}

int FileManagementTabForm::getMaxiumInTaskCount()
{
    QSettings &settings = IniSettings::getGlobalSettingsInstance();
    return settings.value("file/maxQueue", 3).toInt();
}

void FileManagementTabForm::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
        if (hasSection()) {
            copyFiles();
        }
    } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_V) {
            pasteFiles();
    } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_X) {
        if (hasSection()) {
            cutFiles();
        }
    } else if (event->key() == Qt::Key_Delete) {
        if (hasSection()) {
            deleteFiles();
        }
    } else if (event->key() == Qt::Key_F2) {
        if (hasSection()) {
            renameFile();
        }
    }
}

void FileManagementTabForm::on_pushButtonSwitch_clicked()
{
    showSelectDirDialog();
}

void FileManagementTabForm::on_pushButtonDeselectAll_clicked()
{
    ui->treeView->clearSelection();
}

void FileManagementTabForm::on_pushButtonSelectAll_clicked()
{
    ui->treeView->selectAll();
}

void FileManagementTabForm::on_toolButtonGo_clicked()
{
    if (ui->comboBoxPath->currentText().isEmpty())
        return;

    model->fetchDirectory(ui->comboBoxPath->currentText());
}

void FileManagementTabForm::on_pushButtonDownload_clicked()
{
    QString savePath = QFileDialog::getExistingDirectory(this, tr("选择保存位置"));
    if (savePath.isEmpty()) {
        return;
    }

    QList<QString> selectedPaths = getSelectedFiles(false);

    downloadFile(selectedPaths, savePath);
}
