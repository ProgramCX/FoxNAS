#ifndef APIURL_H
#define APIURL_H

#include <QString>
const QString NASSTATUSAPI = "/api/status/status";
const QString NASLOGINAPI = "/api/auth/login";

QString getFullApiPath(QString host, QString api);
#endif // APIURL_H
