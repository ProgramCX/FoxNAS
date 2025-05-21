#include "OverviewDiskItem.h"
#include "ui_OverviewDiskItem.h"

OverviewDiskItem::OverviewDiskItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OverviewDiskItem)
{
    ui->setupUi(this);
}

OverviewDiskItem::~OverviewDiskItem()
{
    delete ui;
}

void OverviewDiskItem::setProgressbar(qint64 used, qint64 total)
{
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(used);
}

void OverviewDiskItem::setDiskName(QString diskName)
{
    ui->labelName->setText(diskName);
}

void OverviewDiskItem::setDiskCondition(QString usedText, QString totalText)
{
    ui->labelCondition->setText(QString("%1 / %2").arg(usedText, totalText));
}
