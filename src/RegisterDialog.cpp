#include "RegisterDialog.h"
#include "LoadingDialog.h"
#include "ui_RegisterDialog.h"

#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include <ApiRequest.h>
#include <ApiUrl.h>

RegisterDialog::RegisterDialog(const QString &fullHost, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    this->fullHost = fullHost;

    this->setWindowTitle(tr("用户注册 - ") + fullHost);

    connect(ui->btnSendCode, &QPushButton::clicked, this, &RegisterDialog::on_btnSendCode_clicked);
    connect(ui->btnRegister, &QPushButton::clicked, this, &RegisterDialog::on_btnRegister_clicked);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_btnSendCode_clicked()
{
    if (isSubmitting) {
        return;
    }
    sendVerifyCode();
}

void RegisterDialog::on_btnRegister_clicked()
{
    if (isSubmitting) {
        return;
    }
    registerUser();
}

void RegisterDialog::on_btnCancel_clicked()
{
    this->reject();
}

void RegisterDialog::sendVerifyCode()
{
    QString email = ui->lineEditEmail->text().trimmed();

    if (email.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入邮箱地址！"));
        return;
    }

    // 简单的邮箱格式验证
    if (!email.contains("@") || !email.contains(".")) {
        QMessageBox::warning(this, tr("警告"), tr("请输入有效的邮箱地址！"));
        return;
    }

    isSubmitting = true;
    ui->btnSendCode->setEnabled(false);

    QString fullApi = getFullApiPath(this->fullHost, NASSENDVERIFYCODE);

    QJsonObject object;
    object["emailAddr"] = email;

    QJsonDocument doc(object);

    ApiRequest *apiRequest = new ApiRequest(fullApi, ApiRequest::POST, doc, this);

    LoadingDialog *loadingDialog = new LoadingDialog(tr("正在发送验证码..."), this);
    apiRequest->sendRequest();
    loadingDialog->show();

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                loadingDialog->close();
                loadingDialog->deleteLater();
                isSubmitting = false;

                if (statusCode == 200) {
                    QMessageBox::information(this, tr("成功"), tr("验证码已发送到您的邮箱！"));

                    // 禁用发送按钮60秒
                    ui->btnSendCode->setEnabled(false);
                    int countdown = 60;
                    QTimer *timer = new QTimer(this);
                    connect(timer, &QTimer::timeout, this, [this, timer, countdown]() mutable {
                        countdown--;
                        if (countdown <= 0) {
                            timer->stop();
                            timer->deleteLater();
                            ui->btnSendCode->setEnabled(true);
                            ui->btnSendCode->setText(tr("发送验证码"));
                        } else {
                            ui->btnSendCode->setText(tr("重新发送(%1)").arg(countdown));
                        }
                    });
                    timer->start(1000);
                } else {
                    QString errorMsg = apiRequest->getErrorMessage(rawContent);
                    if (errorMsg.isEmpty()) {
                        errorMsg = tr("未知错误");
                    }
                    QMessageBox::critical(this, tr("失败"), tr("发送验证码失败：") + errorMsg);
                    ui->btnSendCode->setEnabled(true);
                }

                apiRequest->deleteLater();
            });
}

void RegisterDialog::registerUser()
{
    QString email = ui->lineEditEmail->text().trimmed();
    QString password = ui->lineEditPassword->text();
    QString confirmPassword = ui->lineEditConfirmPassword->text();
    QString code = ui->lineEditCode->text().trimmed();

    // 验证输入
    if (email.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入邮箱地址！"));
        return;
    }

    if (!email.contains("@") || !email.contains(".")) {
        QMessageBox::warning(this, tr("警告"), tr("请输入有效的邮箱地址！"));
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入密码！"));
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, tr("警告"), tr("密码长度至少6位！"));
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, tr("警告"), tr("两次输入的密码不一致！"));
        return;
    }

    if (code.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入验证码！"));
        return;
    }

    isSubmitting = true;
    ui->btnRegister->setEnabled(false);

    QString fullApi = getFullApiPath(this->fullHost, NASREGISTER);

    QJsonObject object;
    // 使用邮箱作为用户名
    object["username"] = email;
    object["password"] = password;
    object["code"] = code;

    QJsonDocument doc(object);

    ApiRequest *apiRequest = new ApiRequest(fullApi, ApiRequest::POST, doc, this);

    LoadingDialog *loadingDialog = new LoadingDialog(tr("正在注册..."), this);
    apiRequest->sendRequest();
    loadingDialog->show();

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                loadingDialog->close();
                loadingDialog->deleteLater();
                isSubmitting = false;

                if (statusCode == 200) {
                    emit registerSuccess(email, password);
                    QTimer::singleShot(0, this, &RegisterDialog::accept);
                } else {
                    QString errorMsg = apiRequest->getErrorMessage(rawContent);
                    if (errorMsg.isEmpty()) {
                        errorMsg = tr("未知错误");
                    }
                    QMessageBox::critical(this, tr("失败"), tr("注册失败：") + errorMsg);
                    ui->btnRegister->setEnabled(true);
                }

                apiRequest->deleteLater();
            });
}
