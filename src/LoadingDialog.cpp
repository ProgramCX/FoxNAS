#include "LoadingDialog.h"
#include "ui_LoadingDialog.h"

LoadingDialog::LoadingDialog(QString textShow, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoadingDialog)
    , text(textShow)
{
    ui->setupUi(this);
    ui->labelInfo->setText(textShow);
    this->setWindowTitle(textShow);
    // setAttribute(Qt::WA_DeleteOnClose);
}

LoadingDialog::~LoadingDialog()
{
    delete ui;
}

void LoadingDialog::setInfoText(QString &text)
{
    this->text = text;
    ui->labelInfo->setText(text);
}

QString LoadingDialog::getInfoText()
{
    return ui->labelInfo->text();
}

qint64 LoadingDialog::getTotal() const
{
    return total;
}

void LoadingDialog::setTotal(qint64 newTotal)
{
    total = newTotal;
    ui->progressBar->setMaximum(total);
    if (newTotal > 0) {
        ui->progressBar->setTextVisible(true);
    }
}

qint64 LoadingDialog::getNow() const
{
    return now;
}

void LoadingDialog::setNow(qint64 newNow)
{
    now = newNow;
    ui->progressBar->setValue(now);
}
