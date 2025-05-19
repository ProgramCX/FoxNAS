#include "OverViewForm.h"
#include "ui_OverViewForm.h"

OverViewForm::OverViewForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OverViewForm)
{
    ui->setupUi(this);
}

OverViewForm::~OverViewForm()
{
    delete ui;
}
