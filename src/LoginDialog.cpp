#include "LoginDialog.h"
#include "Addnasdialog.h"
#include "DetailDialog.h"
#include "ui_LoginDialog.h"

#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QtNetwork/QUdpSocket>

enum ServerRole { ServerRoleName = Qt::UserRole + 1, ServerRoleFullHost, ServerRoleStored };

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    hashServer = getStoredServer();

    // 初始化列表：显示已保存的服务器信息
    for (auto it = hashServer.constBegin(); it != hashServer.constEnd(); ++it) {
        QString name = it.key();
        QString fullHost = it.value();

        QListWidgetItem *item = new QListWidgetItem(name);
        item->setSizeHint(QSize(80, 40));
        item->setData(ServerRoleName, name);
        item->setData(ServerRoleFullHost, fullHost);
        item->setData(ServerRoleStored, true);
        item->setToolTip(fullHost);

        ui->listWidget->addItem(item);
    }

    // 默认启用搜索
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

// 初始化UDP监听，绑定端口并连接readyRead信号
void LoginDialog::getServerDatagram()
{
    if (udpSocket.state() == QAbstractSocket::BoundState) {
        udpSocket.close(); // 避免重复绑定
    }

    if (!udpSocket.bind(bindPort)) {
        QMessageBox::critical(this, tr("错误"), tr("无法绑定UdpSocket，请更改绑定端口后重试"));
        ui->btnSearchServer->setText(tr("搜索(&S)"));
        ui->btnSearchServer->setChecked(false);
        ui->progressBar->setVisible(false);

        disconnect(&udpSocket, &QUdpSocket::readyRead, this, &LoginDialog::onReadyReadDatagram);
        return;
    }

    connect(&udpSocket,
            &QUdpSocket::readyRead,
            this,
            &LoginDialog::onReadyReadDatagram,
            Qt::UniqueConnection);
}

// 获取本地配置文件中保存的服务器信息
QHash<QString, QString> LoginDialog::getStoredServer()
{
    QSettings &settings = IniSettings::getGlobalSettingsInstance();
    settings.beginGroup("server");

    QHash<QString, QString> servers;
    for (const QString &key : settings.allKeys()) {
        servers.insert(key, settings.value(key).toString());
    }

    settings.endGroup();
    return servers;
}

// 保存服务器信息到配置文件
void LoginDialog::storeServerInfo(const QString &name, const QString &fullhost)
{
    QSettings &settings = IniSettings::getGlobalSettingsInstance();
    settings.beginGroup("server");
    settings.setValue(name, fullhost);
    settings.endGroup();
}

void LoginDialog::deleteServerInfo(const QString &name)
{
    QSettings &settings = IniSettings::getGlobalSettingsInstance();
    settings.beginGroup("server");
    settings.remove(name);
    settings.endGroup();
}

// 接收UDP广播数据
void LoginDialog::onReadyReadDatagram()
{
    while (udpSocket.hasPendingDatagrams()) {
        QByteArray byteArray;
        byteArray.resize(udpSocket.pendingDatagramSize());

        QHostAddress hostAddress;
        udpSocket.readDatagram(byteArray.data(), byteArray.size(), &hostAddress);

        QString res(byteArray);
        QString ipString = QHostAddress(hostAddress.toIPv4Address()).toString();

        QJsonDocument jDocument = QJsonDocument::fromJson(res.toUtf8());

        if (jDocument.isNull() || jDocument.isEmpty())
            return;

        QJsonObject obj = jDocument.object();

        if (!obj.contains("port") || !obj.contains("name"))
            return;

        QMap<QString, QVariant> map = obj.toVariantMap();

        QString fullHost = ipString + ":" + map.value("port").toString();

        QString name = map.value("name").toString() + "-" + fullHost;

        bool exists = hashServer.contains(name) && hashServer.value(name) == fullHost;
        if (!exists) {
            QListWidgetItem *item = new QListWidgetItem(name);
            item->setSizeHint(QSize(100, 40));
            item->setData(ServerRoleName, name);
            item->setData(ServerRoleFullHost, fullHost);
            item->setData(ServerRoleStored, false);
            item->setToolTip(fullHost);

            ui->listWidget->addItem(item);

            hashServer.insert(name, fullHost);
        }
    }
}

// 启动或停止服务器搜索
void LoginDialog::on_btnSearchServer_clicked(bool checked)
{
    if (checked) {
        getServerDatagram();
        ui->btnSearchServer->setText(tr("停止搜索(&Q)"));
    } else {
        disconnect(&udpSocket, &QUdpSocket::readyRead, this, &LoginDialog::onReadyReadDatagram);
        udpSocket.close();
        ui->btnSearchServer->setText(tr("搜索(&S)"));
    }
    ui->progressBar->setVisible(checked);
}

// 设置绑定端口
void LoginDialog::on_btnChangePort_clicked()
{
    int value = QInputDialog::getInt(this, tr("设置端口"), tr("端口号:"), bindPort, 0, 65535);
    if (value > 0) {
        bindPort = value;
    }
}

// 切换选择项时启用/禁用编辑和删除按钮
void LoginDialog::onSelectChanged(QListWidgetItem *current, QListWidgetItem * /* previous */)
{
    bool isStored = current && current->data(ServerRoleStored).toBool();
    ui->btnEditServer->setEnabled(isStored);
    ui->btnDeleteServer->setEnabled(isStored);
}

// 手动添加服务器
void LoginDialog::on_btnAddServer_clicked()
{
    AddNASDialog addNasDialog;
    if (addNasDialog.exec() == QDialog::Accepted) {
        QString name = addNasDialog.getName();
        QString ip = addNasDialog.getIpAddress();
        quint16 port = addNasDialog.getPort();

        if (name.isEmpty() || ip.isEmpty()) {
            QMessageBox::critical(this, tr("错误"), tr("输入不能为空！"));
            return;
        }

        QString fullHost = ip + ":" + QString::number(port);
        QString displayName = name;

        if (hashServer.contains(displayName)) {
            QMessageBox::warning(this, tr("警告"), tr("该名称已存在，请更换名称。"));
            return;
        }

        QListWidgetItem *item = new QListWidgetItem(displayName);
        item->setSizeHint(QSize(100, 40));
        item->setData(ServerRoleName, displayName);
        item->setData(ServerRoleFullHost, fullHost);
        item->setData(ServerRoleStored, true);
        item->setToolTip(fullHost);

        ui->listWidget->addItem(item);
        hashServer.insert(displayName, fullHost);
        storeServerInfo(displayName, fullHost);
    }
}

void LoginDialog::on_btnDeleteServer_clicked()
{
    QListWidgetItem *item = ui->listWidget->currentItem();

    if (item->data(ServerRoleStored).toBool()) {
        QString name = item->text();
        ui->listWidget->takeItem(ui->listWidget->row(item));

        deleteServerInfo(name);
    }
}

void LoginDialog::on_btnConnect_clicked() {}

void LoginDialog::on_btnDetail_clicked()
{
    QListWidgetItem *currentItem = ui->listWidget->currentItem();
    DetailDialog detail(this, DetailDialog::detail);

    QString fullHost = currentItem->data(ServerRoleFullHost).toString();

    qint16 port = fullHost.mid(fullHost.indexOf(":") + 1).toInt();
    QString host = fullHost.mid(0, fullHost.indexOf(":"));

    port = (port == -1) ? 21358 : port;
    detail.setIp(host);

    detail.setName(currentItem->data(ServerRoleName).toString());
    detail.setIp(host);
    detail.setPort(port);

    detail.exec();
}

void LoginDialog::on_btnEditServer_clicked()
{
    QListWidgetItem *currentItem = ui->listWidget->currentItem();
    DetailDialog detail(this, DetailDialog::edit);

    QString fullHost = currentItem->data(ServerRoleFullHost).toString();

    qint16 port = fullHost.mid(fullHost.indexOf(":") + 1).toInt();
    QString host = fullHost.mid(0, fullHost.indexOf(":"));

    port = (port == -1) ? 21358 : port;
    detail.setIp(host);

    detail.setName(currentItem->data(ServerRoleName).toString());
    detail.setIp(host);
    detail.setPort(port);

    QString lastName = currentItem->data(ServerRoleName).toString();

    if (detail.exec() == QDialog::Accepted) {
        QString name = detail.getName();
        QString ip = detail.getIp();
        quint16 port = detail.getPort();

        if (name.isEmpty() || ip.isEmpty()) {
            QMessageBox::critical(this, tr("错误"), tr("输入不能为空！"));
            return;
        }

        QString fullHost = ip + ":" + QString::number(port);
        QString displayName = name;

        currentItem->setText(displayName);
        currentItem->setData(ServerRoleName, displayName);
        currentItem->setData(ServerRoleFullHost, fullHost);
        currentItem->setData(ServerRoleStored, true);
        currentItem->setToolTip(fullHost);

        hashServer.remove(lastName);
        hashServer.insert(name, fullHost);

        deleteServerInfo(lastName);
        storeServerInfo(displayName, fullHost);
    }
}
