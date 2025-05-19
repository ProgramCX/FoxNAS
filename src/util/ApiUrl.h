#ifndef APIURL_H
#define APIURL_H

#include <QString>
const QString NASSTATUSAPI = "/api/status/status";
const QString NASLOGINAPI = "/api/auth/login";
const QString NASINIADMIN = "/api/auth/iniAdmin";
const QString NASPERMISSIONAPI = "/api/common/permissions";
QString getFullApiPath(QString host, QString api);
#endif // APIURL_H
