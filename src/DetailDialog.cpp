#include "DetailDialog.h"
#include "ui_DetailDialog.h"

DetailDialog::DetailDialog(QWidget *parent, DetailDialog::MODE mode)
    : QDialog(parent)
    , ui(new Ui::DetailDialog)
{
    ui->setupUi(this);

    if (mode == edit) {
        setWindowTitle("编辑NAS服务器");
        ui->lineEditIP->setEnabled(true);
        ui->lineEditName->setEnabled(true);
        ui->spinBox->setEnabled(true);
    } else {
        setWindowTitle("添加NAS服务器");
        ui->lineEditIP->setEnabled(false);
        ui->lineEditName->setEnabled(false);
        ui->spinBox->setEnabled(false);
    }
}

DetailDialog::~DetailDialog()
{
    delete ui;
}

QString DetailDialog::getName()
{
    return ui->lineEditName->text();
}

QString DetailDialog::getIp()
{
    return ui->lineEditIP->text();
}

quint16 DetailDialog::getPort()
{
    return ui->spinBox->value();
}

void DetailDialog::setName(QString name)
{
    ui->lineEditName->setText(name);
}

void DetailDialog::setIp(QString ip)
{
    ui->lineEditIP->setText(ip);
}

void DetailDialog::setPort(quint16 port)
{
    ui->spinBox->setValue(port);
}

void DetailDialog::on_checkBox_checkStateChanged(const Qt::CheckState &)
{
}
