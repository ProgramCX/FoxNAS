#include "FileManagementForm.h"
#include "ui_FileManagementForm.h"

FileManagementForm::FileManagementForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileManagementForm)
{
    ui->setupUi(this);
}

FileManagementForm::~FileManagementForm()
{
    delete ui;
}
