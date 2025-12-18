#include "UserDirPermissionDialog.h"
#include "ui_UserDirPermissionDialog.h"

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QMessageBox>
#include <QTreeWidgetItem>

#include <FilePermissionManagmentDialog.h>
UserDirPermissionDialog::UserDirPermissionDialog(QString userName, QWidget *parent)
    : QWidget(parent)
    , currentUserName(userName)
    , ui(new Ui::UserDirPermissionDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("%1 的目录权限").arg(userName));
    setAttribute(Qt::WA_DeleteOnClose);

    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    loadData();
}

UserDirPermissionDialog::~UserDirPermissionDialog()
{
    delete ui;
}

void UserDirPermissionDialog::loadData()
{
    ui->treeWidget->clear();

    int itemCount = ui->treeWidget->topLevelItemCount();

    for (int i = 0; i < itemCount; ++i) {
        QTreeWidgetItem *item = ui->treeWidget->takeTopLevelItem(0);
        delete item;
    }

    ApiRequest *request = new ApiRequest(getFullApiPath(FULLHOST, NASUSERALLRESOURCES),
                                         ApiRequest::GET,
                                         this);
    request->addQueryParam("userName", currentUserName);
    connect(request,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    QJsonDocument dataDoc = request->getData(rawContent);

                    QJsonArray dataArr = dataDoc.array();

                    foreach (const QJsonValue &dataValue, dataArr) {
                        QJsonObject dataObj = dataValue.toObject();
                        QString ownerName = dataObj.value("ownerName").toString();
                        QString folderName = dataObj.value("folderName").toString();
                        QJsonArray permissions = dataObj.value("types").toArray();

                        QString permissionList;
                        foreach (const QJsonValue &permValue, permissions) {
                            permissionList += permValue.toString() + ", ";
                        }
                        if (permissionList.endsWith(", ")) {
                            permissionList.chop(2);
                        }
                        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
                        item->setText(0, folderName);
                        item->setIcon(0, QIcon(":/file/resource/favicon/folder.svg"));
                        item->setData(0, Qt::UserRole, folderName);
                        item->setText(1, permissionList);
                        item->setData(1, Qt::UserRole, permissions.toVariantList());

                        ui->treeWidget->addTopLevelItem(item);
                    }
                } else {
                    QMessageBox::warning(this,
                                         tr("错误"),
                                         tr("无法加载用户目录权限: %1")
                                             .arg(request->getErrorMessage(rawContent)));
                }
            });
    request->sendRequest();
}

void UserDirPermissionDialog::on_pushButtonModifyDir_clicked()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (item) {
        QString folderName = item->data(0, Qt::UserRole).toString();
        QVariantList currentPermissions = item->data(1, Qt::UserRole).toList();
        FilePermissionManagmentDialog dialog(folderName, currentPermissions, currentUserName, this);

        if (dialog.exec() == QDialog::Accepted) {
            loadData();
        }
    }
}

void UserDirPermissionDialog::on_pushButtonNewDir_clicked()
{
    FilePermissionManagmentDialog dialog("", {}, currentUserName, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadData();
    }
}

void UserDirPermissionDialog::on_pushButtonDeleteDir_clicked()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (item) {
        ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST,
                                                               NASDELETEUSERDIRPERMISSIONS),
                                                ApiRequest::DELETE,
                                                this);
        QString folderName = item->data(0, Qt::UserRole).toString();
        apiRequest->addQueryParam("userName", currentUserName);
        apiRequest->addQueryParam("resourcePath", folderName);
        connect(apiRequest,
                &ApiRequest::responseRecieved,
                this,
                [=](QString &rawContent, bool hasError, qint16 statusCode) {
                    if (statusCode == 200) {
                        loadData();
                    } else {
                        QMessageBox::critical(this, tr("失败"), tr("删除目录权限失败！"));
                    }
                });
        apiRequest->sendRequest();
    }
}

void UserDirPermissionDialog::on_treeWidget_currentItemChanged(QTreeWidgetItem *current,
                                                               QTreeWidgetItem *previous)
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    bool hasItem = item;
    ui->pushButtonModifyDir->setEnabled(hasItem);
    ui->pushButtonDeleteDir->setEnabled(hasItem);
}
