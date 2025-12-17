#ifndef USERDIRPERMISSIONDIALOG_H
#define USERDIRPERMISSIONDIALOG_H

#include <QWidget>

namespace Ui {
class UserDirPermissionDialog;
}

class UserDirPermissionDialog : public QWidget
{
    Q_OBJECT

public:
    explicit UserDirPermissionDialog(QWidget *parent = nullptr);
    ~UserDirPermissionDialog();

private:
    Ui::UserDirPermissionDialog *ui;
};

#endif // USERDIRPERMISSIONDIALOG_H
