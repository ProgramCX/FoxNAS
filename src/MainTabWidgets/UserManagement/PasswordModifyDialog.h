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
    explicit PasswordModifyDialog(QString uuid, QWidget *parent = nullptr);
    ~PasswordModifyDialog();

    QString getUuid() const;
    void setUuid(const QString &newUuid);

private:
    QString userUuid;

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
