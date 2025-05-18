#include "ApiRequest.h"
#include <QDebug>
#include <IniSettings.hpp>
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
    QSettings &setting = IniSettings::getGlobalSettingsInstance();
    return setting.value("Secret/token").toString();
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
