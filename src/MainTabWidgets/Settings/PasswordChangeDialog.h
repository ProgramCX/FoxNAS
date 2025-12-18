#ifndef PASSWORDCHANGEDIALOG_H
#define PASSWORDCHANGEDIALOG_H

#include <QDialog>

namespace Ui {
class PasswordChangeDialog;
}

class PasswordChangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordChangeDialog(QWidget *parent = nullptr);
    ~PasswordChangeDialog();

private slots:
    void on_pushButtonOK_5_clicked();

private:
    Ui::PasswordChangeDialog *ui;
};

#endif // PASSWORDCHANGEDIALOG_H
