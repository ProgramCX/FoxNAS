#include "MainWindow.h"
#include "MainTabWidgets/DDNSForm.h"
#include "MainTabWidgets/FileManagementForm.h"
#include "MainTabWidgets/InfoForm.h"
#include "MainTabWidgets/OverViewForm.h"
#include "MainTabWidgets/SSHForm.h"
#include "MainTabWidgets/SettingsForm.h"
#include "MainTabWidgets/UserManagementForm.h"
#include "ui_MainWindow.h"

#include <ApiUrl.h>
#include <MemStore.h>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->showMaximized();
    setAttribute(Qt::WA_DeleteOnClose);
    getPermissions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getPermissions()
{
    QString fullApi = getFullApiPath(FULLHOST, NASPERMISSIONAPI) + "?username=" + USERNAME;

    apiRequest = new ApiRequest(fullApi, ApiRequest::GET);

    apiRequest->sendRequest();

    connect(apiRequest, &ApiRequest::responseRecieved, this, &MainWindow::iniPermissionUi);
}

void MainWindow::iniPermissionUi(QString &rawContent, bool hasError, qint16 statusCode)
{
    if (statusCode == 200) {
        QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());

        QJsonArray array = doc.array();
        apiRequest->deleteLater();

        QList<QVariant> list = array.toVariantList();

        QStringList stringList;
        for (const auto &item : list) {
            stringList.append(item.toString());
        }
        OverViewForm *overViewForm = new OverViewForm(ui->tabWidget);
        ui->tabWidget->addTab(overViewForm, "系统概览");

        if (stringList.contains("FILE")) {
            FileManagementForm *fileManagementForm = new FileManagementForm(ui->tabWidget);
            ui->tabWidget->addTab(fileManagementForm, "文件管理");
        }

        if (stringList.contains("DDNS")) {
            DDNSForm *ddnsForm = new DDNSForm(ui->tabWidget);
            ui->tabWidget->addTab(ddnsForm, "动态域名解析");
        }

        if (stringList.contains("SSH")) {
            SSHForm *sshForm = new SSHForm(ui->tabWidget);
            ui->tabWidget->addTab(sshForm, "SSH 登录");
        }

        if (stringList.contains("USER")) {
            UserManagementForm *userManageMent = new UserManagementForm(ui->tabWidget);
            ui->tabWidget->addTab(userManageMent, "用户管理");
        }

        SettingsForm *settingsForm = new SettingsForm(ui->tabWidget);
        ui->tabWidget->addTab(settingsForm, "系统设置");

        InfoForm *infoForm = new InfoForm(ui->tabWidget);
        ui->tabWidget->addTab(infoForm, "系统信息");
    } else {
        apiRequest->loginAgain(statusCode);
        this->close();
    }
}
