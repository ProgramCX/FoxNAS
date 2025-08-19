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
const QString NASCREATEDIRAPI = "/api/file/op/createDir";

const QString NASRENAMEFILEAPI = "/api/file/op/rename";
const QString NASDOWNLOADFILEAPI = "/api/file/op/get";
const QString NASUPLOADFILEAPI = "/api/file/op/upload";
const QString NASRESOURCESTATUSSOCKET = "/ws/overview";

const QString NASVideoMetaData = "/api/file/media/metadata";
const QString NASVideoRangeStream = "/api/file/media/video-stream";
const QString NASVideoToken = "/api/file/media/validate";
const QString NASMediaType = "/api/file/media/media-type";

QString getFullApiPath(QString host, QString api);
#endif // APIURL_H
