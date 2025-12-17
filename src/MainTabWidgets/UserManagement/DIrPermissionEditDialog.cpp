#include "DIrPermissionEditDialog.h"
#include "ui_DIrPermissionEditDialog.h"

DIrPermissionEditDialog::DIrPermissionEditDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DIrPermissionEditDialog)
{
    ui->setupUi(this);
}

DIrPermissionEditDialog::~DIrPermissionEditDialog()
{
    delete ui;
}
