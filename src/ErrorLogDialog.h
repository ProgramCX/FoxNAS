#ifndef ERRORLOGDIALOG_H
#define ERRORLOGDIALOG_H

#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

namespace Ui {
class ErrorLogDialog;
}

struct ErrorLogItem {
    QString id;
    QString userName;
    QString moduleName;
    QString errorMessage;
    QString stackTrace;
    QString uri;
    QString method;
    QString params;
    QString ipAddress;
    QString createdTime;
    QString exceptionType;
};

class ErrorLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorLogDialog(QWidget *parent = nullptr);
    ~ErrorLogDialog();

private:
    Ui::ErrorLogDialog *ui;
    QList<ErrorLogItem> logList;
    int currentPage = 0;
    int pageSize = 20;
    int totalPages = 0;
    int totalElements = 0;

private:
    void loadErrorLogs();
    void searchErrorLogs();
    void showLogDetail(const ErrorLogItem &log);
    void updateTableWidget();
    void parseLogList(const QJsonArray &array);
    void clearLogsBefore(const QDateTime &beforeTime);
    void updatePagination();

private slots:
    void on_buttonSearch_clicked();
    void on_buttonRefresh_clicked();
    void on_buttonClearLogs_clicked();
    void on_tableWidgetLogs_cellClicked(int row, int column);
    void on_buttonPreviousPage_clicked();
    void on_buttonNextPage_clicked();
    void on_buttonFirstPage_clicked();
    void on_buttonLastPage_clicked();
};

#endif // ERRORLOGDIALOG_H
