#include "LoginDialog.h"
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QtNetwork/QUdpSocket>
#include "ui_LoginDialog.h"
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    hashServer = this->getStoredServer();

    for (auto it = hashServer.constBegin(); it != hashServer.constEnd(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(it.key() + "(" + it.value() + ")");
        item->setSizeHint(QSize(80, 40));
        item->setData(2, it.key());
        item->setData(3, it.value());
        item->setData(4, true);
        ui->listWidget->addItem(item);
    }

    //默认启用搜索
    ui->btnSearchServer->setText(tr("停止搜索(&Q)"));
    ui->btnSearchServer->setChecked(true);
    ui->progressBar->setVisible(true);

    getServerDatagram();

    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &LoginDialog::onSelectChanged);
}

LoginDialog::~LoginDialog()
{
    udpSocket.close();
    delete ui;
}

void LoginDialog::getServerDatagram()
{
    if (!udpSocket.bind(bindPort)) {
        QMessageBox::critical(this, tr("错误"), tr("无法绑定UdpSocket，请更改绑定端口后重试"));
        ui->btnSearchServer->setText(tr("搜索(&S)"));
        ui->btnSearchServer->setChecked(false);
        ui->progressBar->setVisible(false);

        QObject::disconnect(&udpSocket,
                            &QUdpSocket::readyRead,
                            this,
                            &LoginDialog::onReadyReadDatagram);
    }

    QObject::connect(&udpSocket,
                     &QUdpSocket::readyRead,
                     this,
                     &LoginDialog::onReadyReadDatagram,
                     Qt::UniqueConnection);
}

QHash<QString, QString> LoginDialog::getStoredServer()
{
    QSettings &settings = IniSettings::getGlobalSettingsInstance();

    settings.beginGroup("server");

    QHash<QString, QString> hashServer;

    //把所有服务器信息键值对添加到hashServer中
    for (const QString &key : settings.allKeys()) {
        hashServer.insert(key, settings.value(key).toString());
    }

    settings.endGroup();

    return hashServer;
}

void LoginDialog::storeServerInfo(QString name, QString fullhost)
{
    QSettings &settings = IniSettings::getGlobalSettingsInstance();

    settings.beginGroup("server");

    settings.setValue(name, fullhost);

    settings.endGroup();
}

void LoginDialog::onReadyReadDatagram()
{
    while (udpSocket.hasPendingDatagrams()) {
        QByteArray byteArray;
        byteArray.resize(udpSocket.pendingDatagramSize());
        QHostAddress hostAddress;
        udpSocket.readDatagram(byteArray.data(), byteArray.size(), &hostAddress);

        QString name(byteArray);

        quint32 ipv4 = hostAddress.toIPv4Address();
        QString ipString = QHostAddress(ipv4).toString();

        name += "-" + ipString;
        QString fullHost = ipString;

        bool contains = false;

        //检查是否包含
        for (auto it = hashServer.constBegin(); it != hashServer.constEnd(); ++it) {
            if (it.key() == name && it.value() == fullHost) {
                contains = true;
                break;
            }
        }

        if (!contains) {
            QListWidgetItem *item = new QListWidgetItem(name);
            item->setSizeHint(QSize(100, 40));
            item->setData(2, name);
            item->setData(3, fullHost);
            item->setData(4, false);
            ui->listWidget->addItem(item);

            hashServer.insert(name, fullHost);
        }
    }
}

void LoginDialog::on_btnSearchServer_clicked(bool checked)
{
    if (checked) {
        getServerDatagram();
        ui->btnSearchServer->setText(tr("停止搜索(&Q)"));
    } else {
        QObject::disconnect(&udpSocket,
                            &QUdpSocket::readyRead,
                            this,
                            &LoginDialog::onReadyReadDatagram);
        udpSocket.close();
        ui->btnSearchServer->setText(tr("搜索(&S)"));
    }

    ui->progressBar->setVisible(checked);
}

void LoginDialog::on_btnChangePort_clicked()
{
    int value = QInputDialog::getInt(this, tr("设置端口"), tr("端口号:"), bindPort, 0, 65535);
    if (value) {
        bindPort = value;
    }
}

void LoginDialog::onSelectChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->btnEditServer->setEnabled(current->data(4).toBool());
    ui->btnDeleteServer->setEnabled(current->data(4).toBool());
}
