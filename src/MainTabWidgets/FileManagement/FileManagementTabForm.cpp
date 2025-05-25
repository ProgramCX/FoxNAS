#include "FileManagementTabForm.h"
#include <DirsAuthedSelectDialog.h>

#include <QDateTime>
#include <QDir>
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
        if (!currentPath.isEmpty() && !back && !forward) {
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

void FileManagementTabForm::on_pushButtonSwitch_clicked()
{
    showSelectDirDialog();
}
