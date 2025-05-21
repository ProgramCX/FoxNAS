#ifndef OVERVIEWDISKITEM_H
#define OVERVIEWDISKITEM_H

#include <QWidget>

namespace Ui {
class OverviewDiskItem;
}

class OverviewDiskItem : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewDiskItem(QWidget *parent = nullptr);
    ~OverviewDiskItem();
    void setProgressbar(qint64 used, qint64 total);
    void setDiskName(QString diskName);
    void setDiskCondition(QString usedText, QString totalText);

private:
    Ui::OverviewDiskItem *ui;
};

#endif // OVERVIEWDISKITEM_H
