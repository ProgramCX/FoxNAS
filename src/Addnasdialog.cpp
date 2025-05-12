#include "Addnasdialog.h"
#include "ui_Addnasdialog.h"

AddNASDialog::AddNASDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddNASDialog)
{
    ui->setupUi(this);
}

AddNASDialog::~AddNASDialog()
{
    delete ui;
}

QString AddNASDialog::getName()
{
    return ui->lineEditName->text();
}

QString AddNASDialog::getIpAddress()
{
    return ui->lineEditIp->text();
}

quint16 AddNASDialog::getPort()
{
    return ui->spinBox->value();
}
