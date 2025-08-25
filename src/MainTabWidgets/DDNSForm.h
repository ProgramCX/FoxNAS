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

private:
    Ui::DDNSForm *ui;

    QStandardItemModel *model;
    void iniTableViewStructure();
    void loadData(const QString &jsonString);
    void fetchData(int page);

    void updateStatus();

    void pauseTask(int id);
    void restartTask(int id);
    void resumeTask(int id);

    void deleteTask(int id, int row);
    void createTask(int id);
    void updateTask(int row);

    void changeTaskStatus(int id, QString api, QString failDesc);
    int findRowById(int id);

private:
    int totalPages;
    int currentPage;

    QTimer *statusTimer;
};

#endif // DDNSFORM_H
