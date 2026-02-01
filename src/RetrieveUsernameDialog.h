#ifndef RETRIEVEUSERNAMEDIALOG_H
#define RETRIEVEUSERNAMEDIALOG_H

#include <QDialog>

namespace Ui {
class RetrieveUsernameDialog;
}

class RetrieveUsernameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RetrieveUsernameDialog(const QString &fullHost, QWidget *parent = nullptr);
    ~RetrieveUsernameDialog();

private slots:
    void on_btnSendCode_clicked();
    void on_btnSubmit_clicked();
    void on_btnCancel_clicked();

private:
    Ui::RetrieveUsernameDialog *ui;
    QString fullHost;
    bool isSubmitting = false;

    void validateEmail();
    void sendVerifyCode();
    void retrieveUsername();
};
#endif // RETRIEVEUSERNAMEDIALOG_H
