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
    explicit NASLoginDialog(const QString &fullHost, QWidget *parent = nullptr);
    ~NASLoginDialog();
    bool getLogined() { return this->logined; }
private slots:
    void on_buttonShowPassword_clicked(bool checked);

private:
    Ui::NASLoginDialog *ui;
    QString fullHost;
    bool logined = false;

private:
    void login();
};

#endif // NASLOGINDIALOG_H
