#include "UserManagementForm.h"
#include "ui_UserManagementForm.h"

UserManagementForm::UserManagementForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserManagementForm)
{
    ui->setupUi(this);
}

UserManagementForm::~UserManagementForm()
{
    delete ui;
}
