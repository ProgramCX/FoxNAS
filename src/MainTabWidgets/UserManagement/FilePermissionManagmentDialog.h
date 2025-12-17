#ifndef FILEPERMISSIONMANAGMENTDIALOG_H
#define FILEPERMISSIONMANAGMENTDIALOG_H

#include <QDialog>

namespace Ui {
class FilePermissionManagmentDialog;
}

class FilePermissionManagmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilePermissionManagmentDialog(QWidget *parent = nullptr);
    ~FilePermissionManagmentDialog();

private slots:

    void on_pushButtonSelect_clicked();

private:
    Ui::FilePermissionManagmentDialog *ui;
};

#endif // FILEPERMISSIONMANAGMENTDIALOG_H
