#include "LoginDialog.h"
#include "LanguageManager.h"

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

    // 使用 LanguageManager 加载语言设置
    LanguageManager::instance().loadLanguageFromSettings();

    LoginDialog w;
    w.show();
    return a.exec();
}
