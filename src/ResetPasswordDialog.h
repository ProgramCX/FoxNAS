#ifndef RESETPASSWORDDIALOG_H
#define RESETPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class ResetPasswordDialog;
}

class ResetPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetPasswordDialog(const QString &fullHost, QWidget *parent = nullptr);
    ~ResetPasswordDialog();

private slots:
    void on_btnSendCode_clicked();
    void on_btnResetPassword_clicked();
    void on_btnCancel_clicked();

private:
    Ui::ResetPasswordDialog *ui;
    QString fullHost;
    bool isSubmitting = false;
    int currentStep = 1;

    void sendVerifyCode();
    void resetPassword();
    void validateEmail();
};
#endif // RESETPASSWORDDIALOG_H
