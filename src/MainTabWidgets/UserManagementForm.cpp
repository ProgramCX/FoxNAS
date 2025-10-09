#include "UserManagementForm.h"
#include "ui_UserManagementForm.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardItemModel>

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <ButtonDelegate.h>
#include <ComboBoxDelegate.h>
#include <MemStore.h>
#include <MultiButtonDelegate.h>
UserManagementForm::UserManagementForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserManagementForm)
{
    ui->setupUi(this);
    model = new QStandardItemModel;

    iniTableViewStructure();
    fetchData(1);
}

UserManagementForm::~UserManagementForm()
{
    delete ui;
}

void UserManagementForm::iniTableViewStructure()
{
    model->setHorizontalHeaderLabels({tr("用户ID"), tr("用户姓名"), tr("用户状态"), tr("操作")});
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    int lastCol = model->columnCount() - 1;
    ui->tableView->horizontalHeader()->setSectionResizeMode(lastCol, QHeaderView::Stretch);
}

void UserManagementForm::loadData(const QString &jsonString)
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

        // 个数
        QStandardItem *itemId = new QStandardItem(QString::number(row + 1));
        itemId->setFlags(itemId->flags() & ~Qt::ItemIsEditable); // 不可编辑
        model->setItem(row, 0, itemId);

        // 用户名称
        model->setItem(row, 1, new QStandardItem(record["userName"].toString()));

        // 是否启用 (复选框)
        QStandardItem *itemStatus = new QStandardItem();
        itemStatus->setCheckable(true);
        itemStatus->setCheckState(record["state"].toString() == "enabled" ? Qt::Checked
                                                                          : Qt::Unchecked);
        model->setItem(row, 2, itemStatus);
    }

    QStringList btns = {tr("修改密码"), tr("系统权限"), tr("目录权限"), tr("删除"), tr("保存")};
    MultiButtonDelegate *btnDelegate = new MultiButtonDelegate(btns, this);
    ui->tableView->setItemDelegateForColumn(3, btnDelegate);

    connect(btnDelegate,
            &MultiButtonDelegate::buttonClicked,
            this,
            [=](const QModelIndex &index, const QString &btn) {
                qDebug() << "行:" << index.row() << "按钮:" << btn;

                int row = index.row();
                QModelIndex idIndex = model->index(row, 0);
                int id = model->data(idIndex, Qt::DisplayRole).toInt();
                if (btn == tr("修改密码")) {
                    // pauseTask(id);
                } else if (btn == tr("编辑系统权限")) {
                    // resumeTask(id);
                } else if (btn == tr("编辑文件目录权限")) {
                    // restartTask(id);
                } else if (btn == tr("删除")) {
                    // deleteTask(id, row);
                } else if (btn == tr("保存")) {
                    // if (model->item(row, 0)->text() == "new")
                    //     createTask(row);
                    // else
                    //     updateTask(row);
                }
            });

    ui->label->setText(tr("第 %1 页，共 %2 页").arg(currentPage).arg(totalPages));
}

void UserManagementForm::fetchData(int page)
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASUSERLIST), ApiRequest::GET);

    apiRequest->addQueryParam("page", QString::number(page));
    apiRequest->addQueryParam("size", QString::number(30));

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
                updateButton();
                delete apiRequest;
            });

    apiRequest->sendRequest();
}

void UserManagementForm::changePassword(QString name, int row) {}

void UserManagementForm::changeUserStatus(QString name, bool enable) {}

void UserManagementForm::updateStatus() {}

void UserManagementForm::updateButton()
{
    ui->pushButtonFirstPage->setEnabled(currentPage > 1);
    ui->pushButtonTailPage->setEnabled(currentPage != totalPages);
    ui->pushButtonNextPage->setEnabled(currentPage + 1 <= totalPages);
    ui->pushButtonLastPage->setEnabled(currentPage - 1 >= 1);

    ui->spinBoxPage->setMaximum(totalPages);
    ui->spinBoxPage->setMinimum(currentPage > 0 ? 1 : 0);
    ui->pushButtonGoPage->setEnabled(currentPage > 0);
}

void UserManagementForm::addNewRow()
{
    int row = model->rowCount(); // 获取当前行数
    model->insertRow(row);       // 在末尾插入新行

    QStandardItem *itemId = new QStandardItem("new");
    itemId->setFlags(itemId->flags() & ~Qt::ItemIsEditable); // 不可编辑

    // 用户ID
    model->setItem(row, 0, itemId);

    // 用户名称
    model->setItem(row, 1, new QStandardItem(tr("user name")));

    // 是否启用 (复选框)
    QStandardItem *itemStatus = new QStandardItem();
    itemStatus->setCheckable(true);
    itemStatus->setCheckState(Qt::Checked); // 默认启用
    model->setItem(row, 2, itemStatus);

    // 操作按钮列，留空即可
    model->setItem(row, 3, new QStandardItem());

    // 可选：选中新行
    ui->tableView->selectRow(row);
    ui->tableView->scrollToBottom();
}

void UserManagementForm::on_pushButtonFirstPage_clicked()
{
    fetchData(1);
}

void UserManagementForm::on_pushButtonLastPage_clicked()
{
    fetchData(currentPage > 1 ? currentPage - 1 : 0);
}

void UserManagementForm::on_pushButtonNextPage_clicked()
{
    fetchData(currentPage + 1 > totalPages ? totalPages : currentPage + 1);
}

void UserManagementForm::on_pushButtonTailPage_clicked()
{
    if (totalPages > 0) {
        fetchData(totalPages);
    }
}

void UserManagementForm::on_pushButtonGoPage_clicked()
{
    fetchData(ui->spinBoxPage->value());
}

void UserManagementForm::on_pushButtonNewRecord_clicked()
{
    addNewRow();
}
