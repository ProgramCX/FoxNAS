#include <ApiUrl.h>
QString getFullApiPath(QString host, QString api)
{
    return "http://" + host + api;
}
