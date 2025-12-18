#include "GeneralSettingForm.h"
#include "ui_GeneralSettingForm.h"

GeneralSettingForm::GeneralSettingForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GeneralSettingForm)
{
    ui->setupUi(this);
}

GeneralSettingForm::~GeneralSettingForm()
{
    delete ui;
}
