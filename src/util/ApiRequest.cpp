#include "ApiRequest.h"
#include "../NASLoginDialog.h"

#include "MemStore.h"

#include <QDebug>
#include <QMessageBox>
#include <QUrlQuery>
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
{
    urlApi.setUrl(apiAddress);
    request.setUrl(urlApi);
}

ApiRequest::ApiRequest(QString apiAddress, METHOD httpMethod, QObject *parent)
    : QObject{parent}
    , api(apiAddress)
    , method(httpMethod)
{
    urlApi.setUrl(apiAddress);
    request.setUrl(QUrl(urlApi.toString(QUrl::FullyEncoded)));
}

QString ApiRequest::getApi() const
{
    return api;
}

void ApiRequest::setApi(const QString &newApi)
{
    api = newApi;
    urlApi.setUrl(api);
    request.setUrl(QUrl(urlApi.toString(QUrl::FullyEncoded)));
}

void ApiRequest::addQueryParam(QString name, QString value)
{
    QUrlQuery query(urlApi.query());

    // 移除已存在的参数
    QList<QPair<QString, QString>> items = query.queryItems();
    for (const auto &item : items) {
        if (item.first == name) {
            query.removeAllQueryItems(name);
            break;
        }
    }

    // 添加新参数
    QByteArray encodedValue = QUrl::toPercentEncoding(value);
    query.addQueryItem(name, QString(encodedValue));

    urlApi.setQuery(query);
    request.setUrl(QUrl(urlApi.toString(QUrl::FullyEncoded)));
}

void ApiRequest::setUrlQuery(QUrlQuery &query)
{
    urlApi.setQuery(query);
    request.setUrl(QUrl(urlApi.toString(QUrl::FullyEncoded)));
}

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

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer " + token).toUtf8());

    qDebug() << "正在发送API请求..\n"
             << "请求token:" << token << "\n请求Url：" << urlApi.toString() << "\n请求体："
             << body.toJson() << "\n";

    if (method == POST) {
        reply = manager.post(request, body.toJson());
    } else if (method == GET) {
        reply = manager.get(request);
    } else if (method == DELETE) {
        reply = manager.sendCustomRequest(request, "DELETE", body.toJson());
    } else if (method == PUT) {
        reply = manager.put(request, body.toJson());
    } else if (method == PATCH) {
        reply = manager.sendCustomRequest(request, "PATCH", body.toJson());
    } else {
        qDebug() << "未知的Http请求方法";
    }

    connect(reply, &QNetworkReply::finished, [&]() {
        QString response;
        if (reply->error() == QNetworkReply::NoError) {
            hasError = false;
        } else {
            hasError = true;
        }

        response = reply->readAll();
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
