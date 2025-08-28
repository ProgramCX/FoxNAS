#include "DDNSSecrectSelectDialog.h"
#include "ui_DDNSSecrectSelectDialog.h"

#include <QMessageBox>
#include <DDNSSecretForm.h>
DDNSSecrectSelectDialog::DDNSSecrectSelectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DDNSSecrectSelectDialog)
{
    ui->setupUi(this);
    ddnsSecretForm = new DDNSSecretForm(DDNSSecretForm::SelectMode, ui->widget);
    QVBoxLayout *layout = new QVBoxLayout(ui->widget);
    layout->addWidget(ddnsSecretForm);

    ui->widget->setLayout(layout);
}

DDNSSecrectSelectDialog::~DDNSSecrectSelectDialog()
{
    delete ui;
}

void DDNSSecrectSelectDialog::on_pushButtonOK_clicked()
{
    if (ddnsSecretForm->getSelectedId() == -1) {
        QMessageBox::warning(this, "警告", "请选择密钥！", tr("确定"));
    } else if (ddnsSecretForm->getSelectedId() == 0) {
        QMessageBox::warning(this, "警告", "创建密钥后请保存再选择！", tr("确定"));
    } else {
        selectedId = ddnsSecretForm->getSelectedId();
        accept();
    }
}

int DDNSSecrectSelectDialog::getSelectedId() const
{
    return selectedId;
}

void DDNSSecrectSelectDialog::on_pushButtonCancel_clicked()
{
    reject();
}
