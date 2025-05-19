#ifndef OVERVIEWFORM_H
#define OVERVIEWFORM_H

#include <QWidget>

namespace Ui {
class OverViewForm;
}

class OverViewForm : public QWidget
{
    Q_OBJECT

public:
    explicit OverViewForm(QWidget *parent = nullptr);
    ~OverViewForm();

private:
    Ui::OverViewForm *ui;
};

#endif // OVERVIEWFORM_H
