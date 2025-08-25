#include "DDNSForm.h"
#include "ui_DDNSForm.h"

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

DDNSForm::DDNSForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DDNSForm)
{
    ui->setupUi(this);

    model = new QStandardItemModel;

    iniTableViewStructure();
    fetchData(1);

    statusTimer = new QTimer;
    statusTimer->start(10000);

    connect(statusTimer, &QTimer::timeout, this, &DDNSForm::updateStatus);

    connect(ui->tableView->itemDelegate(),
            &QAbstractItemDelegate::commitData,
            this,
            [=](QWidget *editor) {
                QModelIndex index = ui->tableView->currentIndex();
                updateTask(index.row());
            });
}

DDNSForm::~DDNSForm()
{
    delete ui;
}

void DDNSForm::iniTableViewStructure()
{
    model->setHorizontalHeaderLabels({tr("任务ID"),
                                      tr("任务名称"),
                                      tr("任务描述"),
                                      tr("IP地址"),
                                      tr("域名RR记录"),
                                      tr("主域名"),
                                      tr("关联的访问密钥"),
                                      tr("同步间隔（秒）"),
                                      tr("任务状态"),
                                      tr("公网IP"),
                                      tr("IP类型"),
                                      tr("实时状态"),
                                      tr("操作")});

    ui->tableView->setModel(model);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    int lastCol = model->columnCount() - 1;
    ui->tableView->horizontalHeader()->setSectionResizeMode(lastCol, QHeaderView::Stretch);
}

void DDNSForm::loadData(const QString &jsonString)
{
    model->removeRows(0, model->rowCount());

    // 解析 JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isObject())
        return;

    QJsonObject root = doc.object();
    QJsonArray records = root["records"].toArray();

    currentPage = root["current"].toInt();
    totalPages = root["total"].toInt();

    model->setRowCount(records.size());

    for (int row = 0; row < records.size(); ++row) {
        QJsonObject record = records[row].toObject();

        // 任务ID
        QStandardItem *itemId = new QStandardItem(QString::number(record["id"].toInt()));
        itemId->setFlags(itemId->flags() & ~Qt::ItemIsEditable); // 不可编辑
        model->setItem(row, 0, itemId);

        // 任务名称
        model->setItem(row, 1, new QStandardItem(record["taskName"].toString()));

        // 任务描述
        model->setItem(row, 2, new QStandardItem(record["taskDescription"].toString()));

        // IP 地址
        model->setItem(row, 3, new QStandardItem(record["taskIp"].toString()));

        // 域名RR记录
        model->setItem(row, 4, new QStandardItem(record["domainRr"].toString()));

        // 主域名
        model->setItem(row, 5, new QStandardItem(record["mainDomain"].toString()));

        // 关联的访问密钥 (下拉框)
        QStandardItem *itemSecret = new QStandardItem();
        itemSecret->setFlags(itemId->flags() & ~Qt::ItemIsEditable);
        itemSecret->setData(record["dnsSecretId"].toInt(), Qt::UserRole);
        itemSecret->setData(tr("查看或编辑"), Qt::DisplayRole);
        model->setItem(row, 6, itemSecret);

        // 同步间隔
        QStandardItem *itemInterval = new QStandardItem();
        itemInterval->setData(record["syncInterval"].toInt(), Qt::EditRole);
        model->setItem(row, 7, itemInterval);

        // 任务状态 (下拉框)
        QStandardItem *itemStatus = new QStandardItem();
        int statusValue = record["status"].toInt();
        QStringList statusOptions = {tr("停用"), tr("启用")};

        itemStatus->setData(statusValue, Qt::EditRole);
        itemStatus->setData(statusOptions[statusValue], Qt::DisplayRole);
        model->setItem(row, 8, itemStatus);

        // 公网IP (复选框)
        QStandardItem *itemPubIp = new QStandardItem();
        itemPubIp->setFlags(itemId->flags() & ~Qt::ItemIsEditable);
        itemPubIp->setCheckable(true);
        itemPubIp->setCheckState(record["isPublicIp"].toInt() ? Qt::Checked : Qt::Unchecked);
        model->setItem(row, 9, itemPubIp);

        // IP类型
        QStandardItem *ipTypeItem = new QStandardItem(record["ipType"].toString().toUpper());
        model->setItem(row, 10, ipTypeItem);

        QStandardItem *statusItem = new QStandardItem();
        statusItem->setFlags(itemId->flags() & ~Qt::ItemIsEditable);
        statusItem->setData(tr("正在获取实时状态..."), Qt::DisplayRole);
        model->setItem(row, 11, statusItem);

        model->setItem(row, 12, new QStandardItem());
    }

    // 下拉框列
    QStringList statusOptions = {tr("停用"), tr("启用")};
    ComboBoxDelegate *statusDelegate = new ComboBoxDelegate(statusOptions, this);
    ui->tableView->setItemDelegateForColumn(8, statusDelegate);

    QStringList ipOptions = {tr("IPV4"), tr("IPV6")};
    ComboBoxDelegate *ipOptionsDelegate = new ComboBoxDelegate(ipOptions, this);
    ui->tableView->setItemDelegateForColumn(10, ipOptionsDelegate);

    // 按钮列
    ButtonDelegate *btnSecretDelegate = new ButtonDelegate(this);
    ui->tableView->setItemDelegateForColumn(6, btnSecretDelegate);

    QStringList btns = {tr("删除")};
    MultiButtonDelegate *btnDelegate = new MultiButtonDelegate(btns, this);
    ui->tableView->setItemDelegateForColumn(12, btnDelegate);

    connect(btnDelegate,
            &MultiButtonDelegate::buttonClicked,
            this,
            [=](const QModelIndex &index, const QString &btn) {
                qDebug() << "行:" << index.row() << "按钮:" << btn;

                int row = index.row();
                QModelIndex idIndex = model->index(row, 0);
                int id = model->data(idIndex, Qt::DisplayRole).toInt();
                if (btn == tr("暂停")) {
                    pauseTask(id);
                } else if (btn == tr("恢复")) {
                    resumeTask(id);
                } else if (btn == tr("重启")) {
                    restartTask(id);
                } else if (btn == tr("删除")) {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(this,
                                                  tr("确认删除"),
                                                  tr("确定要删除该任务吗？"),
                                                  QMessageBox::Yes | QMessageBox::No);

                    if (reply == QMessageBox::Yes) {
                        deleteTask(id, row);
                    }
                }
            });

    ui->label->setText(tr("第 %1 页，共 %2 页").arg(currentPage).arg(totalPages));
}

void DDNSForm::fetchData(int page)
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASDDNSTASKLIST),
                                            ApiRequest::GET);

    apiRequest->addQueryParam("page", QString::number(page));
    apiRequest->addQueryParam("size", QString::number(10));

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (!hasError && statusCode == 200) {
                    loadData(rawContent);
                    updateStatus();
                } else {
                    QMessageBox::critical(nullptr, "失败", "获取DDNS列表失败！", tr("确定"));
                }
                delete apiRequest;
            });

    apiRequest->sendRequest();
}

void DDNSForm::updateStatus()
{
    int rowCount = model->rowCount();

    if (rowCount == 0) {
        return;
    }
    QJsonArray idList;
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex indexId = model->index(row, 0);
        int taskId = model->data(indexId, Qt::DisplayRole).toInt();
        idList.append(taskId);
    }

    QJsonDocument doc(idList);

    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASDDNSSTATUS),
                                            ApiRequest::GET,
                                            doc);

    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (!hasError && statusCode == 200) {
                    QJsonDocument doc = QJsonDocument::fromJson(rawContent.toUtf8());
                    QJsonArray jsonArray = doc.array();

                    for (const QJsonValue &val : jsonArray) {
                        if (!val.isObject())
                            continue;

                        QJsonObject obj = val.toObject();
                        int id = obj["id"].toInt();
                        QString status = obj["status"].toString();

                        QString display;
                        QBrush brush;

                        QStringList newBtns;
                        if (status == "running") {
                            display = " • 运行中";
                            brush = QBrush(QColor(76, 175, 80)); // 绿色
                            newBtns.append(tr("暂停"));
                            newBtns.append(tr("重启"));
                        } else if (status == "paused") {
                            display = " ‖ 已暂停";
                            brush = QBrush(QColor(255, 193, 7)); // 黄色
                            newBtns.append(tr("恢复"));
                            newBtns.append(tr("重启"));
                        } else if (status == "completed") {
                            display = " ✓ 已完成";
                            brush = QBrush(QColor(33, 150, 243)); // 蓝色
                            newBtns.append(tr("重启"));
                        } else if (status == "blocked") {
                            display = " … 阻塞中";
                            brush = QBrush(QColor(158, 158, 158)); // 灰色
                            newBtns.append(tr("暂停"));
                            newBtns.append(tr("重启"));
                        } else if (status == "error") {
                            display = " ✕ 错误";
                            brush = QBrush(QColor(244, 67, 54)); // 红色
                            newBtns.append(tr("重启"));
                        } else {
                            display = "未知";
                            brush = QBrush(QColor(97, 97, 97)); // 灰色
                        }

                        int row = findRowById(id);
                        QStandardItem *statusItem = model->item(row, 11);
                        if (statusItem) {
                            statusItem->setText(display);
                            statusItem->setForeground(brush);
                            statusItem->setData(status, Qt::UserRole);

                            QAbstractItemDelegate *delegate = ui->tableView->itemDelegateForColumn(
                                12);
                            MultiButtonDelegate *btnDelegate = qobject_cast<MultiButtonDelegate *>(
                                delegate);

                            newBtns.append(tr("删除"));
                            if (btnDelegate) {
                                btnDelegate->setButtons(newBtns);
                            }
                        }
                    }
                }
                delete apiRequest;
            });

    apiRequest->sendRequest();
}

void DDNSForm::pauseTask(int id)
{
    changeTaskStatus(id, NASDDNSPAUSE, tr("暂停任务失败！id为 %1").arg(id));
}

void DDNSForm::restartTask(int id)
{
    changeTaskStatus(id, NASDDNSRESTART, tr("重启任务失败！id为 %1").arg(id));
}

void DDNSForm::resumeTask(int id)
{
    changeTaskStatus(id, NASDDNSRESUME, tr("恢复任务失败！id为 %1").arg(id));
}

void DDNSForm::deleteTask(int id, int row)
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASDDNSDELETE),
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

void DDNSForm::createTask(int id) {}

void DDNSForm::updateTask(int row)
{
    QJsonObject record;

    record["id"] = model->item(row, 0)->text().toInt();
    record["taskName"] = model->item(row, 1)->text();
    record["taskDescription"] = model->item(row, 2)->text();
    record["taskIp"] = model->item(row, 3)->text();
    record["domainRr"] = model->item(row, 4)->text();
    record["mainDomain"] = model->item(row, 5)->text();

    record["dnsSecretId"] = model->item(row, 6)->data(Qt::UserRole).toInt();

    record["syncInterval"] = model->item(row, 7)->data(Qt::EditRole).toInt();

    int status = model->item(row, 8)->data(Qt::EditRole).toInt();
    record["status"] = status;

    record["isPublicIp"] = (model->item(row, 9)->checkState() == Qt::Checked) ? 1 : 0;

    record["ipType"] = model->item(row, 10)->text().toLower();

    record["lastFailed"] = false;

    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASDDNSUPDATE),
                                            ApiRequest::PUT,
                                            QJsonDocument(record));
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(nullptr, "失败", tr("更新DDNS记录失败！"), tr("确定"));
                }

                fetchData(currentPage);
                delete apiRequest;
            });
    apiRequest->sendRequest();
}

void DDNSForm::changeTaskStatus(int id, QString api, QString failDesc)
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, api), ApiRequest::PUT);
    apiRequest->addQueryParam("id", QString::number(id));
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(nullptr, "失败", failDesc, tr("确定"));
                } else {
                    updateStatus();
                }
                delete apiRequest;
            });

    apiRequest->sendRequest();
}

int DDNSForm::findRowById(int id)
{
    int rowCount = model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QStandardItem *item = model->item(row, 0);
        if (!item)
            continue;

        if (item->text().toInt() == id) {
            return row;
        }
    }
    return -1;
}
