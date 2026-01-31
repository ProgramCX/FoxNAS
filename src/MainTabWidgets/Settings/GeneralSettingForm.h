#ifndef GENERALSETTINGFORM_H
#define GENERALSETTINGFORM_H

#include <QWidget>

namespace Ui {
class GeneralSettingForm;
}

class GeneralSettingForm : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralSettingForm(QWidget *parent = nullptr);
    ~GeneralSettingForm();

private slots:
    void onLanguageChanged(int index);

private:
    Ui::GeneralSettingForm *ui;

    void initLanguageComboBox();
    void retranslateUi();

protected:
    void changeEvent(QEvent *event) override;
};

#endif // GENERALSETTINGFORM_H
