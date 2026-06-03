#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double bgmVolume READ bgmVolume WRITE setBgmVolume NOTIFY bgmVolumeChanged)
    Q_PROPERTY(double seVolume READ seVolume WRITE setSeVolume NOTIFY seVolumeChanged)
    Q_PROPERTY(double voiceVolume READ voiceVolume WRITE setVoiceVolume NOTIFY voiceVolumeChanged)
    Q_PROPERTY(double textSpeed READ textSpeed WRITE setTextSpeed NOTIFY textSpeedChanged)
    Q_PROPERTY(double autoSpeed READ autoSpeed WRITE setAutoSpeed NOTIFY autoSpeedChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen WRITE setFullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    static SettingsManager* instance();
    explicit SettingsManager(QObject *parent = nullptr);

    double bgmVolume() const;
    void setBgmVolume(double volume);

    double seVolume() const;
    void setSeVolume(double volume);

    double voiceVolume() const;
    void setVoiceVolume(double volume);

    double textSpeed() const;
    void setTextSpeed(double speed);

    double autoSpeed() const;
    void setAutoSpeed(double speed);

    bool fullscreen() const;
    void setFullscreen(bool fs);

    QString language() const;
    void setLanguage(const QString &lang);

    Q_INVOKABLE QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant()) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);

signals:
    void bgmVolumeChanged();
    void seVolumeChanged();
    void voiceVolumeChanged();
    void textSpeedChanged();
    void autoSpeedChanged();
    void fullscreenChanged();
    void languageChanged();

private:
    QSettings m_settings;
};

#endif // SETTINGS_MANAGER_H
