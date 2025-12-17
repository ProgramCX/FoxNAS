#ifndef DIRPERMISSIONEDITDIALOG_H
#define DIRPERMISSIONEDITDIALOG_H

#include <QWidget>

namespace Ui {
class DIrPermissionEditDialog;
}

class DIrPermissionEditDialog : public QWidget
{
    Q_OBJECT

public:
    explicit DIrPermissionEditDialog(QWidget *parent = nullptr);
    ~DIrPermissionEditDialog();

private:
    Ui::DIrPermissionEditDialog *ui;
};

#endif // DIRPERMISSIONEDITDIALOG_H
