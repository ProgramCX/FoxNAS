#ifndef DDNSSECRETFORM_H
#define DDNSSECRETFORM_H

#include <QWidget>

namespace Ui {
class DDNSSecretForm;
}

class QStandardItemModel;
class QItemSelection;
class DDNSSecretForm : public QWidget
{
    Q_OBJECT

public:
    enum FormMode { SelectMode = 0, ManageMode };
    explicit DDNSSecretForm(DDNSSecretForm::FormMode formMode = ManageMode,
                            QWidget *parent = nullptr);
    ~DDNSSecretForm();

    long getSelectedId() const;
    void setSelectedId(long newSelectedId);

private:
    Ui::DDNSSecretForm *ui;

    QStandardItemModel *model;
    FormMode currentWidgetMode = ManageMode;

    int totalPages;
    int currentPage;
    long selectedId = -1;
    QStringList providerOps = {tr("Cloudflare"), tr("阿里云"), tr("腾讯云"), tr("华为云")};
private slots:
    void on_pushButtonFirstPage_clicked();

    void on_pushButtonLastPage_clicked();

    void on_pushButtonNextPage_clicked();

    void on_pushButtonTailPage_clicked();

    void on_pushButtonGoPage_clicked();

    void on_pushButtonNewRecord_clicked();

    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void iniUi();
    void fetchData(int page);
    void loadData(const QString &jsonString);

    void updateButton();
    void createSecret(int row);
    void updateTask(int row);
    void deleteTask(int id, int row);

    void addNewRow();
};

#endif // DDNSSECRETFORM_H
