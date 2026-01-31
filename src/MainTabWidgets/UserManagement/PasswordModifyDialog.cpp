#include "PasswordModifyDialog.h"
#include "ui_PasswordModifyDialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

PasswordModifyDialog::PasswordModifyDialog(QString uuid, QWidget *parent)
    : QDialog(parent)
    , userUuid(uuid)
    , ui(new Ui::PasswordModifyDialog)
{
    ui->setupUi(this);
    ui->lineEditNew->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->lineEditNewConfirm->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    // setAttribute(Qt::WA_DeleteOnClose);
}

PasswordModifyDialog::~PasswordModifyDialog()
{
    delete ui;
}

QString PasswordModifyDialog::getUuid() const
{
    return userUuid;
}

void PasswordModifyDialog::setUuid(const QString &newUuid)
{
    userUuid = newUuid;
}

bool PasswordModifyDialog::verifyForm()
{
    if (ui->lineEditNew->text() != ui->lineEditNewConfirm->text()) {
        QMessageBox::warning(this, "警告", tr("两次输入的新密码不一致！"), tr("确定"));
        return false;
    }
    return true;
}

void PasswordModifyDialog::on_pushButtonOK_clicked()
{
    if (verifyForm()) {
        changePassword();
    }
}

void PasswordModifyDialog::on_pushButtonOK_2_clicked()
{
    reject();
}

void PasswordModifyDialog::changePassword()
{
    QJsonObject record;
    record["uuid"] = userUuid;
    record["password"] = ui->lineEditNew->text();

    QJsonDocument doc(record);
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASUSERCHANGEPASSWORD),
                                            ApiRequest::PUT,
                                            doc);
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(nullptr,
                                          "失败",
                                          tr("修改用户 UUID 为 %1 的密码失败").arg(userUuid),
                                          tr("确定"));
                } else {
                }
                delete apiRequest;
                accept();
            });

    apiRequest->sendRequest();
}
