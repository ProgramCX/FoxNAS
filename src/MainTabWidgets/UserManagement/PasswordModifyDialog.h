#ifndef PASSWORDMODIFYDIALOG_H
#define PASSWORDMODIFYDIALOG_H

#include <QDialog>

namespace Ui {
class PasswordModifyDialog;
}

class PasswordModifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordModifyDialog(QString userName, QWidget *parent = nullptr);
    ~PasswordModifyDialog();

    QString getUserName() const;
    void setUserName(const QString &newUserName);

private:
    QString userName;

private:
    bool verifyForm();

private slots:
    void on_pushButtonOK_clicked();

    void on_pushButtonOK_2_clicked();

    void changePassword();

private:
    Ui::PasswordModifyDialog *ui;
};

#endif // PASSWORDMODIFYDIALOG_H
