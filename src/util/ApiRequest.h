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
    enum METHOD { GET = 0, POST, DELETE, PUT, PATCH };
    ApiRequest(QString apiAddress,
               METHOD httpMethod,
               QJsonDocument requestBody,
               QObject *parent = nullptr);
    ApiRequest(QString apiAddress, METHOD httpMethod, QObject *parent = nullptr);

private:
    QString api;
    QUrl urlApi;
    QJsonDocument body;
    METHOD method;
    bool hasError = false;

    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QNetworkRequest request;

private:
    QString getToken();

public:
    void sendRequest();
    bool refreshToken();
    void loginAgain(qint16 statusCode);

    QString getApi() const;
    void setApi(const QString &newApi);
    void addQueryParam(QString name, QString value);
    void setUrlQuery(QUrlQuery &query);

    QJsonDocument getData(QString rawContent);
    QString getErrorMessage(QString rawContent);

signals:
    void responseRecieved(QString &rawContent, bool hasError, qint16 statusCode);
};
#endif // APIREQUEST_H
