#ifndef DETAILDIALOG_H
#define DETAILDIALOG_H

#include <QDialog>

namespace Ui {
class DetailDialog;
}

class DetailDialog : public QDialog
{
    Q_OBJECT

public:
    enum MODE { edit, detail };
    explicit DetailDialog(QWidget *parent = nullptr, DetailDialog::MODE mode = edit);
    ~DetailDialog();

public:
    QString getName();
    QString getIp();
    quint16 getPort();
    void setName(QString name);
    void setIp(QString ip);
    void setPort(quint16 port);
private slots:
    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::DetailDialog *ui;
    QString name;
    QString ip;
    quint16 port;
};

#endif // DETAILDIALOG_H
