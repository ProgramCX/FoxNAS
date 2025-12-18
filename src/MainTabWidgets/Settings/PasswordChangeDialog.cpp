#include "PasswordChangeDialog.h"
#include "ui_PasswordChangeDialog.h"

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QMessageBox>
PasswordChangeDialog::PasswordChangeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PasswordChangeDialog)
{
    ui->setupUi(this);
    ui->lineEditOld->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->lineEditNew_3->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->lineEditNewConfirm_3->setEchoMode(QLineEdit::PasswordEchoOnEdit);
}

PasswordChangeDialog::~PasswordChangeDialog()
{
    delete ui;
}

void PasswordChangeDialog::on_pushButtonOK_5_clicked()
{
    if (ui->lineEditNew_3->text() != ui->lineEditNewConfirm_3->text()) {
        QMessageBox::warning(this, tr("错误"), tr("两次输入的新密码不一致！"));
        return;
    }
    ApiRequest *request = new ApiRequest(getFullApiPath(FULLHOST, NASSELFCHANGEPASSWORD),
                                         ApiRequest::PUT,
                                         this);
    request->addQueryParam("oldPassword", ui->lineEditOld->text());
    request->addQueryParam("newPassword", ui->lineEditNew_3->text());
    connect(request,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    QMessageBox::information(this, tr("成功"), tr("密码修改成功！"));
                    accept();
                } else {
                    QMessageBox::critical(this,
                                          tr("失败"),
                                          tr("密码修改失败：%1")
                                              .arg(request->getErrorMessage(rawContent)));
                }
            });

    request->sendRequest();
}
