#include "FileManagementTabForm.h"
#include <ClipboardManager.h>
#include <DirsAuthedSelectDialog.h>

#include <QDateTime>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
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
    ui->pushButtonDelete->setEnabled(selected.indexes().size() > 0);
    ui->pushButtonCopy->setEnabled(selected.indexes().size() > 0);
    ui->pushButtonCut->setEnabled(selected.indexes().size() > 0);

    QModelIndex index = ui->treeView->currentIndex();
    RemoteFileSystemNode *node = static_cast<RemoteFileSystemNode *>(index.internalPointer());

    if (node) {
        ui->pushButtonEdit->setEnabled(node->type == "file");
        ui->pushButtonDownload->setEnabled(node->type == "file");
        ui->pushButtonDownload->setEnabled(node->type == "file");

        ui->pushButtonCopy->setEnabled(true);
        ui->pushButtonCut->setEnabled(true);
        ui->pushButtonDelete->setEnabled(true);
        ui->pushButtonOpen->setEnabled(true);
        ui->pushButtonProperty->setEnabled(true);
        ui->pushButtonRename->setEnabled(true);
    } else {
        ui->pushButtonEdit->setEnabled(false);
        ui->pushButtonCopy->setEnabled(false);
        ui->pushButtonCut->setEnabled(false);
        ui->pushButtonDelete->setEnabled(false);
        ui->pushButtonOpen->setEnabled(false);
        ui->pushButtonProperty->setEnabled(false);
        ui->pushButtonRename->setEnabled(false);
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
        QString newName = QInputDialog::getText(this,
                                                QString("重命名%1")
                                                    .arg(node->type == "file" ? tr("文件")
                                                                              : tr("文件夹")),
                                                QString("请输入%1文件的新名称：").arg(node->name));
        if (!newName.trimmed().isEmpty()) {
            model->renameFile(node->path, newName);
        }
    }
}

QList<QString> FileManagementTabForm::getSelectedFiles()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedRows(0);
    QList<QString> selectedItemPaths;
    for (const QModelIndex &index : selectedIndexes) {
        RemoteFileSystemNode *node = static_cast<RemoteFileSystemNode *>(index.internalPointer());
        if (node) {
            selectedItemPaths.append(node->path);
        }
    }
    return selectedItemPaths;
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
