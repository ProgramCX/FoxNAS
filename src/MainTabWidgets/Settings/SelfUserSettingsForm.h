#ifndef SELFUSERSETTINGSFORM_H
#define SELFUSERSETTINGSFORM_H

#include <QWidget>

namespace Ui {
class SelfUserSettingsForm;
}

class SelfUserSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SelfUserSettingsForm(QWidget *parent = nullptr);
    ~SelfUserSettingsForm();

private slots:
    void on_pushButtonModifyPsw_clicked();

    void on_lineEditUserName_textChanged(const QString &arg1);

    void on_pushButtonModifyName_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::SelfUserSettingsForm *ui;

private:
    void changeUserName();
    void iniUi();
};

#endif // SELFUSERSETTINGSFORM_H
