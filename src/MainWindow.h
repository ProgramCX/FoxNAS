#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <ApiRequest.h>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ApiRequest *apiRequest;

    void getPermissions();
    void iniPermissionUi(QString &rawContent, bool hasError, qint16 statusCode);
};

#endif // MAINWINDOW_H
