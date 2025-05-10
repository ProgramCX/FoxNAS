#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QSet>
#include <QSettings>
#include <QUdpSocket>
#include "IniSettings.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_btnSearchServer_clicked(bool checked);

private:
    Ui::LoginDialog *ui;

    QHash<QString, QString> hashServer;

    QUdpSocket udpSocket;

    quint16 bindPort = 8845;

private:
    void getServerDatagram();

    QHash<QString, QString> getStoredServer();

    void storeServerInfo(QString name, QString fullhost);

    bool detectServer();

private slots:
    void onReadyReadDatagram();
    void on_btnChangePort_clicked();
    void onSelectChanged(QListWidgetItem *current, QListWidgetItem *previous);
};
#endif // LOGINDIALOG_H
