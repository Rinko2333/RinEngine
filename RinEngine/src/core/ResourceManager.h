#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QUrl>

class ResourceManager : public QObject
{
    Q_OBJECT
public:
    static ResourceManager* instance();

    Q_INVOKABLE QUrl getImage(const QString &path);
    Q_INVOKABLE QUrl getAudio(const QString &path);
    Q_INVOKABLE QUrl getVideo(const QString &path);
    Q_INVOKABLE void preload(const QStringList &paths);
    Q_INVOKABLE void release(const QString &path);
    Q_INVOKABLE void clearCache();
    Q_INVOKABLE QString resolvePath(const QString &path) const;

    static QString assetsBasePath();

signals:
    void cacheCleared();

private:
    explicit ResourceManager(QObject *parent = nullptr);

    QString resolveProtocol(const QString &path) const;
    QString findImageFile(const QString &basePath) const;
    QString findAudioFile(const QString &basePath) const;
    QString findVideoFile(const QString &basePath) const;
    QString findFileByExtensions(const QString &basePath, const QStringList &extensions) const;

    // Cache key (protocol path "bg:school") -> full file path
    QHash<QString, QString> m_cache;

    static const QHash<QString, QString> s_protocolMap;
    static const QStringList s_imageExtensions;
    static const QStringList s_audioExtensions;
    static const QStringList s_videoExtensions;
};

#endif // RESOURCE_MANAGER_H
