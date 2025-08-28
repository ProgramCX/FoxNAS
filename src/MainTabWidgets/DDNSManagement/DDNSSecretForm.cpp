#include "DDNSSecretForm.h"
#include "ui_DDNSSecretForm.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTimer>

#include <ButtonDelegate.h>
#include <ComboBoxDelegate.h>
#include <MultiButtonDelegate.h>

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

DDNSSecretForm::DDNSSecretForm(DDNSSecretForm::FormMode formMode, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DDNSSecretForm)
    , currentWidgetMode(formMode)
{
    ui->setupUi(this);

    model = new QStandardItemModel;

    iniUi();
    fetchData(1);

    connect(ui->tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DDNSSecretForm::onSelectionChanged);
}

DDNSSecretForm::~DDNSSecretForm()
{
    delete ui;
}

long DDNSSecretForm::getSelectedId() const
{
    return selectedId;
}

void DDNSSecretForm::setSelectedId(long newSelectedId)
{
    selectedId = newSelectedId;
}

void DDNSSecretForm::iniUi()
{
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->tableView->setSelectionBehavior(currentWidgetMode == ManageMode
                                            ? QAbstractItemView::SelectItems
                                            : QAbstractItemView::SelectRows);

    model->setHorizontalHeaderLabels({tr("密钥ID"),
                                      tr("Key"),
                                      tr("Secret"),
                                      tr("名称"),
                                      tr("描述"),
                                      tr("所属DNS服务商"),
                                      tr("操作")});

    ui->tableView->setModel(model);

    int lastCol = model->columnCount() - 1;
    ui->tableView->horizontalHeader()->setSectionResizeMode(lastCol, QHeaderView::Stretch);
}

void DDNSSecretForm::fetchData(int page)
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASDDNSSECRETLIST),
                                            ApiRequest::GET);

    apiRequest->addQueryParam("page", QString::number(page));
    apiRequest->addQueryParam("size", QString::number(30));

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (!hasError && statusCode == 200) {
                    loadData(rawContent);
                    updateButton();
                } else {
                    QMessageBox::critical(nullptr, "失败", "获取DDNS密钥列表失败！", tr("确定"));
                }
                delete apiRequest;
            });

    apiRequest->sendRequest();
}

void DDNSSecretForm::loadData(const QString &jsonString)
{
    model->removeRows(0, model->rowCount());

    // 解析 JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isObject())
        return;

    QJsonObject root = doc.object();
    QJsonArray records = root["records"].toArray();

    currentPage = root["current"].toInt();
    totalPages = root["pages"].toInt();

    model->setRowCount(records.size());

    for (int row = 0; row < records.size(); ++row) {
        QJsonObject record = records[row].toObject();

        // 密钥ID
        QStandardItem *itemId = new QStandardItem(QString::number(record["id"].toInt()));
        itemId->setFlags(itemId->flags() & ~Qt::ItemIsEditable); // 不可编辑
        model->setItem(row, 0, itemId);

        // Key
        model->setItem(row, 1, new QStandardItem(record["accessKey"].toString()));

        // Secret
        model->setItem(row, 2, new QStandardItem(record["accessSecret"].toString()));

        // 名称
        model->setItem(row, 3, new QStandardItem(record["accessName"].toString()));

        // 描述
        model->setItem(row, 4, new QStandardItem(record["accessDescription"].toString()));

        // 所属DNS服务商

        QStandardItem *ipDNSProvider = new QStandardItem(providerOps[record["dnsCode"].toInt() - 1]);
        model->setItem(row, 5, ipDNSProvider);

        // 操作
        QStandardItem *itemOp = new QStandardItem();
        itemOp->setFlags(itemId->flags() & ~Qt::ItemIsEditable);
        model->setItem(row, 6, itemOp);
    }

    ComboBoxDelegate *providerOpsDelegate = new ComboBoxDelegate(providerOps, this);
    ui->tableView->setItemDelegateForColumn(5, providerOpsDelegate);

    QStringList btns = {tr("删除"), tr("保存")};
    MultiButtonDelegate *btnDelegate = new MultiButtonDelegate(btns, this);
    ui->tableView->setItemDelegateForColumn(6, btnDelegate);

    connect(btnDelegate,
            &MultiButtonDelegate::buttonClicked,
            this,
            [=](const QModelIndex &index, const QString &btn) {
                int row = index.row();
                QModelIndex idIndex = model->index(row, 0);
                int id = model->data(idIndex, Qt::DisplayRole).toInt();
                if (btn == tr("删除")) {
                    if (model->item(row, 0)->text() == "new") {
                        model->removeRow(row);
                        return;
                    }

                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(this,
                                                  tr("确认删除"),
                                                  tr("确定要删除该任务吗？"),
                                                  QMessageBox::Yes | QMessageBox::No);

                    if (reply == QMessageBox::Yes) {
                        deleteTask(id, row);
                    }
                } else if (btn == tr("保存")) {
                    if (model->item(row, 0)->text() == "new")
                        createSecret(row);
                    else
                        updateTask(row);
                }
            });

    ui->label->setText(tr("第 %1 页，共 %2 页").arg(currentPage).arg(totalPages));
}

void DDNSSecretForm::updateButton()
{
    ui->pushButtonFirstPage->setEnabled(currentPage > 1);
    ui->pushButtonTailPage->setEnabled(currentPage != totalPages);
    ui->pushButtonNextPage->setEnabled(currentPage + 1 <= totalPages);
    ui->pushButtonLastPage->setEnabled(currentPage - 1 >= 1);

    ui->spinBoxPage->setMaximum(totalPages);
    ui->spinBoxPage->setMinimum(currentPage > 0 ? 1 : 0);
    ui->pushButtonGoPage->setEnabled(currentPage > 0);
}

void DDNSSecretForm::createSecret(int row)
{
    QJsonObject record;

    record["id"] = 0;
    record["accessKey"] = model->item(row, 1)->text();
    record["accessSecret"] = model->item(row, 2)->text();
    record["accessName"] = model->item(row, 3)->text();
    record["accessDescription"] = model->item(row, 4)->text();

    QStandardItem *providerItem = model->item(row, 5);
    const int index = providerOps.indexOf(providerItem->text());
    record["dnsCode"] = index == -1 ? 1 : index + 1;

    if (model->item(row, 1)->text().isEmpty() || model->item(row, 2)->text().isEmpty()
        || model->item(row, 3)->text().isEmpty() || model->item(row, 4)->text().isEmpty()) {
        QMessageBox::warning(this, "失败", tr("请填写完整表格！"), tr("确定"));
    }
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASDDNSSECRETCREATE),
                                            ApiRequest::POST,
                                            QJsonDocument(record));
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, apiRequest, record, row](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(this, "失败", tr("添加密钥记录失败！"), tr("确定"));
                } else {
                    QMessageBox::information(this,
                                             "成功",
                                             tr("添加密钥记录成功，ID=%1")
                                                 .arg(this->model->item(row, 0)->text().toInt()),
                                             tr("确定"));
                    fetchData(currentPage);
                }

                delete apiRequest;
            });
    apiRequest->sendRequest();
}

void DDNSSecretForm::updateTask(int row)
{
    QJsonObject record;

    record["id"] = model->item(row, 0)->text().toInt();
    record["accessKey"] = model->item(row, 1)->text();
    record["accessSecret"] = model->item(row, 2)->text();
    record["accessName"] = model->item(row, 3)->text();
    record["accessDescription"] = model->item(row, 4)->text();

    QStandardItem *providerItem = model->item(row, 5);
    const int index = providerOps.indexOf(providerItem->text());
    record["dnsCode"] = index == -1 ? 1 : index + 1;

    record["dnsSecretId"] = model->item(row, 6)->data(Qt::UserRole).toInt();

    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASDDNSSECRETUPDATE),
                                            ApiRequest::PUT,
                                            QJsonDocument(record));
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, apiRequest, record, row](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(this, "失败", tr("更新密钥记录失败！"), tr("确定"));
                } else {
                    QMessageBox::information(this,
                                             "成功",
                                             tr("更新密钥记录成功，ID=%1")
                                                 .arg(this->model->item(row, 0)->text().toInt()),
                                             tr("确定"));
                    fetchData(currentPage);
                }

                delete apiRequest;
            });
    apiRequest->sendRequest();
}

void DDNSSecretForm::deleteTask(int id, int row)
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASDDNSSECRETREMOVE),
                                            ApiRequest::DELETE);
    apiRequest->addQueryParam("id", QString::number(id));
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(nullptr,
                                          "失败",
                                          tr("删除ID为 %1 的任务失败").arg(id),
                                          tr("确定"));
                } else {
                    model->removeRow(row);
                }
                delete apiRequest;
            });

    apiRequest->sendRequest();
}

void DDNSSecretForm::addNewRow()
{
    int row = model->rowCount(); // 获取当前行数
    model->insertRow(row);       // 在末尾插入新行

    // 任务ID
    QStandardItem *itemId = new QStandardItem("new");
    itemId->setFlags(itemId->flags() & ~Qt::ItemIsEditable); // 不可编辑
    model->setItem(row, 0, itemId);

    // Key
    model->setItem(row, 1, new QStandardItem(tr("请输入key")));

    // Secret
    model->setItem(row, 2, new QStandardItem(tr("请输入secret")));

    // 名称
    model->setItem(row, 3, new QStandardItem(tr("请输入名称")));

    // 描述
    model->setItem(row, 4, new QStandardItem(tr("请输入名描述")));

    // 所属DNS服务商
    QStandardItem *ipDNSProvider = new QStandardItem("Cloudflare");
    model->setItem(row, 5, ipDNSProvider);

    QStandardItem *itemOp = new QStandardItem();
    itemOp->setFlags(itemId->flags() & ~Qt::ItemIsEditable);
    model->setItem(row, 6, itemOp);
}

void DDNSSecretForm::on_pushButtonFirstPage_clicked()
{
    fetchData(1);
}

void DDNSSecretForm::on_pushButtonLastPage_clicked()
{
    fetchData(currentPage > 1 ? currentPage - 1 : 0);
}

void DDNSSecretForm::on_pushButtonNextPage_clicked()
{
    fetchData(currentPage + 1 > totalPages ? totalPages : currentPage + 1);
}

void DDNSSecretForm::on_pushButtonTailPage_clicked()
{
    if (totalPages > 0) {
        fetchData(totalPages);
    }
}

void DDNSSecretForm::on_pushButtonGoPage_clicked()
{
    fetchData(ui->spinBoxPage->value());
}

void DDNSSecretForm::on_pushButtonNewRecord_clicked()
{
    addNewRow();
}

void DDNSSecretForm::onSelectionChanged(const QItemSelection &selected,
                                        const QItemSelection &deselected)
{
    QModelIndexList selectedRows = selected.indexes();

    if (!selectedRows.isEmpty()) {
        setSelectedId(model->item(selectedRows.first().row(), 0)->text().toInt());
    }
}
