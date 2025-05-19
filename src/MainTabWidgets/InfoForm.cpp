#include "InfoForm.h"
#include "ui_InfoForm.h"

InfoForm::InfoForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InfoForm)
{
    ui->setupUi(this);
}

InfoForm::~InfoForm()
{
    delete ui;
}
