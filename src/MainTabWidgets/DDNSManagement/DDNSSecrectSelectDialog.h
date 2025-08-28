#ifndef DDNSSECRECTSELECTDIALOG_H
#define DDNSSECRECTSELECTDIALOG_H

#include <QDialog>

namespace Ui {
class DDNSSecrectSelectDialog;
}

class DDNSSecretForm;
class DDNSSecrectSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DDNSSecrectSelectDialog(QWidget *parent = nullptr);
    ~DDNSSecrectSelectDialog();

    int getSelectedId() const;

private slots:
    void on_pushButtonOK_clicked();

    void on_pushButtonCancel_clicked();

private:
    Ui::DDNSSecrectSelectDialog *ui;

    DDNSSecretForm *ddnsSecretForm;

    int selectedId = -1;
};

#endif // DDNSSECRECTSELECTDIALOG_H
