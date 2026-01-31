#ifndef USERMANAGEMENTFORM_H
#define USERMANAGEMENTFORM_H

#include <QWidget>

namespace Ui {
class UserManagementForm;
}
class QStandardItemModel;

class UserManagementForm : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementForm(QWidget *parent = nullptr);
    ~UserManagementForm();

private slots:
    void on_pushButtonFirstPage_clicked();

    void on_pushButtonLastPage_clicked();

    void on_pushButtonNextPage_clicked();

    void on_pushButtonTailPage_clicked();

    void on_pushButtonGoPage_clicked();

    void on_pushButtonNewRecord_clicked();

private:
    Ui::UserManagementForm *ui;
    QStandardItemModel *model;
    void iniTableViewStructure();
    void loadData(const QString &jsonString);
    void fetchData(int page);

    void changePassword(QString name, int row);
    void changeUserStatus(QString name, bool enable);
    void updateStatus();
    void updateButton();
    void addNewRow();
    void updateUser(int row);
void createUser(int row);
    void deleteUser(QString uuid, int row);
    void showSystemPermissions(QString name, int row);

private:
    int totalPages;
    int currentPage;
};

#endif // USERMANAGEMENTFORM_H
