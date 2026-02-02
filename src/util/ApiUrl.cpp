#include <ApiUrl.h>
#include <MemStore.h>
QString getFullApiPath(QString host, QString api)
{
    if (host.contains(":443")) {
        return "https://" + host + api;
    }
    return "http://" + host + api;
}
