#include <ApiUrl.h>
#include <MemStore.h>
QString getFullApiPath(QString host, QString api)
{
    return "http://" + host + api;
}
