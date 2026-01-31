#include "LanguageManager.h"
#include "IniSettings.hpp"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QLibraryInfo>
#include <QSettings>
#include <QWidget>

LanguageManager::LanguageManager()
    : QObject(nullptr)
    , m_appTranslator(new QTranslator(this))
    , m_qtTranslator(new QTranslator(this))
{
    initSupportedLanguages();
}

LanguageManager::~LanguageManager()
{
}

LanguageManager &LanguageManager::instance()
{
    static LanguageManager instance;
    return instance;
}

void LanguageManager::initSupportedLanguages()
{
    // 初始化支持的语言列表
    m_supportedLanguages = {
        {"en", "English", "English", Qt::LeftToRight},
        {"zh_CN", "简体中文", "Simplified Chinese", Qt::LeftToRight},
        {"zh_TW", "繁體中文", "Traditional Chinese", Qt::LeftToRight},
        {"ja", "日本語", "Japanese", Qt::LeftToRight},
        {"fr", "Français", "French", Qt::LeftToRight},
        {"de", "Deutsch", "German", Qt::LeftToRight},
        {"pt", "Português", "Portuguese", Qt::LeftToRight},
        {"ru", "Русский", "Russian", Qt::LeftToRight},
        {"ar", "العربية", "Arabic", Qt::RightToLeft}
    };
}

QVector<LanguageInfo> LanguageManager::supportedLanguages() const
{
    return m_supportedLanguages;
}

QString LanguageManager::currentLanguage() const
{
    return m_currentLanguage;
}

QString LanguageManager::currentLanguageName() const
{
    for (const auto &lang : m_supportedLanguages) {
        if (lang.code == m_currentLanguage) {
            return lang.nativeName;
        }
    }
    return "English";
}

int LanguageManager::languageIndex(const QString &languageCode) const
{
    for (int i = 0; i < m_supportedLanguages.size(); ++i) {
        if (m_supportedLanguages[i].code == languageCode) {
            return i;
        }
    }
    return 0; // 默认返回英语索引
}

QString LanguageManager::languageCodeAt(int index) const
{
    if (index >= 0 && index < m_supportedLanguages.size()) {
        return m_supportedLanguages[index].code;
    }
    return "en";
}

bool LanguageManager::switchLanguage(const QString &languageCode)
{
    if (m_currentLanguage == languageCode) {
        return true;
    }

    if (!loadTranslator(languageCode)) {
        qWarning() << "Failed to load translator for language:" << languageCode;
        return false;
    }

    m_currentLanguage = languageCode;
    saveLanguageToSettings();

    // 设置布局方向
    for (const auto &lang : m_supportedLanguages) {
        if (lang.code == languageCode) {
            qApp->setLayoutDirection(lang.layoutDirection);
            break;
        }
    }

    emit languageChanged(languageCode);
    qDebug() << "Language switched to:" << languageCode;
    return true;
}

bool LanguageManager::loadTranslator(const QString &languageCode)
{
    // 移除旧的翻译器
    if (m_appTranslator) {
        qApp->removeTranslator(m_appTranslator);
    }
    if (m_qtTranslator) {
        qApp->removeTranslator(m_qtTranslator);
    }

    // 英语是默认语言，不需要加载翻译文件
    // 只需移除翻译器即可恢复到源代码中的原始字符串
    // if (languageCode == "en") {
    //     // 发送 LanguageChange 事件来更新界面
    //     for (QWidget *widget : qApp->topLevelWidgets()) {
    //         QEvent event(QEvent::LanguageChange);
    //         QCoreApplication::sendEvent(widget, &event);
    //     }
    //     return true;
    // }

    // 加载应用程序翻译文件
    QString appTranslationFile = "FoxNAS_" + languageCode;
    bool appLoaded = false;

    // 尝试从资源文件加载
    if (m_appTranslator->load(":/i18n/" + appTranslationFile)) {
        appLoaded = true;
        qDebug() << "Loaded app translation from resource:" << appTranslationFile;
    }
    // 尝试从当前目录加载
    else if (m_appTranslator->load(appTranslationFile, QCoreApplication::applicationDirPath())) {
        appLoaded = true;
        qDebug() << "Loaded app translation from app dir:" << appTranslationFile;
    }
    // 尝试从 translations 目录加载
    else if (m_appTranslator->load(appTranslationFile,
                                    QCoreApplication::applicationDirPath() + "/translations")) {
        appLoaded = true;
        qDebug() << "Loaded app translation from translations dir:" << appTranslationFile;
    }

    if (appLoaded) {
        qApp->installTranslator(m_appTranslator);
    } else {
        qWarning() << "Could not load app translation:" << appTranslationFile;
    }

    // 加载 Qt 标准翻译文件
    QString qtTranslationFile = "qt_" + languageCode;
    if (m_qtTranslator->load(qtTranslationFile,
                              QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        qApp->installTranslator(m_qtTranslator);
        qDebug() << "Loaded Qt translation:" << qtTranslationFile;
    }

    return appLoaded;
}

void LanguageManager::loadLanguageFromSettings()
{
    QSettings &settings = IniSettings::getGlobalSettingsInstance();
    QString savedLanguage = settings.value("General/language", "").toString();

    if (savedLanguage.isEmpty()) {
        // 如果没有保存的语言设置，尝试使用系统语言
        QLocale systemLocale = QLocale::system();
        QString systemLanguage = systemLocale.name(); // 例如 "zh_CN"

        // 检查系统语言是否在支持列表中
        bool found = false;
        for (const auto &lang : m_supportedLanguages) {
            if (lang.code == systemLanguage) {
                savedLanguage = systemLanguage;
                found = true;
                break;
            }
        }

        // 尝试匹配语言代码的前缀（如 zh）
        if (!found) {
            QString langPrefix = systemLanguage.split('_').first();
            for (const auto &lang : m_supportedLanguages) {
                if (lang.code.startsWith(langPrefix)) {
                    savedLanguage = lang.code;
                    found = true;
                    break;
                }
            }
        }

        // 如果还是没找到，使用英语
        if (!found) {
            savedLanguage = "en";
        }
    }

    switchLanguage(savedLanguage);
}

void LanguageManager::saveLanguageToSettings()
{
    QSettings &settings = IniSettings::getGlobalSettingsInstance();
    settings.setValue("General/language", m_currentLanguage);
    settings.sync();
}
