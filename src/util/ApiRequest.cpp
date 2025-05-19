#include "ApiRequest.h"
#include "../NASLoginDialog.h"

#include "MemStore.h"

#include <QDebug>
#include <QMessageBox>
#include <IniSettings.hpp>
#include <MemStore.h>

ApiRequest::ApiRequest(QString apiAddress,
                       METHOD httpMethod,
                       QJsonDocument requestBody,
                       QObject *parent)
    : QObject{parent}
    , api(apiAddress)
    , body(requestBody)
    , method(httpMethod)
{}

ApiRequest::ApiRequest(QString apiAddress, METHOD httpMethod, QObject *parent)
    : QObject{parent}
    , api(apiAddress)
    , method(httpMethod)
{}

QString ApiRequest::getToken()
{
    return NASTOKEN;
}

void ApiRequest::sendRequest()
{
    if (api.isEmpty()) {
        qDebug() << "API 地址为空";
        return;
    }

    QString token = getToken();
    QNetworkRequest request{QUrl(api)};

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer " + token).toUtf8());

    qDebug() << "正在发送API请求..\n"
             << "请求token:" << token << "\n请求Url：" << api << "\n请求体：" << body.toJson()
             << "\n";
    if (method == POST) {
        reply = manager.post(request, body.toJson());
    } else if (method == GET) {
        reply = manager.get(request);
    } else {
        qDebug() << "未知的Http请求方法";
    }

    connect(reply, &QNetworkReply::finished, [&]() {
        QString response;
        if (reply->error() == QNetworkReply::NoError) {
            response = reply->readAll();
            hasError = false;
        } else {
            hasError = true;
        }

        qint16 statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        emit responseRecieved(response, hasError, statusCode);
    });
}

void ApiRequest::loginAgain(qint16 statusCode)
{
    switch (statusCode) {
    case 400: {
        QMessageBox::critical(nullptr, "Bad Request", "API 请求方法不正确！", tr("确定"));

        break;
    }
    case 403: {
        QMessageBox::critical(nullptr, "Forbidden", "没有权限！", tr("确定"));
        break;
    }
    }
    NASLoginDialog dialog(FULLHOST);
    dialog.exec();
}
