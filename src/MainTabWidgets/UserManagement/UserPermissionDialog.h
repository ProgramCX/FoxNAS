#ifndef USERPERMISSIONDIALOG_H
#define USERPERMISSIONDIALOG_H

#include <QDialog>

class QTreeWidgetItem;
namespace Ui {
class UserPermissionDialog;
}

class UserPermissionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserPermissionDialog(QString uuid, QWidget *parent = nullptr);
    ~UserPermissionDialog();

private:
    Ui::UserPermissionDialog *ui;
    QString selectedPermission;
    QTreeWidgetItem *currentItem = nullptr;
    QString userUuid;
    QStringList allPermissionsList;
    QStringList grantedPermissions;

private:
    void iniUi();
    void getData();
    void updateButton();
    void grantPermission();
    void revokePermission();
};

#endif // USERPERMISSIONDIALOG_H
