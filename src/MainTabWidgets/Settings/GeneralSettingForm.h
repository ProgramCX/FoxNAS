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

private:
    Ui::GeneralSettingForm *ui;
};

#endif // GENERALSETTINGFORM_H
