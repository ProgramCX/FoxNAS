#include "ResetPasswordDialog.h"
#include "LoadingDialog.h"
#include "ui_ResetPasswordDialog.h"

#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include <ApiRequest.h>
#include <ApiUrl.h>

ResetPasswordDialog::ResetPasswordDialog(const QString &fullHost, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResetPasswordDialog)
{
    ui->setupUi(this);
    this->fullHost = fullHost;

    this->setWindowTitle(tr("找回密码 - ") + fullHost);

    // 初始化隐藏第二步
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->btnSendCode, &QPushButton::clicked, this, &ResetPasswordDialog::on_btnSendCode_clicked);
    connect(ui->btnSendCode2, &QPushButton::clicked, this, &ResetPasswordDialog::on_btnSendCode_clicked);
    connect(ui->btnResetPassword, &QPushButton::clicked, this, &ResetPasswordDialog::on_btnResetPassword_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &ResetPasswordDialog::on_btnCancel_clicked);
}

ResetPasswordDialog::~ResetPasswordDialog()
{
    delete ui;
}

void ResetPasswordDialog::on_btnSendCode_clicked()
{
    if (isSubmitting) {
        return;
    }
    validateEmail();
}

void ResetPasswordDialog::on_btnResetPassword_clicked()
{
    if (isSubmitting) {
        return;
    }
    resetPassword();
}

void ResetPasswordDialog::on_btnCancel_clicked()
{
    this->reject();
}

void ResetPasswordDialog::validateEmail()
{
    QString email = ui->lineEditEmail->text().trimmed();

    if (email.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入邮箱地址！"));
        return;
    }

    if (!email.contains("@") || !email.contains(".")) {
        QMessageBox::warning(this, tr("警告"), tr("请输入有效的邮箱地址！"));
        return;
    }

    sendVerifyCode();
}

void ResetPasswordDialog::sendVerifyCode()
{
    QString email = ui->lineEditEmail->text().trimmed();

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

                if (statusCode == 200 || statusCode == 429) {
                    if (statusCode == 429) {
                        QMessageBox::warning(this, tr("提示"), rawContent);
                    } else {
                        QMessageBox::information(this, tr("成功"), tr("验证码已发送到您的邮箱！"));
                    }

                    currentStep = 2;
                    ui->stackedWidget->setCurrentIndex(1);
                    ui->lineEditCode->setEnabled(true);
                    ui->lineEditCode->setFocus();

                    ui->btnSendCode->setEnabled(false);
                    ui->btnSendCode2->setEnabled(false);

                    int countdown = 60;
                    QTimer *timer = new QTimer(this);
                    connect(timer, &QTimer::timeout, this, [this, timer, countdown]() mutable {
                        static int currentCount = countdown;
                        currentCount--;
                        if (currentCount <= 0) {
                            timer->stop();
                            timer->deleteLater();
                            ui->btnSendCode->setEnabled(true);
                            ui->btnSendCode2->setEnabled(true);
                            ui->btnSendCode->setText(tr("发送验证码"));
                            ui->btnSendCode2->setText(tr("发送验证码"));
                            currentCount = countdown;
                        } else {
                            ui->btnSendCode->setText(tr("重新发送(%1)").arg(currentCount));
                            ui->btnSendCode2->setText(tr("重新发送(%1)").arg(currentCount));
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

void ResetPasswordDialog::resetPassword()
{
    QString email = ui->lineEditEmail->text().trimmed();
    QString code = ui->lineEditCode->text().trimmed();
    QString password = ui->lineEditNewPassword->text();
    QString confirmPassword = ui->lineEditConfirmPassword->text();

    // 验证输入
    if (code.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入验证码！"));
        return;
    }

    if (code.length() != 6) {
        QMessageBox::warning(this, tr("警告"), tr("验证码长度为6位！"));
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入新密码！"));
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

    isSubmitting = true;
    ui->btnResetPassword->setEnabled(false);

    QString fullApi = getFullApiPath(this->fullHost, NASRESETPASSWORD);

    QJsonObject object;
    object["emailAddr"] = email;
    object["code"] = code;
    object["newPassword"] = password;

    QJsonDocument doc(object);

    ApiRequest *apiRequest = new ApiRequest(fullApi, ApiRequest::POST, doc, this);

    LoadingDialog *loadingDialog = new LoadingDialog(tr("正在重置密码..."), this);
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
                    QMessageBox::information(this, tr("成功"), tr("密码重置成功！请使用新密码登录。"));
                    this->accept();
                } else {
                    QString errorMsg = apiRequest->getErrorMessage(rawContent);
                    if (errorMsg.isEmpty()) {
                        errorMsg = tr("未知错误");
                    }
                    QMessageBox::critical(this, tr("失败"), tr("重置密码失败：") + errorMsg);
                    ui->btnResetPassword->setEnabled(true);
                }

                apiRequest->deleteLater();
            });
}
