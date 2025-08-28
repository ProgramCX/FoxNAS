#ifndef DDNSFORM_H
#define DDNSFORM_H
#include <QWidget>
namespace Ui {
class DDNSForm;
}

class QTimer;
class QStandardItemModel;

class DDNSForm : public QWidget
{
    Q_OBJECT

public:
    explicit DDNSForm(QWidget *parent = nullptr);
    ~DDNSForm();

private slots:
    void on_pushButtonNewRecord_clicked();

    void on_pushButtonFirstPage_clicked();

    void on_pushButtonLastPage_clicked();

    void on_pushButtonNextPage_clicked();

    void on_pushButtonTailPage_clicked();

    void on_pushButtonGoPage_clicked();

    void on_pushButtonDDNSConfig_clicked();

private:
    Ui::DDNSForm *ui;

    QStandardItemModel *model;
    void iniTableViewStructure();
    void loadData(const QString &jsonString);
    void updateButton();
    void fetchData(int page);

    void updateStatus();

    void pauseTask(int id);
    void restartTask(int id);
    void resumeTask(int id);

    void deleteTask(int id, int row);
    void createTask(int row);
    void updateTask(int row);

    void addNewRow();
    void changeTaskStatus(int id, QString api, QString failDesc);
    int findRowById(int id);

private:
    int totalPages;
    int currentPage;

    QTimer *statusTimer;
};

#endif // DDNSFORM_H
