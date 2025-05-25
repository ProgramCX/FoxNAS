#ifndef DIRSAUTHEDSELECTDIALOG_H
#define DIRSAUTHEDSELECTDIALOG_H

#include <QDialog>

namespace Ui {
class DirsAuthedSelectDialog;
}

class DirsAuthedSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DirsAuthedSelectDialog(QWidget *parent = nullptr);
    ~DirsAuthedSelectDialog();

    QString getSelectedPath() const;

private:
    Ui::DirsAuthedSelectDialog *ui;
    QString selectedPath;

private:
    void getAuthedDirs();
    void iniUi();
};

#endif // DIRSAUTHEDSELECTDIALOG_H
