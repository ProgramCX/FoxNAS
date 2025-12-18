#include "SettingsForm.h"
#include "ui_SettingsForm.h"

#include <GeneralSettingForm.h>
#include <SelfUserSettingsForm.h>

SettingsForm::SettingsForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsForm)
{
    ui->setupUi(this);
    iniUI();
}

SettingsForm::~SettingsForm()
{
    delete ui;
}

void SettingsForm::iniUI()
{
    SelfUserSettingsForm *selfUserSettingsForm = new SelfUserSettingsForm(ui->tabWidget);
    GeneralSettingForm *generalSettingForm = new GeneralSettingForm(ui->tabWidget);
    ui->tabWidget->addTab(selfUserSettingsForm, tr("我的账号"));
    ui->tabWidget->addTab(generalSettingForm, tr("常用设置"));
}
