#ifndef APIREQUEST_H
#define APIREQUEST_H

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
class ApiRequest : public QObject
{
    Q_OBJECT
public:
    enum METHOD { GET = 0, POST };
    ApiRequest(QString apiAddress,
               METHOD httpMethod,
               QJsonDocument requestBody,
               QObject *parent = nullptr);
    ApiRequest(QString apiAddress, METHOD httpMethod, QObject *parent = nullptr);

private:
    QString api;
    QJsonDocument body;
    METHOD method;
    bool hasError = false;

    QNetworkAccessManager manager;
    QNetworkReply *reply;

private:
    QString getToken();

public:
    void sendRequest();

signals:
    void responseRecieved(QString &rawContent, bool hasError, qint16 statusCode);
};
#endif // APIREQUEST_H
