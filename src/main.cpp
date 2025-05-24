#include "LoginDialog.h"

#include <QApplication>
#include <QLocale>
#include <QStyleFactory>
#include <QTranslator>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef Q_OS_WIN
    a.setStyle(QStyleFactory::create("WindowsVista"));
#endif

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "FoxNAS_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    LoginDialog w;
    w.show();
    return a.exec();
}
