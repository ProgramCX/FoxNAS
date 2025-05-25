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

    qint64 getTotal() const;
    void setTotal(qint64 newTotal);

    qint64 getNow() const;
    void setNow(qint64 newNow);

private:
    Ui::LoadingDialog *ui;
    QString text;

    qint64 total;
    qint64 now;
};

#endif // LOADINGDIALOG_H
