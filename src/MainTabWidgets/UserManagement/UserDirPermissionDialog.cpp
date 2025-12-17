#include "UserDirPermissionDialog.h"
#include "ui_UserDirPermissionDialog.h"

UserDirPermissionDialog::UserDirPermissionDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserDirPermissionDialog)
{
    ui->setupUi(this);
}

UserDirPermissionDialog::~UserDirPermissionDialog()
{
    delete ui;
}
