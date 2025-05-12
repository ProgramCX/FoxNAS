#ifndef ADDNASDIALOG_H
#define ADDNASDIALOG_H

#include <QDialog>

namespace Ui {
class AddNASDialog;
}

class AddNASDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddNASDialog(QWidget *parent = nullptr);
    ~AddNASDialog();

public:
    QString getName();
    QString getIpAddress();
    quint16 getPort();

private:
    Ui::AddNASDialog *ui;
    QString name;
    QString ipAddress;
    quint16 port;
};

#endif // ADDNASDIALOG_H
