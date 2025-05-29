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

private:
    Ui::FileManagementTabForm *ui;
    FileSystemRemoteModel *model;

    QString currentPath;
    bool back = 0;
    bool forward = 0;
    int inTaskFiles = 0;

    QStack<QString> backHistoryStack;
    QStack<QString> forwardHistoryStack;

    QQueue<FileTranferListItem *> taskQueue;
    QQueue<FileTranferListItem *> waitingQueue;

    QMutex inTaskMutex;
    bool isStartingDownload = false;

private:
    void onScrollBarValueChanged(int value);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void iniTreeView();
    void connectSlots();
    void handleItemDoubleClicked(const QModelIndex &index);

    void updateNavButtonState();
    bool hasSection();
    void backHistoryDir();
    void forwardHistoryDir();

    void backTopDir();

    void refresh();
    void deleteFiles();

    void copyFiles();
    void pasteFiles();
    void cutFiles();
    void renameFile();

    void removeItemFromTransferList(FileTranferListItem *item);
    void downloadFile(QList<QString> filesToDownload, QString savePath);

    void tryStartDownloadNext();

    QList<QString> getSelectedFiles(bool hasDir = true);
    int getMaxiumInTaskCount();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // FILEMANAGEMENTTABFORM_H
