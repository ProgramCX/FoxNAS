#ifndef FILEMANAGEMENTTABFORM_H
#define FILEMANAGEMENTTABFORM_H

#include <QWidget>

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

private:
    Ui::FileManagementTabForm *ui;
    FileSystemRemoteModel *model;

private:
    void onScrollBarValueChanged(int value);
    void iniTreeView();
};

#endif // FILEMANAGEMENTTABFORM_H
