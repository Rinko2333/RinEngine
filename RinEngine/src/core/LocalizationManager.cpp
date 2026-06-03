#include "LocalizationManager.h"
#include "SettingsManager.h"
#include "Logger.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

LocalizationManager::LocalizationManager(QObject *parent)
    : QObject(parent)
    , m_localeVersion(0)
{
    QString lang = SettingsManager::instance()->language();
    loadDictionary(lang);

    connect(SettingsManager::instance(), &SettingsManager::languageChanged,
            this, [this]() {
        loadDictionary(SettingsManager::instance()->language());
    });
}

LocalizationManager* LocalizationManager::instance()
{
    static LocalizationManager inst;
    return &inst;
}

int LocalizationManager::localeVersion() const
{
    return m_localeVersion;
}

QString LocalizationManager::currentLanguage() const
{
    return m_currentLanguage;
}

QString LocalizationManager::tr(const QString &key) const
{
    if (m_dict.contains(key))
        return m_dict[key];
    return key;
}

void LocalizationManager::loadDictionary(const QString &lang)
{
    if (lang == m_currentLanguage)
        return;

    m_dict.clear();

    // Try Qt resource first, then filesystem
    QString path = ":/l10n/" + lang + ".json";
    QFile file(path);
    if (!file.exists()) {
        path = "l10n/" + lang + ".json";
        file.setFileName(path);
    }
    if (!file.exists()) {
        path = QDir::currentPath() + "/l10n/" + lang + ".json";
        file.setFileName(path);
    }

    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->warn("L10n", "Cannot open dictionary: " + path);
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        Logger::instance()->error("L10n", "JSON parse error in " + path + ": " + error.errorString());
        return;
    }

    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        m_dict.insert(it.key(), it.value().toString());
    }

    m_currentLanguage = lang;
    m_localeVersion++;
    Logger::instance()->info("L10n", "Loaded dictionary: " + lang + " (" + QString::number(m_dict.size()) + " keys)");
    emit languageChanged();
}
