#ifndef DIRSELECTDIALOG_H
#define DIRSELECTDIALOG_H

#include <QDialog>

class QTreeWidgetItem;
namespace Ui {
class DirSelectDialog;
}

class DirSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DirSelectDialog(QString initialDir = "", QWidget *parent = nullptr);
    ~DirSelectDialog();

    QString getSelectedDir() const;

private slots:
    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_treeWidget_itemExpanded(QTreeWidgetItem *item);

private:
    Ui::DirSelectDialog *ui;
    QString selectedDir;
    QList<QString> onRequestList;

private:
    void fetchDirectory(QString path = "", QTreeWidgetItem *parentItem = nullptr);
    void clearChild(QTreeWidgetItem *item = nullptr);
};

#endif // DIRSELECTDIALOG_H
