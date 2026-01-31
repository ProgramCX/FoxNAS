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
#include <UserPermissionDialog.h>

#include <PasswordModifyDialog.h>
#include <UserDirPermissionDialog.h>

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
    model->setHorizontalHeaderLabels({tr("用户UUID"), tr("用户姓名"), tr("用户状态"), tr("操作")});
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

        // 用户 UUID
        QStandardItem *itemUuid = new QStandardItem(record["id"].toString());
        itemUuid->setFlags(itemUuid->flags() & ~Qt::ItemIsEditable); // 不可编辑
        model->setItem(row, 0, itemUuid);

        // 用户名称
        QStandardItem *itemUserName = new QStandardItem(record["userName"].toString());
        itemUserName->setData(record["id"].toString(), Qt::UserRole); // 存储 UUID
        model->setItem(row, 1, itemUserName);

        // 是否启用 (复选框)
        QStandardItem *itemStatus = new QStandardItem();
        itemStatus->setCheckable(true);
        itemStatus->setCheckState(record["state"].toString() == "enabled" ? Qt::Checked
                                                                          : Qt::Unchecked);
        model->setItem(row, 2, itemStatus);
    }

    QStringList btns = {tr("设置密码"), tr("系统权限"), tr("目录权限"), tr("删除"), tr("保存")};
    MultiButtonDelegate *btnDelegate = new MultiButtonDelegate(btns, this);
    ui->tableView->setItemDelegateForColumn(3, btnDelegate);

    connect(btnDelegate,
            &MultiButtonDelegate::buttonClicked,
            this,
            [=](const QModelIndex &index, const QString &btn) {
                qDebug() << "行:" << index.row() << "按钮:" << btn;

                int row = index.row();
                QString uuid = model->item(row, 0)->text();
                if (btn == tr("设置密码")) {
                    if (uuid.isEmpty() || uuid == "new") {
                        QMessageBox::critical(this,
                                              tr("错误"),
                                              tr("请先点击保存按钮保存用户后设置密码！"));
                        return;
                    }
                    changePassword(uuid, row);
                } else if (btn == tr("系统权限")) {
                    if (uuid.isEmpty() || uuid == "new") {
                        QMessageBox::critical(this,
                                              tr("错误"),
                                              tr("请先点击保存按钮保存用户后设置系统权限！"));
                        return;
                    }
                    showSystemPermissions(uuid, row);
                } else if (btn == tr("目录权限")) {
                    if (uuid.isEmpty() || uuid == "new") {
                        QMessageBox::critical(this,
                                              tr("错误"),
                                              tr("请先点击保存按钮保存用户后设置目录权限！"));
                        return;
                    }
                    UserDirPermissionDialog *dialog = new UserDirPermissionDialog(uuid);
                    dialog->show();
                } else if (btn == tr("删除")) {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(
                        this,                                                   // 父窗口
                        "确认删除",                                             // 标题
                        "确定要删除该用户吗？关于该用户的所有信息都将被删除！", // 提示文本
                        QMessageBox::Yes | QMessageBox::No                      // 按钮
                    );

                    if (reply == QMessageBox::Yes) {
                        deleteUser(uuid, row);
                    }

                } else if (btn == tr("保存")) {
                    if (model->item(row, 0)->text() == "new")
                        createUser(row);
                    else
                        updateUser(row);
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
                    QMessageBox::critical(nullptr, "失败", "获取用户列表失败！", tr("确定"));
                }
                updateButton();
                delete apiRequest;
            });

    apiRequest->sendRequest();
}

void UserManagementForm::changePassword(QString uuid, int row)
{
    PasswordModifyDialog dialog(uuid, this);
    dialog.exec();
}

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

    QStandardItem *itemUuid = new QStandardItem("new");
    itemUuid->setFlags(itemUuid->flags() & ~Qt::ItemIsEditable); // 不可编辑

    // 用户 UUID
    model->setItem(row, 0, itemUuid);

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

void UserManagementForm::updateUser(int row)
{
    QString uuid = model->item(row, 0)->text();
    QJsonObject record;
    record["userName"] = model->item(row, 1)->text();
    if (record["userName"].toString().isEmpty()) {
        QMessageBox::warning(this, "警告", tr("用户名不能为空！"), tr("确定"));
        return;
    }
    record["state"] = model->item(row, 2)->checkState() == Qt::Checked ? "enabled" : "disabled";

    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASUSERUPDATE),
                                            ApiRequest::PUT,

                                            QJsonDocument(record));
    apiRequest->addQueryParam("uuid", uuid);
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, apiRequest, record, row](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(this, "失败", tr("更新用户信息失败！"), tr("确定"));
                } else {
                    updateStatus();
                    QMessageBox::information(this,
                                             "成功",
                                             tr("更新用户信息成功，userName = %1")
                                                 .arg(this->model->item(row, 1)->text()),
                                             tr("确定"));
                    fetchData(currentPage);
                }

                delete apiRequest;
            });
    apiRequest->sendRequest();
}

void UserManagementForm::createUser(int row)
{
    QJsonObject record;
    record["userName"] = model->item(row, 1)->text();
    record["password"] = "";
    if (record["userName"].toString().isEmpty()) {
        QMessageBox::warning(this, "警告", tr("用户名不能为空！"), tr("确定"));
        return;
    }
    record["state"] = model->item(row, 2)->checkState() == Qt::Checked ? "enabled" : "disabled";

    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASUSERADD),
                                            ApiRequest::POST,

                                            QJsonDocument(record));
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [this, apiRequest, record, row](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(this, "失败", tr("添加用户失败！"), tr("确定"));
                } else {
                    updateStatus();
                    QMessageBox::information(
                        this,
                        "成功",
                        tr("添加用户成功，userName=%"
                           "1。请继续添加该用户的密码（密码默认为空）、文件目录权限和系统权限")
                            .arg(this->model->item(row, 1)->text()),
                        tr("确定"));
                    fetchData(currentPage);
                }

                delete apiRequest;
            });
    apiRequest->sendRequest();
}

void UserManagementForm::deleteUser(QString uuid, int row)
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASUSERDELETE),
                                            ApiRequest::DELETE);
    apiRequest->addQueryParam("uuid", uuid);
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (hasError || statusCode != 200) {
                    QMessageBox::critical(nullptr,
                                          "失败",
                                          tr("删除用户失败"),
                                          tr("确定"));
                } else {
                    model->removeRow(row);
                    QMessageBox::information(nullptr, "成功", tr("删除用户成功"), tr("确定"));
                }
                delete apiRequest;
            });

    apiRequest->sendRequest();
}

void UserManagementForm::showSystemPermissions(QString uuid, int row)
{
    UserPermissionDialog dialog(uuid, this);
    dialog.exec();
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
