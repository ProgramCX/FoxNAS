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

private:
    Ui::NASLoginDialog *ui;
    QString fullHost;

private:
    void login();
};

#endif // NASLOGINDIALOG_H
