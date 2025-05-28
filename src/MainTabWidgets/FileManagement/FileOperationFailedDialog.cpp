#include "FileOperationFailedDialog.h"
#include "ui_FileOperationFailedDialog.h"

FileOperationFailedDialog::FileOperationFailedDialog(qint64 total,
                                                     qint64 failed,
                                                     QString message,
                                                     QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileOperationFailedDialog)
{
    ui->setupUi(this);

    QString messages("%1 总共 %2 个，%3 个失败，是否重试？");
    ui->labelMessage->setText(
        messages.arg(message, QString::number(total), QString::number(failed)));
}

FileOperationFailedDialog::~FileOperationFailedDialog()
{
    delete ui;
}

void FileOperationFailedDialog::addFileLists(QList<QString> fileList)
{
    list = fileList;
    for (const QString &file : fileList) {
        QListWidgetItem *item = new QListWidgetItem(file, ui->listWidget);
        item->setCheckState(Qt::Checked);
        ui->listWidget->addItem(item);
    }
}

void FileOperationFailedDialog::addFileLists(QList<QMap<QString, QString>> fileList)
{
    mapList = fileList;
    for (const QMap<QString, QString> &file : fileList) {
        QListWidgetItem *item = new QListWidgetItem(file["oldPath"] + " -> " + file["newPath"],
                                                    ui->listWidget);
        item->setCheckState(Qt::Checked);
        ui->listWidget->addItem(item);
    }
}

QList<QString> FileOperationFailedDialog::getResult()
{
    QList<QString> selectedPaths;
    for (int i = 0; i < ui->listWidget->count(); i++) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (item->checkState() == Qt::Checked || item->isSelected()) {
            selectedPaths.append(list[i]);
        }
    }
    return selectedPaths;
}

QList<QMap<QString, QString>> FileOperationFailedDialog::getMapResult()
{
    QList<QMap<QString, QString>> selectedPaths;
    for (int i = 0; i < ui->listWidget->count(); i++) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (item->checkState() == Qt::Checked || item->isSelected()) {
            selectedPaths.append(mapList[i]);
        }
    }
    return selectedPaths;
}
