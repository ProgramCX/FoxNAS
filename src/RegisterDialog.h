#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(const QString &fullHost, QWidget *parent = nullptr);
    ~RegisterDialog();

signals:
    void registerSuccess(const QString &userName, const QString &password);

private slots:
    void on_btnSendCode_clicked();
    void on_btnRegister_clicked();
    void on_btnCancel_clicked();

private:
    Ui::RegisterDialog *ui;
    QString fullHost;
    bool isSubmitting = false;
    void sendVerifyCode();
    void registerUser();
};

#endif // REGISTERDIALOG_H
