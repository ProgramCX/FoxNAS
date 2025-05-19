#ifndef FILEMANAGEMENTFORM_H
#define FILEMANAGEMENTFORM_H

#include <QWidget>

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
};

#endif // FILEMANAGEMENTFORM_H
