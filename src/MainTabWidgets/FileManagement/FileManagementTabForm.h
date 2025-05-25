#ifndef FILEMANAGEMENTTABFORM_H
#define FILEMANAGEMENTTABFORM_H

#include <QWidget>

#include <QStack>
#include <FileSystemRemoteModel.h>
#include <RemoteFileSystemNode.h>

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

private:
    Ui::FileManagementTabForm *ui;
    FileSystemRemoteModel *model;

    QString currentPath;
    bool back = 0;
    bool forward = 0;

    QStack<QString> backHistoryStack;
    QStack<QString> forwardHistoryStack;

private:
    void onScrollBarValueChanged(int value);
    void iniTreeView();
    void connectSlots();
    void handleItemDoubleClicked(const QModelIndex &index);

    void updateNavButtonState();

    void backHistoryDir();
    void forwardHistoryDir();

    void backTopDir();

    void refresh();
    void deleteFiles();
};

#endif // FILEMANAGEMENTTABFORM_H
