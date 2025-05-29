#ifndef APIURL_H
#define APIURL_H

#include <QString>
const QString NASSTATUSAPI = "/api/status/status";
const QString NASLOGINAPI = "/api/auth/login";
const QString NASINIADMIN = "/api/auth/iniAdmin";
const QString NASPERMISSIONAPI = "/api/common/permissions";

const QString NASFILEDIRLISTAPI = "/api/file/info/getList";
const QString NASFILEAUTHEDDIRSAPI = "/api/filePermission/getAuthedDirs";
const QString NASDIELETEFILEAPI = "/api/file/op/delete";

const QString NASCOPYAPI = "/api/file/op/copy";
const QString NASMOVEAPI = "/api/file/op/move";

const QString NASRENAMEFILEAPI = "/api/file/op/rename";

const QString NASRESOURCESTATUSSOCKET = "/ws/overview";

QString getFullApiPath(QString host, QString api);
#endif // APIURL_H
