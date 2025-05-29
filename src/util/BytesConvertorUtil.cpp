#include "BytesConvertorUtil.h"

BytesConvertorUtil &BytesConvertorUtil::getInstance()
{
    static BytesConvertorUtil instance;
    return instance;
}

QPair<double, QString> BytesConvertorUtil::getReasonaleDataUnit(double bytes)
{
    QPair<double, QString> ret;
    if (bytes > 1024 * 1024 * 1024 * 1024.0) {
        ret.first = bytes / (1024 * 1024 * 1024 * 1024.0);
        ret.second = "TB";
    } else if (bytes > 1024 * 1024 * 1024.0) {
        ret.first = bytes / (1024 * 1024 * 1024.0);
        ret.second = "GB";
    } else if (bytes > 1024 * 1024.0) {
        ret.first = bytes / (1024 * 1024.0);
        ret.second = "MB";
    } else if (bytes > 1024) {
        ret.first = bytes / 1024.0;
        ret.second = "KB";
    } else {
        ret.first = bytes;
        ret.second = "B";
    }
    return ret;
}

double BytesConvertorUtil::getDataAccordingUnit(double bytes, QString unit)
{
    if (unit == "B") {
        return bytes;
    } else if (unit == "KB") {
        return bytes / 1024.0;
    } else if (unit == "MB") {
        return bytes / 1024.0 / 1024;
    } else if (unit == "GB") {
        return bytes / 1024.0 / 1024 / 1024;
    } else if (unit == "TB") {
        return bytes / 1024.0 / 1024 / 1024 / 1024;
    } else {
        return -1;
    }
}

BytesConvertorUtil::BytesConvertorUtil() {}
