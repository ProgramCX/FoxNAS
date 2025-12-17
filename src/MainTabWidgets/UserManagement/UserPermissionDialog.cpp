#include "UserPermissionDialog.h"
#include "ui_UserPermissionDialog.h"

#include <QMessageBox>
#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
UserPermissionDialog::UserPermissionDialog(QString userName, QWidget *parent)
    : QDialog(parent)
    , userName(userName)
    , ui(new Ui::UserPermissionDialog)
{
    ui->setupUi(this);
    iniUi();
    getData();
}

UserPermissionDialog::~UserPermissionDialog()
{
    delete ui;
}

void UserPermissionDialog::iniUi()
{
    setWindowTitle(tr("%1 的系统权限列表").arg(userName));
    connect(ui->treeWidget,
            &QTreeWidget::currentItemChanged,
            this,
            [this](QTreeWidgetItem *current, QTreeWidgetItem *previous) {
                if (current) {
                    selectedPermission = current->text(1);
                    currentItem = current;
                }
                updateButton();
            });
    connect(ui->pushButtonChange, &QPushButton::clicked, this, [this] {
        if (currentItem->data(2, Qt::UserRole).toBool()) {
            revokePermission();
        } else {
            grantPermission();
        }
    });
}

void UserPermissionDialog::getData()
{
    auto apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASUSERALLPERMISSIONS),
                                     ApiRequest::GET,
                                     this);

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, apiRequest](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    allPermissionsList.clear();
                    grantedPermissions.clear();
                    ui->treeWidget->clear();

                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    QJsonArray arr = doc.array();
                    for (auto var : arr.toVariantList()) {
                        auto map = var.toMap();
                        auto item = new QTreeWidgetItem(ui->treeWidget);
                        allPermissionsList.append(map["name"].toString());
                        item->setText(0, map["description"].toString());
                        item->setText(1, map["name"].toString());
                    }
                    ui->treeWidget->resizeColumnToContents(0);

                    auto request = new ApiRequest(getFullApiPath(FULLHOST, NASUSERPERMISSIONSLIST),
                                                  ApiRequest::GET,
                                                  this);
                    request->addQueryParam("userName", userName);
                    connect(request,
                            &ApiRequest::responseRecieved,
                            this,
                            [this, request](QString &rawContent, bool hasError, qint16 statusCode) {
                                if (statusCode == 200) {
                                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                                    QJsonArray arr = doc.array();
                                    for (auto var : arr.toVariantList()) {
                                        auto map = var.toMap();
                                        QString permissionName = map["areaName"].toString();
                                        grantedPermissions.append(permissionName);
                                    }
                                    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
                                        auto item = ui->treeWidget->topLevelItem(i);
                                        bool granted = grantedPermissions.contains(item->text(1));
                                        item->setData(2, Qt::UserRole, granted);
                                        item->setText(2, granted ? tr("是") : tr("否"));
                                    }
                                }
                                request->deleteLater();
                            });
                    request->sendRequest();
                }
                apiRequest->deleteLater();
            });

    apiRequest->sendRequest();
}

void UserPermissionDialog::updateButton()
{
    if (selectedPermission.isEmpty()) {
        ui->pushButtonChange->setEnabled(false);
        ui->pushButtonChange->setText(tr("更改权限(&C)"));
    } else {
        ui->pushButtonChange->setEnabled(true);
        if (currentItem) {
            if (currentItem->data(2, Qt::UserRole).toBool()) {
                ui->pushButtonChange->setText(tr("撤销权限(&R)"));
            } else {
                ui->pushButtonChange->setText(tr("授予权限(&G)"));
            }
        }
    }
}

void UserPermissionDialog::grantPermission()
{
    //获取权限代码
    QString areaName = currentItem->text(1);
    auto apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASUSERGRANTPERMISSION),
                                     ApiRequest::PUT,
                                     this);
    apiRequest->addQueryParam("userName", userName);
    apiRequest->addQueryParam("areaName", areaName);
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, apiRequest, areaName](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    getData();
                    QMessageBox::information(this,
                                             tr("成功"),
                                             tr("授予 %1 权限成功！").arg(areaName),
                                             tr("确定"));
                } else {
                    QMessageBox::critical(this,
                                          tr("失败"),
                                          tr("授予 %1 权限失败").arg(areaName),
                                          tr("确定"));
                }
            });
    apiRequest->sendRequest();
}

void UserPermissionDialog::revokePermission()
{
    //获取权限代码
    QString areaName = currentItem->text(1);
    auto apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASUSERREVOKEPERMISSION),
                                     ApiRequest::PUT,
                                     this);
    apiRequest->addQueryParam("userName", userName);
    apiRequest->addQueryParam("areaName", areaName);
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, apiRequest, areaName](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    getData();
                    QMessageBox::information(this,
                                             tr("成功"),
                                             tr("撤销 %1 权限成功！").arg(areaName),
                                             tr("确定"));
                } else {
                    QMessageBox::critical(this,
                                          tr("失败"),
                                          tr("撤销 %1 权限失败").arg(areaName),
                                          tr("确定"));
                }
            });
    apiRequest->sendRequest();
}
