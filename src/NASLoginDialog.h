#ifndef NASLOGINDIALOG_H
#define NASLOGINDIALOG_H

#include <QDialog>

namespace Ui {
class NASLoginDialog;
}

class NASLoginDialog : public QDialog
{
    Q_OBJECT

public:
    void setUserNameAndPassword(const QString &userName, const QString &password);
    explicit NASLoginDialog(const QString &fullHost, QWidget *parent = nullptr);
    ~NASLoginDialog();
    bool getLogined() { return this->logined; }
private slots:
    void on_buttonShowPassword_clicked(bool checked);
    void on_buttonRegister_clicked();
    void on_buttonForgotPassword_clicked();
    void on_buttonForgotUsername_clicked();

private:
    Ui::NASLoginDialog *ui;
    QString fullHost;
    bool logined = false;

private:
    void login();
};

#endif // NASLOGINDIALOG_H
