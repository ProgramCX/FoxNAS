#ifndef USERDIRPERMISSIONDIALOG_H
#define USERDIRPERMISSIONDIALOG_H

#include <QWidget>
class QTreeWidgetItem;
namespace Ui {
class UserDirPermissionDialog;
}

class UserDirPermissionDialog : public QWidget
{
    Q_OBJECT

public:
    explicit UserDirPermissionDialog(QString userName, QWidget *parent = nullptr);
    ~UserDirPermissionDialog();

private slots:
    void on_pushButtonModifyDir_clicked();

    void on_pushButtonNewDir_clicked();

    void on_pushButtonDeleteDir_clicked();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
    Ui::UserDirPermissionDialog *ui;
    QString currentUserName;

private:
    void loadData();
};

#endif // USERDIRPERMISSIONDIALOG_H
