#include "SettingsForm.h"
#include "ui_SettingsForm.h"

SettingsForm::SettingsForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsForm)
{
    ui->setupUi(this);
}

SettingsForm::~SettingsForm()
{
    delete ui;
}
