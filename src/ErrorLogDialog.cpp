#include "ErrorLogDialog.h"
#include "LoadingDialog.h"
#include "ui_ErrorLogDialog.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QDateTime>
#include <QPushButton>
#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

ErrorLogDialog::ErrorLogDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ErrorLogDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("错误日志"));
    this->setFixedSize(1200, 700);

    ui->tableWidgetLogs->setColumnCount(7);
    QStringList headers = { tr("ID"), tr("模块名称"), tr("用户名"), tr("异常类型"), tr("错误信息"), tr("创建时间"), tr("操作") };
    ui->tableWidgetLogs->setHorizontalHeaderLabels(headers);
    ui->tableWidgetLogs->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetLogs->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetLogs->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetLogs->setAlternatingRowColors(true);

    connect(ui->buttonSearch, &QPushButton::clicked, this, &ErrorLogDialog::on_buttonSearch_clicked);
    connect(ui->buttonRefresh, &QPushButton::clicked, this, &ErrorLogDialog::on_buttonRefresh_clicked);
    connect(ui->buttonClearLogs, &QPushButton::clicked, this, &ErrorLogDialog::on_buttonClearLogs_clicked);
    connect(ui->tableWidgetLogs, &QTableWidget::cellClicked, this, &ErrorLogDialog::on_tableWidgetLogs_cellClicked);
    connect(ui->buttonPreviousPage, &QPushButton::clicked, this, &ErrorLogDialog::on_buttonPreviousPage_clicked);
    connect(ui->buttonNextPage, &QPushButton::clicked, this, &ErrorLogDialog::on_buttonNextPage_clicked);
    connect(ui->buttonFirstPage, &QPushButton::clicked, this, &ErrorLogDialog::on_buttonFirstPage_clicked);
    connect(ui->buttonLastPage, &QPushButton::clicked, this, &ErrorLogDialog::on_buttonLastPage_clicked);

    loadErrorLogs();
}

ErrorLogDialog::~ErrorLogDialog()
{
    delete ui;
}

void ErrorLogDialog::loadErrorLogs()
{
    LoadingDialog *loading = new LoadingDialog(tr("正在加载日志..."), this);
    loading->show();

    ApiRequest *apiRequest = new ApiRequest(
        getFullApiPath(FULLHOST, NASERRORLOGLIST) + QString("?page=%1&size=%2").arg(currentPage).arg(pageSize),
        ApiRequest::GET,
        this
    );

    connect(apiRequest, &ApiRequest::responseRecieved, this, [this, loading](QString &rawContent, bool hasError, qint16 statusCode) {
        loading->close();
        loading->deleteLater();

        if (hasError) {
            QMessageBox::warning(this, tr("错误"), tr("获取日志列表失败：") + rawContent);
            return;
        }

        QJsonParseError jsonParseError;
        QJsonDocument document = QJsonDocument::fromJson(rawContent.toUtf8(), &jsonParseError);

        if (jsonParseError.error != QJsonParseError::NoError) {
            QMessageBox::warning(this, tr("错误"), tr("解析响应失败"));
            return;
        }

        QJsonObject root = document.object();
        QJsonArray list = root.value("content").toArray();
        totalElements = root.value("totalElements").toInt();
        totalPages = (totalElements + pageSize - 1) / pageSize;

        logList.clear();
        parseLogList(list);
        updateTableWidget();
        updatePagination();
    });

    apiRequest->sendRequest();
}

void ErrorLogDialog::searchErrorLogs()
{
    currentPage = 0;
    LoadingDialog *loading = new LoadingDialog(tr("正在搜索日志..."), this);
    loading->show();

    QString moduleName = ui->lineEditModuleName->text();
    QString userName = ui->lineEditUserName->text();
    QString exceptionType = ui->lineEditExceptionType->text();
    QString startTime = ui->dateTimeEditStart->dateTime().toString(Qt::ISODate);
    QString endTime = ui->dateTimeEditEnd->dateTime().toString(Qt::ISODate);

    QString url = getFullApiPath(FULLHOST, NASERRORLOGLIST) + QString("?page=%1&size=%2").arg(currentPage).arg(pageSize);

    if (!moduleName.isEmpty()) {
        url += QString("&moduleName=%1").arg(moduleName);
    }
    if (!userName.isEmpty()) {
        url += QString("&userName=%1").arg(userName);
    }
    if (!exceptionType.isEmpty()) {
        url += QString("&exceptionType=%1").arg(exceptionType);
    }
    if (ui->dateTimeEditStart->dateTime().isValid() && ui->dateTimeEditEnd->dateTime().isValid()) {
        url += QString("&startTime=%1&endTime=%2").arg(startTime).arg(endTime);
    }

    ApiRequest *apiRequest = new ApiRequest(url, ApiRequest::GET, this);

    connect(apiRequest, &ApiRequest::responseRecieved, this, [this, loading](QString &rawContent, bool hasError, qint16 statusCode) {
        loading->close();
        loading->deleteLater();

        if (hasError) {
            QMessageBox::warning(this, tr("错误"), tr("搜索日志失败：") + rawContent);
            return;
        }

        QJsonParseError jsonParseError;
        QJsonDocument document = QJsonDocument::fromJson(rawContent.toUtf8(), &jsonParseError);

        if (jsonParseError.error != QJsonParseError::NoError) {
            QMessageBox::warning(this, tr("错误"), tr("解析响应失败"));
            return;
        }

        QJsonObject root = document.object();
        QJsonArray list = root.value("content").toArray();
        totalElements = root.value("totalElements").toInt();
        totalPages = (totalElements + pageSize - 1) / pageSize;

        logList.clear();
        parseLogList(list);
        updateTableWidget();
        updatePagination();
    });

    apiRequest->sendRequest();
}

void ErrorLogDialog::parseLogList(const QJsonArray &array)
{
    for (int i = 0; i < array.size(); ++i) {
        QJsonObject obj = array.at(i).toObject();
        ErrorLogItem item;
        item.id = obj.value("id").toString();
        item.userName = obj.value("userName").toString();
        item.moduleName = obj.value("moduleName").toString();
        item.errorMessage = obj.value("errorMessage").toString();
        item.stackTrace = obj.value("stackTrace").toString();
        item.uri = obj.value("uri").toString();
        item.method = obj.value("method").toString();
        item.params = obj.value("params").toString();
        item.ipAddress = obj.value("ipAddress").toString();
        item.createdTime = obj.value("createdTime").toString();
        item.exceptionType = obj.value("exceptionType").toString();
        logList.append(item);
    }
}

void ErrorLogDialog::updateTableWidget()
{
    ui->tableWidgetLogs->setRowCount(logList.size());

    for (int i = 0; i < logList.size(); ++i) {
        const ErrorLogItem &log = logList.at(i);

        QTableWidgetItem *idItem = new QTableWidgetItem(log.id);
        idItem->setToolTip(log.id);
        ui->tableWidgetLogs->setItem(i, 0, idItem);

        ui->tableWidgetLogs->setItem(i, 1, new QTableWidgetItem(log.moduleName));
        ui->tableWidgetLogs->setItem(i, 2, new QTableWidgetItem(log.userName));
        ui->tableWidgetLogs->setItem(i, 3, new QTableWidgetItem(log.exceptionType));

        QTableWidgetItem *errorMsgItem = new QTableWidgetItem(log.errorMessage);
        errorMsgItem->setToolTip(log.errorMessage);
        ui->tableWidgetLogs->setItem(i, 4, errorMsgItem);

        ui->tableWidgetLogs->setItem(i, 5, new QTableWidgetItem(log.createdTime));

        QPushButton *detailButton = new QPushButton(tr("详情"));
        detailButton->setProperty("row", i);
        connect(detailButton, &QPushButton::clicked, this, [this, i]() {
            on_tableWidgetLogs_cellClicked(i, 0);
        });

        ui->tableWidgetLogs->setCellWidget(i, 6, detailButton);
    }

    ui->tableWidgetLogs->resizeColumnToContents(0);
    ui->tableWidgetLogs->setColumnWidth(0, 200);
    ui->tableWidgetLogs->setColumnWidth(1, 100);
    ui->tableWidgetLogs->setColumnWidth(2, 100);
    ui->tableWidgetLogs->setColumnWidth(3, 150);
    ui->tableWidgetLogs->setColumnWidth(5, 160);
}

void ErrorLogDialog::showLogDetail(const ErrorLogItem &log)
{
    QString detailText = QStringLiteral(R"(
<h3 style="color: #d32f2f;">错误日志详情</h3>
<table style="width: 100%; border-collapse: collapse; margin: 10px 0;">
<tr><td style="padding: 8px; font-weight: bold; width: 120px;">日志ID:</td><td style="padding: 8px;">%1</td></tr>
<tr><td style="padding: 8px; font-weight: bold;">用户名:</td><td style="padding: 8px;">%2</td></tr>
<tr><td style="padding: 8px; font-weight: bold;">模块名称:</td><td style="padding: 8px;">%3</td></tr>
<tr><td style="padding: 8px; font-weight: bold;">异常类型:</td><td style="padding: 8px;"><font color="#d32f2f">%4</font></td></tr>
<tr><td style="padding: 8px; font-weight: bold;">IP地址:</td><td style="padding: 8px;">%5</td></tr>
<tr><td style="padding: 8px; font-weight: bold;">请求地址:</td><td style="padding: 8px;">%6 %7</td></tr>
<tr><td style="padding: 8px; font-weight: bold;">创建时间:</td><td style="padding: 8px;">%8</td></tr>
</table>

<h4 style="color: #d32f2f;">错误信息:</h4>
<pre style="background: #ffebee; padding: 12px; border-radius: 4px; overflow-x: auto;">%9</pre>

<h4>请求参数:</h4>
<pre style="background: #f5f5f5; padding: 12px; border-radius: 4px; overflow-x: auto;">%10</pre>

<h4>堆栈跟踪:</h4>
<pre style="background: #f5f5f5; padding: 12px; border-radius: 4px; overflow-x: auto; max-height: 300px;">%11</pre>
)")
        .arg(log.id)
        .arg(log.userName)
        .arg(log.moduleName)
        .arg(log.exceptionType)
        .arg(log.ipAddress)
        .arg(log.method)
        .arg(log.uri)
        .arg(log.createdTime)
        .arg(log.errorMessage)
        .arg(log.params)
        .arg(log.stackTrace);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("日志详情"));
    msgBox.setText(detailText);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void ErrorLogDialog::clearLogsBefore(const QDateTime &beforeTime)
{
    LoadingDialog *loading = new LoadingDialog(tr("正在清理日志..."), this);
    loading->show();

    QString url = getFullApiPath(FULLHOST, NASDELETEERRORLOGSBEFORE) + QString("?beforeTime=%1").arg(beforeTime.toString(Qt::ISODate));

    ApiRequest *apiRequest = new ApiRequest(url, ApiRequest::DELETE, this);

    connect(apiRequest, &ApiRequest::responseRecieved, this, [this, loading](QString &rawContent, bool hasError, qint16 statusCode) {
        loading->close();
        loading->deleteLater();

        if (hasError) {
            QMessageBox::warning(this, tr("错误"), tr("清理日志失败：") + rawContent);
            return;
        }

        QMessageBox::information(this, tr("成功"), tr("日志清理成功！"));
        loadErrorLogs();
    });

    apiRequest->sendRequest();
}

void ErrorLogDialog::updatePagination()
{
    ui->labelPageInfo->setText(tr("第 %1 页 / 共 %2 页 (共 %3 条)").arg(currentPage + 1).arg(totalPages).arg(totalElements));
    ui->buttonPreviousPage->setEnabled(currentPage > 0);
    ui->buttonNextPage->setEnabled(currentPage < totalPages - 1);
    ui->buttonFirstPage->setEnabled(currentPage > 0);
    ui->buttonLastPage->setEnabled(currentPage < totalPages - 1);
}

void ErrorLogDialog::on_buttonSearch_clicked()
{
    searchErrorLogs();
}

void ErrorLogDialog::on_buttonRefresh_clicked()
{
    loadErrorLogs();
}

void ErrorLogDialog::on_buttonClearLogs_clicked()
{
    QDateTime selectedDateTime = ui->dateTimeEditClearBefore->dateTime();
    if (!selectedDateTime.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请选择日期时间"));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认清理"),
        tr("确定要删除 %1 之前的所有日志吗？").arg(selectedDateTime.toString()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        clearLogsBefore(selectedDateTime);
    }
}

void ErrorLogDialog::on_tableWidgetLogs_cellClicked(int row, int column)
{
    if (row >= 0 && row < logList.size()) {
        showLogDetail(logList.at(row));
    }
}

void ErrorLogDialog::on_buttonPreviousPage_clicked()
{
    if (currentPage > 0) {
        currentPage--;
        loadErrorLogs();
    }
}

void ErrorLogDialog::on_buttonNextPage_clicked()
{
    if (currentPage < totalPages - 1) {
        currentPage++;
        loadErrorLogs();
    }
}

void ErrorLogDialog::on_buttonFirstPage_clicked()
{
    if (currentPage > 0) {
        currentPage = 0;
        loadErrorLogs();
    }
}

void ErrorLogDialog::on_buttonLastPage_clicked()
{
    if (currentPage < totalPages - 1) {
        currentPage = totalPages - 1;
        loadErrorLogs();
    }
}
