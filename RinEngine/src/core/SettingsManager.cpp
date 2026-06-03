#include "SettingsManager.h"
#include <QCoreApplication>

SettingsManager* SettingsManager::instance()
{
    static SettingsManager inst;
    return &inst;
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
}

double SettingsManager::bgmVolume() const
{
    return m_settings.value("audio/bgmVolume", 80.0).toDouble();
}

void SettingsManager::setBgmVolume(double volume)
{
    if (qFuzzyCompare(volume, bgmVolume())) return;
    m_settings.setValue("audio/bgmVolume", volume);
    emit bgmVolumeChanged();
}

double SettingsManager::seVolume() const
{
    return m_settings.value("audio/seVolume", 80.0).toDouble();
}

void SettingsManager::setSeVolume(double volume)
{
    if (qFuzzyCompare(volume, seVolume())) return;
    m_settings.setValue("audio/seVolume", volume);
    emit seVolumeChanged();
}

double SettingsManager::voiceVolume() const
{
    return m_settings.value("audio/voiceVolume", 80.0).toDouble();
}

void SettingsManager::setVoiceVolume(double volume)
{
    if (qFuzzyCompare(volume, voiceVolume())) return;
    m_settings.setValue("audio/voiceVolume", volume);
    emit voiceVolumeChanged();
}

double SettingsManager::textSpeed() const
{
    return m_settings.value("game/textSpeed", 50.0).toDouble();
}

void SettingsManager::setTextSpeed(double speed)
{
    if (qFuzzyCompare(speed, textSpeed())) return;
    m_settings.setValue("game/textSpeed", speed);
    emit textSpeedChanged();
}

double SettingsManager::autoSpeed() const
{
    return m_settings.value("game/autoSpeed", 3.0).toDouble();
}

void SettingsManager::setAutoSpeed(double speed)
{
    if (qFuzzyCompare(speed, autoSpeed())) return;
    m_settings.setValue("game/autoSpeed", speed);
    emit autoSpeedChanged();
}

bool SettingsManager::fullscreen() const
{
    return m_settings.value("display/fullscreen", false).toBool();
}

void SettingsManager::setFullscreen(bool fs)
{
    if (fs == fullscreen()) return;
    m_settings.setValue("display/fullscreen", fs);
    emit fullscreenChanged();
}

QString SettingsManager::language() const
{
    return m_settings.value("locale/language", "zh").toString();
}

void SettingsManager::setLanguage(const QString &lang)
{
    if (lang == language()) return;
    m_settings.setValue("locale/language", lang);
    emit languageChanged();
}

QVariant SettingsManager::getValue(const QString &key, const QVariant &defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void SettingsManager::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
}
