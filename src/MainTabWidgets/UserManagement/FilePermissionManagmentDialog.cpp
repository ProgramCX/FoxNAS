#include "FilePermissionManagmentDialog.h"
#include "ui_FilePermissionManagmentDialog.h"

#include <DirSelectDialog.h>
FilePermissionManagmentDialog::FilePermissionManagmentDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FilePermissionManagmentDialog)
{
    ui->setupUi(this);
}

FilePermissionManagmentDialog::~FilePermissionManagmentDialog()
{
    delete ui;
}

void FilePermissionManagmentDialog::on_pushButtonSelect_clicked()
{
    DirSelectDialog dir(ui->lineEdit->text());
    dir.exec();
}
