#ifndef FILEMANAGEMENTFORM_H
#define FILEMANAGEMENTFORM_H

#include <QToolButton>
#include <QWidget>

#include "FileManagement/FileManagementTabForm.h"

namespace Ui {
class FileManagementForm;
}

class FileManagementForm : public QWidget
{
    Q_OBJECT

public:
    explicit FileManagementForm(QWidget *parent = nullptr);
    ~FileManagementForm();

private:
    Ui::FileManagementForm *ui;
    QToolButton *newToolButton;

private:
    void addWidgets();
    void addNewTab(QString path);

    int tabCount = 0;
};

#endif // FILEMANAGEMENTFORM_H
