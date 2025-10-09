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
const QString NASVideoHeartBeats = "/api/file/media/prolong-token";

const QString NASDDNSTASKLIST = "/api/ddns/tasks/list";
const QString NASDDNSSTATUS = "/api/ddns/tasks/status";
const QString NASDDNSPAUSE = "/api/ddns/tasks/pause";
const QString NASDDNSRESUME = "/api/ddns/tasks/resume";
const QString NASDDNSRESTART = "/api/ddns/tasks/restart";
const QString NASDDNSDELETE = "/api/ddns/tasks/delete";
const QString NASDDNSUPDATE = "/api/ddns/tasks/update";
const QString NASDDNSCREATE = "/api/ddns/tasks/create";

const QString NASDDNSSECRETLIST = "/api/ddns/config/accessKeys";
const QString NASDDNSSECRETCREATE = "/api/ddns/config/addAccessKey";
const QString NASDDNSSECRETREMOVE = "/api/ddns/config/delAccessKey";
const QString NASDDNSSECRETUPDATE = "/api/ddns/config/updateAccessKey";

const QString NASUSERADD = "/api/user/addUser";
const QString NASUSERDELETE = "/api/user/delUser";
const QString NASUSERBLOCK = "/api/user/blockUser";
const QString NASUSERUNBLOCK = "/api/user/unblockUser";
const QString NASUSERLIST = "/api/user/list";
const QString NASUSERCHANGEPASSWORD = "/api/user/changePassword";
const QString NASUSERPERMISSIONSLIST = "/api/user/permissions";
const QString NASUSERGRANTPERMISSION = "/api/user/grantPermission";
const QString NASUSERREVOKEPERMISSION = "/api/user/revokePermission";
const QString NASUSERALLPERMISSIONS = "/api/user/allPermissions";
QString getFullApiPath(QString host, QString api);
#endif // APIURL_H
