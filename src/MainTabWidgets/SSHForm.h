#ifndef SSHFORM_H
#define SSHFORM_H

#include <QWidget>

namespace Ui {
class SSHForm;
}

class SSHForm : public QWidget
{
    Q_OBJECT

public:
    explicit SSHForm(QWidget *parent = nullptr);
    ~SSHForm();

private:
    Ui::SSHForm *ui;
};

#endif // SSHFORM_H
