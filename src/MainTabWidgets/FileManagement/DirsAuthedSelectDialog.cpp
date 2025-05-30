#include "DirsAuthedSelectDialog.h"
#include "ui_DirsAuthedSelectDialog.h"

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTreeWidgetItem>

DirsAuthedSelectDialog::DirsAuthedSelectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DirsAuthedSelectDialog)
{
    ui->setupUi(this);

    iniUi();
    getAuthedDirs();
}

DirsAuthedSelectDialog::~DirsAuthedSelectDialog()
{
    delete ui;
}

QString DirsAuthedSelectDialog::getSelectedPath() const
{
    return selectedPath;
}

void DirsAuthedSelectDialog::getAuthedDirs()
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASFILEAUTHEDDIRSAPI),
                                            ApiRequest::GET,
                                            this);

    apiRequest->sendRequest();
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    QJsonArray arr = doc.array();
                    for (QVariant var : arr.toVariantList()) {
                        QMap<QString, QVariant> map = var.toMap();
                        QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);

                        treeItem->setText(0, map["path"].toString());
                        treeItem->setText(1, map["permissions"].toString());
                        treeItem->setIcon(0, QIcon(":/file/resource/favicon/folder.svg"));
                    }
                    ui->treeWidget->resizeColumnToContents(0);
                }
            });
}

void DirsAuthedSelectDialog::iniUi()
{
    connect(ui->treeWidget,
            &QTreeWidget::currentItemChanged,
            this,
            [this](QTreeWidgetItem *current, QTreeWidgetItem *previous) {
                selectedPath = current->text(0);
            });
    connect(ui->treeWidget,
            &QTreeWidget::itemDoubleClicked,
            this,
            [this](QTreeWidgetItem *item, int column) { accept(); });
}
