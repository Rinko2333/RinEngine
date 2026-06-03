#ifndef LOCALIZATION_MANAGER_H
#define LOCALIZATION_MANAGER_H

#include <QObject>
#include <QHash>
#include <QString>

class LocalizationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int localeVersion READ localeVersion NOTIFY languageChanged)
    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY languageChanged)

public:
    static LocalizationManager* instance();
    explicit LocalizationManager(QObject *parent = nullptr);

    int localeVersion() const;
    QString currentLanguage() const;

    Q_INVOKABLE QString tr(const QString &key) const;

    void loadDictionary(const QString &lang);

signals:
    void languageChanged();

private:
    QHash<QString, QString> m_dict;
    QString m_currentLanguage;
    int m_localeVersion;
};

#endif // LOCALIZATION_MANAGER_H
