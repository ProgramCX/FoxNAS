#ifndef FILEMANAGEMENTTABFORM_H
#define FILEMANAGEMENTTABFORM_H

#include <QWidget>

#include <QItemSelection>
#include <QKeyEvent>
#include <QMutex>
#include <QQueue>
#include <QStack>

#include <FileSystemRemoteModel.h>
#include <FileTranferListItem.h>

#include <IniSettings.hpp>

namespace Ui {
class FileManagementTabForm;
}

class FileManagementTabForm : public QWidget
{
    Q_OBJECT

public:
    explicit FileManagementTabForm(QWidget *parent = nullptr);
    ~FileManagementTabForm();

    void showSelectDirDialog();
private slots:
    void on_pushButtonSwitch_clicked();

    void on_pushButtonDeselectAll_clicked();

    void on_pushButtonSelectAll_clicked();

    void on_toolButtonGo_clicked();

    void on_pushButtonDownload_clicked();

    void on_pushButtonUpload_clicked();

    void on_comboBoxSort_currentIndexChanged(int index);

    void on_pushButtonCreateDir_clicked();

    void on_treeView_customContextMenuRequested(const QPoint &pos);

private:
    Ui::FileManagementTabForm *ui;
    FileSystemRemoteModel *model;

    QString currentPath;

    bool back = 0;
    bool forward = 0;
    bool isStartingTransfer = false;

    int inTaskFiles = 0;

    QString currentOrder = "asc";
    QString currentSortBy = "name";

    QStack<QString> backHistoryStack;
    QStack<QString> forwardHistoryStack;

    QQueue<FileTranferListItem *> taskQueue;
    QQueue<FileTranferListItem *> waitingQueue;

    QMutex inTaskMutex;

    QMenu *contextMenu;
    QMenu *sortOrderSubMenu;

private:
    void onScrollBarValueChanged(int value);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void iniTreeView();
    void iniContextMenu();
    void connectSlots();
    void handleItemDoubleClicked(const QModelIndex &index);

    void updateNavButtonState();
    bool hasSection();
    void backHistoryDir();
    void forwardHistoryDir();

    void backTopDir();

    void refresh();
    void deleteFiles();
    void createDir();

    void copyFiles();
    void pasteFiles();
    void cutFiles();
    void renameFile();
    void openFile();

    void removeItemFromTransferList(FileTranferListItem *item);
    void downloadFile(const QList<QString>& filesToDownload, const QString& savePath);

    void uploadFile(const QList<QString>& filesToUpload, const QString& savePath, const QString& basePath);
    void tryStartTransferNext();

    void handleTransferCompleted(FileTranferListItem *item);

    void handleTransferPaused(FileTranferListItem *item);

    void handleTransferFailed(FileTranferListItem *item);

    void handleTransferTryingResume(FileTranferListItem *item);

    void handleTransferCancel(FileTranferListItem *item);

    void handleFileDirsDraggedDrop(const QList<QString> &filesPath, const QList<QString> dirsPath);

    int getMaxiumInTaskCount();

    QList<QString> getSelectedFiles(bool hasDir = true) const;

    QList<QString> getFilePathRecursively(const QString &path) const;

    void updateComboBox(QString sortBy, QString order);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // FILEMANAGEMENTTABFORM_H
