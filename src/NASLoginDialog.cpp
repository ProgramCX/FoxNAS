#include "NASLoginDialog.h"
#include "LoadingDialog.h"
#include "MainWindow.h"
#include "MemStore.h"
#include "RegisterDialog.h"
#include "ResetPasswordDialog.h"
#include "RetrieveUsernameDialog.h"
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
    ui->checkBox->setChecked(settings.value("Secret/remember").toBool());

    connect(ui->buttonLogin, &QPushButton::clicked, this, &NASLoginDialog::login);
    connect(ui->buttonRegister,
            &QPushButton::clicked,
            this,
            &NASLoginDialog::on_buttonRegister_clicked);
    connect(ui->buttonForgotPassword,
            &QPushButton::clicked,
            this,
            &NASLoginDialog::on_buttonForgotPassword_clicked);
    connect(ui->buttonForgotUsername,
            &QPushButton::clicked,
            this,
            &NASLoginDialog::on_buttonForgotUsername_clicked);
}

NASLoginDialog::~NASLoginDialog()
{
    delete ui;
}

void NASLoginDialog::setUserNameAndPassword(const QString &userName, const QString &password)
{
    ui->lineEditUserName->setText(userName);
    ui->lineEditPassword->setText(password);
    // 注册成功后自动登录
    login();
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
                    // QMessageBox::information(this,
                    //                          "登录成功！",
                    //                          "登录成功，token为\n" + rawContent,
                    //                          QMessageBox::Ok);
                    //解析响应体
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    QJsonObject obj = doc.object();
                    NASTOKEN = obj["accessToken"].toString(); //存储token
                    REFRESHTOKEN = obj["refreshToken"].toString();
                    FULLHOST = this->fullHost;
                    USERNAME = userName;

                    // 从 accessToken 中解析 UUID (JWT 的 sub 字段)
                    QString accessToken = obj["accessToken"].toString();
                    QStringList parts = accessToken.split('.');
                    if (parts.size() >= 2) {
                        QString payloadBase64 = parts[1];
                        payloadBase64.replace('-', '+');
                        payloadBase64.replace('_', '/');
                        int mod4 = payloadBase64.length() % 4;
                        if (mod4 > 0) {
                            payloadBase64.append(QString(4 - mod4, '='));
                        }
                        QByteArray payload = QByteArray::fromBase64(payloadBase64.toUtf8());
                        QJsonDocument payloadDoc = QJsonDocument::fromJson(payload);
                        if (payloadDoc.isObject()) {
                            QJsonObject payloadObj = payloadDoc.object();
                            if (payloadObj.contains("sub")) {
                                USERUUID = payloadObj["sub"].toString();
                            }
                        }
                    }
                    qDebug() << "解析出的 USERUUID:" << USERUUID;

                    QSettings &settings = IniSettings::getGlobalSettingsInstance();
                    settings.setValue("Secret/remember", ui->checkBox->isChecked());

                    if (ui->checkBox->isChecked()) {
                        settings.setValue("Secret/userName", userName);
                        settings.setValue("Secret/password", password);
                    }

                    MainWindow *mainWindow = new MainWindow;
                    mainWindow->show();

                    logined = true;
                    this->close();

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

void NASLoginDialog::on_buttonShowPassword_clicked(bool checked)
{
    if (checked) {
        ui->lineEditPassword->setEchoMode(QLineEdit::Normal);
    } else {
        ui->lineEditPassword->setEchoMode(QLineEdit::Password);
    }
}

void NASLoginDialog::on_buttonRegister_clicked()
{
    RegisterDialog registerDialog(fullHost, this);
    connect(&registerDialog,
            &RegisterDialog::registerSuccess,
            this,
            &NASLoginDialog::setUserNameAndPassword);
    registerDialog.exec();
}

void NASLoginDialog::on_buttonForgotPassword_clicked()
{
    ResetPasswordDialog resetPasswordDialog(fullHost, this);
    resetPasswordDialog.exec();
}

void NASLoginDialog::on_buttonForgotUsername_clicked()
{
    RetrieveUsernameDialog retrieveUsernameDialog(fullHost, this);
    retrieveUsernameDialog.exec();
}
