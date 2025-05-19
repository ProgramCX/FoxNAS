#include "DDNSForm.h"
#include "ui_DDNSForm.h"

DDNSForm::DDNSForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DDNSForm)
{
    ui->setupUi(this);
}

DDNSForm::~DDNSForm()
{
    delete ui;
}
