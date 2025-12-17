#include "DirSelectDialog.h"
#include "ui_DirSelectDialog.h"

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QTreeWidgetItem>

DirSelectDialog::DirSelectDialog(QString initialDir, QWidget *parent)
    : QDialog(parent)
    , selectedDir(initialDir)
    , ui(new Ui::DirSelectDialog)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);

    if (!selectedDir.isEmpty()) {
        ui->lineEdit->setText(selectedDir);
    }

    fetchDirectory();
}

DirSelectDialog::~DirSelectDialog()
{
    delete ui;
}

void DirSelectDialog::fetchDirectory(QString path, QTreeWidgetItem *parentItem)
{
    if (onRequestList.contains(path))
        return;
    if (parentItem)
        onRequestList.append(path);

    ui->progressBar->setVisible(true);

    ApiRequest *req = new ApiRequest(getFullApiPath(FULLHOST, NASUSERDIRTREE), ApiRequest::GET);
    req->addQueryParam("path", path);

    connect(req,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    QJsonDocument dataDoc = req->getData(rawContent);
                    QJsonArray dataArr = dataDoc.array();

                    foreach (const QJsonValue &dataVar, dataArr) {
                        QJsonObject dataObj = dataVar.toObject();
                        QString name = dataObj.value("name").toString();
                        QString path = dataObj.value("path").toString();
                        bool hasChild = dataObj.value("childCount").toInt(0) > 0;

                        if (!parentItem) {
                            //如果是根节点
                            QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);
                            treeItem->setText(0, name);
                            treeItem->setData(0, Qt::UserRole, path);
                            treeItem->setIcon(0, QIcon(":/file/resource/favicon/folder.svg"));
                            if (hasChild) {
                                treeItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                            }
                        } else {
                            //如果是子节点
                            QTreeWidgetItem *treeItem = new QTreeWidgetItem();
                            treeItem->setText(0, name);
                            treeItem->setData(0, Qt::UserRole, path);
                            treeItem->setIcon(0, QIcon(":/file/resource/favicon/folder.svg"));
                            if (hasChild) {
                                treeItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                            }
                            parentItem->addChild(treeItem);
                        }
                    }
                } else {
                    QMessageBox::critical(this,
                                          tr("失败"),
                                          tr("获取目录列表失败！\n") + rawContent,
                                          tr("确定"));
                }

                ui->progressBar->setVisible(false);
            });

    req->sendRequest();
}

void DirSelectDialog::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    selectedDir = item->data(0, Qt::UserRole).toString();
    ui->lineEdit->setText(selectedDir);
}

QString DirSelectDialog::getSelectedDir() const
{
    return selectedDir;
}

void DirSelectDialog::on_treeWidget_itemExpanded(QTreeWidgetItem *item)
{
    if (item->childCount() == 0) {
        fetchDirectory(item->data(0, Qt::UserRole).toString(), item);
    }
}
