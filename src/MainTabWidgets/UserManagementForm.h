#ifndef USERMANAGEMENTFORM_H
#define USERMANAGEMENTFORM_H

#include <QWidget>

namespace Ui {
class UserManagementForm;
}

class UserManagementForm : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementForm(QWidget *parent = nullptr);
    ~UserManagementForm();

private:
    Ui::UserManagementForm *ui;
};

#endif // USERMANAGEMENTFORM_H
