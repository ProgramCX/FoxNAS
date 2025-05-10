#ifndef INISETTINGS_HPP
#define INISETTINGS_HPP

#include <QObject>
#include <QSettings>
class IniSettings : public QObject
{
private:
    static QSettings settings;

public:
    static QSettings& getGlobalSettingsInstance()
    {
        static QSettings settings("config.ini", QSettings::IniFormat);
        return settings;
    }

private:
    explicit IniSettings(QObject* parent = nullptr)
        : QObject(parent)
    {}
};

#endif // INISETTINGS_HPP
