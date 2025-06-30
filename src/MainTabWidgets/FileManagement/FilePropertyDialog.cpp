#include "FilePropertyDialog.h"
#include "ui_FilePropertyDialog.h"

FilePropertyDialog::FilePropertyDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FilePropertyDialog)
{
    ui->setupUi(this);
}

FilePropertyDialog::~FilePropertyDialog()
{
    delete ui;
}
