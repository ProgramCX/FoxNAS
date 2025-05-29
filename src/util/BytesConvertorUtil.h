#ifndef BYTESCONVERTORUTIL_H
#define BYTESCONVERTORUTIL_H

#include <QObject>

class BytesConvertorUtil
{
public:
    static BytesConvertorUtil& getInstance();

    QPair<double, QString> getReasonaleDataUnit(double bytes);
    double getDataAccordingUnit(double bytes, QString unit);

private:
    explicit BytesConvertorUtil();
};

#endif // BYTESCONVERTORUTIL_H
