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
    explicit FilePermissionManagmentDialog(QString folderName,
                                           QVariantList list,
                                           QString userName,
                                           QWidget *parent = nullptr);
    ~FilePermissionManagmentDialog();

private slots:

    void on_pushButtonSelect_clicked();

    void on_checkBoxRead_clicked(bool checked);

    void on_checkBoxWrite_clicked(bool checked);

    void on_pushButtonCancel_clicked();

    void on_pushButtonOK_clicked();

private:
    Ui::FilePermissionManagmentDialog *ui;

    QStringList currentPermissionList;
    QString oldFolderName;
    QString userName;
    bool isNew;
};

#endif // FILEPERMISSIONMANAGMENTDIALOG_H
