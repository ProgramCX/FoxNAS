#ifndef FILEOPERATIONFAILEDDIALOG_H
#define FILEOPERATIONFAILEDDIALOG_H

#include <QDialog>
#include <QList>
namespace Ui {
class FileOperationFailedDialog;
}

class FileOperationFailedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileOperationFailedDialog(qint64 total,
                                       qint64 failed,
                                       QString message,
                                       QWidget *parent = nullptr);
    ~FileOperationFailedDialog();

public:
    void addFileLists(QList<QString> fileList);
    void addFileLists(QList<QMap<QString, QString>> fileList);

    QList<QMap<QString, QString>> mapList;
    QList<QString> list;

    QList<QMap<QString, QString>> getMapResult();
    QList<QString> getResult();

private:
    Ui::FileOperationFailedDialog *ui;
};

#endif // FILEOPERATIONFAILEDDIALOG_H
