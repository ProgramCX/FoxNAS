#ifndef INFOFORM_H
#define INFOFORM_H

#include <QWidget>

namespace Ui {
class InfoForm;
}

class InfoForm : public QWidget
{
    Q_OBJECT

public:
    explicit InfoForm(QWidget *parent = nullptr);
    ~InfoForm();

    void loadHardwareInfo();

private slots:
    void on_pushButtonCopy_clicked();

    void on_pushButtonAbout_clicked();

    void on_pushButtonViewLogs_clicked();

private:
    Ui::InfoForm *ui;
};

#endif // INFOFORM_H
