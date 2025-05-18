#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>

namespace Ui {
class LoadingDialog;
}

class LoadingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingDialog(QString textShow, QWidget *parent = nullptr);
    ~LoadingDialog();
    void setInfoText(QString &text);
    QString getInfoText();

private:
    Ui::LoadingDialog *ui;
    QString text;
};

#endif // LOADINGDIALOG_H
