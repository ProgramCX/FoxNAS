#include "SelfUserSettingsForm.h"
#include "ui_SelfUserSettingsForm.h"

#include <PasswordChangeDialog.h>

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QMessageBox>
#include "../LoginDialog.h"
SelfUserSettingsForm::SelfUserSettingsForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SelfUserSettingsForm)
{
    ui->setupUi(this);
    iniUi();
}

SelfUserSettingsForm::~SelfUserSettingsForm()
{
    delete ui;
}

void SelfUserSettingsForm::on_pushButtonModifyPsw_clicked()
{
    PasswordChangeDialog d;
    d.exec();
}

void SelfUserSettingsForm::on_lineEditUserName_textChanged(const QString &arg1)
{
    ui->pushButtonModifyName->setEnabled(USERNAME != arg1);
}

void SelfUserSettingsForm::changeUserName()
{
    ApiRequest *request = new ApiRequest(getFullApiPath(FULLHOST, NASSELFCHANGEUSERNAME),
                                         ApiRequest::PUT,
                                         this);
    request->addQueryParam("newUserName", ui->lineEditUserName->text());
    ui->lineEditUserName->setEnabled(false);
    connect(request,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    USERNAME = ui->lineEditUserName->text();
                    QMessageBox::information(this, tr("成功"), tr("用户名修改成功！"));
                    iniUi();
                    on_lineEditUserName_textChanged(ui->lineEditUserName->text());
                } else {
                    QMessageBox::critical(this,
                                          tr("失败"),
                                          tr("用户名修改失败：%1")
                                              .arg(request->getErrorMessage(rawContent)));
                }
                ui->lineEditUserName->setEnabled(true);
            });
    request->sendRequest();
}

void SelfUserSettingsForm::iniUi()
{
    ui->lineEditUserName->setText(USERNAME);
}

void SelfUserSettingsForm::on_pushButtonModifyName_clicked()
{
    changeUserName();
}

void SelfUserSettingsForm::on_pushButton_3_clicked()
{
    LoginDialog *loginDialog = new LoginDialog();
    loginDialog->show();
    close();
}
