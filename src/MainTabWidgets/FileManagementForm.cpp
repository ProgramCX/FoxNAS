#include "FileManagementForm.h"
#include "ui_FileManagementForm.h"

FileManagementForm::FileManagementForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileManagementForm)
{
    ui->setupUi(this);

    addWidgets();
}

FileManagementForm::~FileManagementForm()
{
    delete ui;
}

void FileManagementForm::addWidgets()
{
    newToolButton = new QToolButton;
    newToolButton->setText("+");
    ui->tabWidget->setCornerWidget(newToolButton, Qt::TopLeftCorner);
    connect(newToolButton, &QToolButton::clicked, [&]() { addNewTab(""); });

    QObject::connect(ui->tabWidget, &QTabWidget::tabCloseRequested, [=](int index) {
        QWidget *tab = ui->tabWidget->widget(index);
        if (ui->tabWidget->count() == 1) {
            return;
        }
        ui->tabWidget->removeTab(index);
        delete tab;
    });
}

int FileManagementForm::getTabCount() const
{
    return tabCount;
}

void FileManagementForm::addNewTab(QString path)
{
    if (path == "") {
        FileManagementTabForm *fileManagementTabForm = new FileManagementTabForm(this);
        ui->tabWidget->addTab(fileManagementTabForm, "主页");
        ui->tabWidget->setCurrentWidget(fileManagementTabForm);
        fileManagementTabForm->showSelectDirDialog();
    }
    tabCount++;
}
