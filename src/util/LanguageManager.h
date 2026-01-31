#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QApplication>
#include <QLocale>
#include <QObject>
#include <QString>
#include <QTranslator>
#include <QVector>

/**
 * @brief 语言信息结构体
 */
struct LanguageInfo
{
    QString code;       // 语言代码，如 "zh_CN"
    QString nativeName; // 本地名称，如 "简体中文"
    QString englishName; // 英文名称，如 "Simplified Chinese"
    Qt::LayoutDirection layoutDirection; // 布局方向
};

/**
 * @brief 语言管理器单例类
 * 
 * 用于管理应用程序的多语言切换功能
 */
class LanguageManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static LanguageManager &instance();

    /**
     * @brief 获取支持的语言列表
     */
    QVector<LanguageInfo> supportedLanguages() const;

    /**
     * @brief 获取当前语言代码
     */
    QString currentLanguage() const;

    /**
     * @brief 获取当前语言的本地名称
     */
    QString currentLanguageName() const;

    /**
     * @brief 切换语言
     * @param languageCode 语言代码
     * @return 是否切换成功
     */
    bool switchLanguage(const QString &languageCode);

    /**
     * @brief 从设置中加载语言
     */
    void loadLanguageFromSettings();

    /**
     * @brief 保存当前语言到设置
     */
    void saveLanguageToSettings();

    /**
     * @brief 获取语言代码在列表中的索引
     */
    int languageIndex(const QString &languageCode) const;

    /**
     * @brief 根据索引获取语言代码
     */
    QString languageCodeAt(int index) const;

signals:
    /**
     * @brief 语言切换信号
     * @param languageCode 新的语言代码
     */
    void languageChanged(const QString &languageCode);

private:
    LanguageManager();
    ~LanguageManager();
    LanguageManager(const LanguageManager &) = delete;
    LanguageManager &operator=(const LanguageManager &) = delete;

    void initSupportedLanguages();
    bool loadTranslator(const QString &languageCode);

    QVector<LanguageInfo> m_supportedLanguages;
    QString m_currentLanguage;
    QTranslator *m_appTranslator;
    QTranslator *m_qtTranslator;
};

#endif // LANGUAGEMANAGER_H
