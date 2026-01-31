#include "FilePermissionManagmentDialog.h"
#include "ui_FilePermissionManagmentDialog.h"

#include <QMessageBox>
#include <DirSelectDialog.h>

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>
FilePermissionManagmentDialog::FilePermissionManagmentDialog(QString folderName,
                                                             QVariantList list,
                                                             QString uuid,
                                                             QWidget *parent)
    : QDialog(parent)
    , userUuid(uuid)
    , ui(new Ui::FilePermissionManagmentDialog)
{
    ui->setupUi(this);

    if (folderName.isEmpty()) {
        isNew = true;
        setWindowTitle(tr("添加权限目录"));
    } else {
        isNew = false;
        setWindowTitle(tr("修改权限目录"));
    }

    ui->lineEdit->setText(folderName);
    oldFolderName = folderName;

    foreach (const QVariant &var, list) {
        currentPermissionList.append(var.toString());

        if (var.toString().toLower() == "read") {
            ui->checkBoxRead->setChecked(true);
        } else if (var.toString().toLower() == "write") {
            ui->checkBoxWrite->setChecked(true);
        }
    }
}

FilePermissionManagmentDialog::~FilePermissionManagmentDialog()
{
    delete ui;
}

void FilePermissionManagmentDialog::on_pushButtonSelect_clicked()
{
    DirSelectDialog dir(ui->lineEdit->text());
    if (dir.exec() == QDialog::Accepted) {
        ui->lineEdit->setText(dir.getSelectedDir());
    }
}

void FilePermissionManagmentDialog::on_checkBoxRead_clicked(bool checked)
{
    if (checked) {
        if (!currentPermissionList.contains("Read")) {
            currentPermissionList.append("Read");
        }
    } else {
        currentPermissionList.removeAll("Read");
    }
    qDebug() << currentPermissionList;
}

void FilePermissionManagmentDialog::on_checkBoxWrite_clicked(bool checked)
{
    if (checked) {
        if (!currentPermissionList.contains("Write")) {
            currentPermissionList.append("Write");
        }
    } else {
        currentPermissionList.removeAll("Write");
    }

    qDebug() << currentPermissionList;
}

void FilePermissionManagmentDialog::on_pushButtonCancel_clicked()
{
    reject();
}

void FilePermissionManagmentDialog::on_pushButtonOK_clicked()
{
    if (currentPermissionList.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请至少选择一种权限类型！"));
        return;
    }

    if (ui->lineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("目录不能为空！"));
    }

    QJsonArray permissionArray;
    for (const QString &perm : currentPermissionList) {
        permissionArray.append(perm);
    }

    QJsonDocument requestDoc(permissionArray);

    if (!isNew) {
        ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST,
                                                               NASMODIFYUSERDIRPERMISSIONS),
                                                ApiRequest::PUT,
                                                requestDoc,
                                                this);

        apiRequest->addQueryParam("newResourcePath", ui->lineEdit->text());
        apiRequest->addQueryParam("oldResourcePath", oldFolderName);
        apiRequest->addQueryParam("uuid", userUuid);

        connect(apiRequest,
                &ApiRequest::responseRecieved,
                this,
                [=](QString &rawContent, bool hasError, qint16 statusCode) {
                    if (statusCode == 200) {
                        accept();
                    } else {
                        QMessageBox::critical(this, tr("失败"), tr("修啊目录权限失败！"));
                    }
                });

        apiRequest->sendRequest();
    } else {
        ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST,
                                                               NASCREATEUSERDIRPERMISSIONS),
                                                ApiRequest::POST,
                                                requestDoc,
                                                this);
        apiRequest->addQueryParam("resourcePath", ui->lineEdit->text());
        apiRequest->addQueryParam("uuid", userUuid);
        connect(apiRequest,
                &ApiRequest::responseRecieved,
                this,
                [=](QString &rawContent, bool hasError, qint16 statusCode) {
                    if (statusCode == 200) {
                        accept();
                    } else {
                        QMessageBox::critical(this, tr("失败"), tr("添加目录权限失败！"));
                    }
                });
        apiRequest->sendRequest();
    }
}
