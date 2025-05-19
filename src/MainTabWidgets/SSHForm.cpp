#include "SSHForm.h"
#include "ui_SSHForm.h"

SSHForm::SSHForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SSHForm)
{
    ui->setupUi(this);
}

SSHForm::~SSHForm()
{
    delete ui;
}
