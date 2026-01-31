#include "ApiRequest.h"
#include "../NASLoginDialog.h"
#include "../LoginDialog.h"

#include "MemStore.h"
#include "ApiUrl.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QMessageBox>
#include <QUrlQuery>
#include <IniSettings.hpp>
#include <MemStore.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

QJsonDocument ApiRequest::getData(QString rawContent)
{
    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());

    if (doc.isNull()) {
        qDebug() << "无法解析API响应为JSON";
        return QJsonDocument();
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("data")) {
            QJsonValue dataValue = obj.value("data");

            if (dataValue.isObject()) {
                return QJsonDocument(dataValue.toObject());
            } else if (dataValue.isArray()) {
                return QJsonDocument(dataValue.toArray());
            }
        }
    }

    return QJsonDocument();
}

QString ApiRequest::getErrorMessage(QString rawContent)
{
    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());

    if (doc.isNull()) {
        qDebug() << "无法解析API响应为JSON";
        return rawContent;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("message")) {
            QJsonValue dataValue = obj.value("message");

            if (dataValue.isString()) {
                return dataValue.toString();
            } else {
                return rawContent;
            }
        }
    }

    return rawContent;
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
        reply = manager.get(request, body.toJson());
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
        if (response.isEmpty() && hasError) {
            response = reply->errorString();
        }

        qint16 statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        // 401 未授权，尝试使用 refresh token 刷新
        if (statusCode == 401 && !REFRESHTOKEN.isEmpty()) {
            if (refreshToken()) {
                // Token 刷新成功，重新发送请求
                qDebug() << "Token 刷新成功，正在重新发送请求...";
                sendRequest();
                return;
            }
        }
        
        emit responseRecieved(response, hasError, statusCode);
    });
}

bool ApiRequest::refreshToken()
{
    qDebug() << "开始刷新 Token...";

    if (FULLHOST.isEmpty() || REFRESHTOKEN.isEmpty() || USERUUID.isEmpty()) {
        qDebug() << "Token 刷新失败: FULLHOST 或 REFRESHTOKEN 或 USERUUID 为空";
        qDebug() << "FULLHOST:" << FULLHOST;
        qDebug() << "REFRESHTOKEN:" << REFRESHTOKEN;
        qDebug() << "USERUUID:" << USERUUID;
        loginAgain(401);
        return false;
    }

    QUrl url(getFullApiPath(FULLHOST, NASREFRESHTOKEN));
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject jsonBody;
    jsonBody["refreshToken"] = REFRESHTOKEN;
    jsonBody["uuid"] = USERUUID;

    QJsonDocument jsonDoc(jsonBody);

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request, jsonDoc.toJson());

    QEventLoop eventLoop;
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QByteArray responseData = reply->readAll();
    qint16 statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() == QNetworkReply::NoError && statusCode == 200) {
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        if (responseDoc.isObject()) {
            QJsonObject responseObj = responseDoc.object();

            if (responseObj.contains("accessToken") && responseObj.contains("refreshToken")) {
                NASTOKEN = responseObj["accessToken"].toString();
                REFRESHTOKEN = responseObj["refreshToken"].toString();

                qDebug() << "Token 刷新成功！";
                qDebug() << "新的 AccessToken:" << NASTOKEN;
                qDebug() << "新的 RefreshToken:" << REFRESHTOKEN;

                reply->deleteLater();
                return true;
            }
        }
        qDebug() << "Token 刷新响应格式错误:" << responseData;
    } else {
        qDebug() << "Token 刷新失败，状态码:" << statusCode << "错误:" << reply->errorString();
    }

    reply->deleteLater();

    qDebug() << "Token 刷新失败，需要重新登录";
    loginAgain(statusCode);
    return false;
}

void ApiRequest::loginAgain(qint16 statusCode)
{
    // 清除 Token 信息
    NASTOKEN.clear();
    REFRESHTOKEN.clear();
    USERUUID.clear();
    FULLHOST.clear();
    USERNAME.clear();

    QString message;
    switch (statusCode) {
    case 400:
        message = tr("请求错误，请重新登录");
        break;
    case 401:
        message = tr("登录已过期，请重新登录");
        break;
    case 403:
        message = tr("没有权限，请重新登录");
        break;
    default:
        message = tr("连接失败，请重新登录");
        break;
    }

    QMessageBox::warning(nullptr, tr("需要重新登录"), message);

    // 关闭所有窗口
    QApplication::closeAllWindows();

    // 显示广播发现对话框
    LoginDialog *loginDialog = new LoginDialog();
    loginDialog->setAttribute(Qt::WA_DeleteOnClose);
    loginDialog->show();
}
