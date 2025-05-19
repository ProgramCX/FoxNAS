#ifndef DDNSFORM_H
#define DDNSFORM_H

#include <QWidget>

namespace Ui {
class DDNSForm;
}

class DDNSForm : public QWidget
{
    Q_OBJECT

public:
    explicit DDNSForm(QWidget *parent = nullptr);
    ~DDNSForm();

private:
    Ui::DDNSForm *ui;
};

#endif // DDNSFORM_H
