#include "NASLoginDialog.h"
#include "LoadingDialog.h"
#include "ui_NASLoginDialog.h"

#include <QMessageBox>

#include <QJsonDocument>
#include <QJsonObject>
#include <ApiRequest.h>
#include <ApiUrl.h>
#include <IniSettings.hpp>

NASLoginDialog::NASLoginDialog(const QString &fullHost, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NASLoginDialog)
{
    ui->setupUi(this);
    this->fullHost = fullHost;

    this->setWindowTitle(tr("登录到") + fullHost);

    QSettings &settings = IniSettings::getGlobalSettingsInstance();
    ui->lineEditUserName->setText(settings.value("Secret/userName").toString());
    ui->lineEditPassword->setText(settings.value("Secret/password").toString());

    connect(ui->buttonLogin, &QPushButton::clicked, this, &NASLoginDialog::login);
}

NASLoginDialog::~NASLoginDialog()
{
    delete ui;
}

void NASLoginDialog::login()
{
    QString fullHost = getFullApiPath(this->fullHost, NASLOGINAPI);
    QString userName = ui->lineEditUserName->text();
    QString password = ui->lineEditPassword->text();

    //构造JSON
    QJsonObject object;
    object["username"] = userName;
    object["password"] = password;

    QJsonDocument doc(object);

    ApiRequest *apiRequest = new ApiRequest(fullHost, ApiRequest::POST, doc, this);
    LoadingDialog *loadingDialog = new LoadingDialog(tr("正在登录到") + fullHost + "...");
    apiRequest->sendRequest();
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                loadingDialog->close();
                if (statusCode == 200) {
                    QMessageBox::information(this,
                                             "登录成功！",
                                             "登录成功，token为\n" + rawContent,
                                             QMessageBox::Ok);
                    //存储token
                    QSettings &settings = IniSettings::getGlobalSettingsInstance();
                    settings.setValue("Secret/token", rawContent);

                    if (ui->checkBox->isChecked()) {
                        settings.setValue("Secret/userName", userName);
                        settings.setValue("Secret/password", password);
                    }

                } else if (statusCode == 401) {
                    QMessageBox::critical(this,
                                          tr("登录失败"),
                                          tr("用户名或密码错误！"),
                                          QMessageBox::Ok);
                } else if (statusCode == 403) {
                    QMessageBox::critical(this,
                                          tr("登录失败"),
                                          tr("密码错误次数过多，请稍后再试！"),
                                          QMessageBox::Ok);
                } else {
                    QMessageBox::critical(this, tr("登录失败"), rawContent, QMessageBox::Ok);
                }
            });
    loadingDialog->exec();
}
