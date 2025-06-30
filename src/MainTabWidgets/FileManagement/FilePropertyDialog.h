#ifndef FILEPROPERTYDIALOG_H
#define FILEPROPERTYDIALOG_H

#include <QDialog>

namespace Ui {
class FilePropertyDialog;
}

class FilePropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilePropertyDialog(QWidget *parent = nullptr);
    ~FilePropertyDialog();

private:
    Ui::FilePropertyDialog *ui;
};

#endif // FILEPROPERTYDIALOG_H
