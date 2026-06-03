#include "ResourceManager.h"
#include "Logger.h"
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QUrl>

const QHash<QString, QString> ResourceManager::s_protocolMap = {
    {"bg:",   "bg/"},
    {"char:", "char/"},
    {"bgm:",  "bgm/"},
    {"se:",   "se/"},
    {"voice:","voice/"},
    {"video:","video/"},
};

const QStringList ResourceManager::s_imageExtensions = {".png", ".jpg", ".jpeg", ".bmp", ".webp"};

ResourceManager* ResourceManager::instance()
{
    static ResourceManager inst;
    return &inst;
}

ResourceManager::ResourceManager(QObject *parent)
    : QObject(parent)
{
}

QString ResourceManager::assetsBasePath()
{
    QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/assets",
        QCoreApplication::applicationDirPath() + "/../../assets",
        QDir::currentPath() + "/assets",
    };
    for (const auto &c : candidates) {
        if (QDir(c).exists()) {
            return QDir(c).absolutePath() + "/";
        }
    }
    return QDir::currentPath() + "/../../assets/";
}

QString ResourceManager::resolveProtocol(const QString &path) const
{
    for (auto it = s_protocolMap.begin(); it != s_protocolMap.end(); ++it) {
        if (path.startsWith(it.key())) {
            QString relativePath = path.mid(it.key().length());
            // If image protocol, try to find the file with extension
            if (it.key() == "bg:" || it.key() == "char:") {
                QString basePath = assetsBasePath() + it.value() + relativePath;
                return findImageFile(basePath);
            }
            return assetsBasePath() + it.value() + relativePath;
        }
    }
    // No protocol — try as image
    QString found = findImageFile(assetsBasePath() + path);
    if (!found.isEmpty()) return found;
    return assetsBasePath() + path;
}

QString ResourceManager::findImageFile(const QString &basePath) const
{
    // Check if the base path already has an extension
    if (QFileInfo::exists(basePath)) {
        return basePath;
    }
    // Try each extension
    for (const auto &ext : s_imageExtensions) {
        QString withExt = basePath + ext;
        if (QFileInfo::exists(withExt)) {
            return withExt;
        }
    }
    return basePath; // return original and let caller deal with it
}

QString ResourceManager::resolvePath(const QString &path) const
{
    return resolveProtocol(path);
}

QUrl ResourceManager::getImage(const QString &path)
{
    // Return from cache if available
    if (m_cache.contains(path)) {
        return QUrl::fromLocalFile(m_cache[path]);
    }

    QString fullPath = resolveProtocol(path);
    QFileInfo fi(fullPath);

    if (!fi.exists()) {
        Logger::instance()->warn("ResourceManager", "Image not found: " + fullPath);
        return QUrl();
    }

    // Cache the resolved file path
    m_cache.insert(path, fullPath);
    Logger::instance()->debug("ResourceManager", "Resolved image: " + path + " -> " + fullPath);

    return QUrl::fromLocalFile(fullPath);
}

void ResourceManager::preload(const QStringList &paths)
{
    for (const auto &path : paths) {
        getImage(path);
    }
    Logger::instance()->info("ResourceManager", "Preloaded " + QString::number(paths.size()) + " images");
}

void ResourceManager::release(const QString &path)
{
    m_cache.remove(path);
}

void ResourceManager::clearCache()
{
    m_cache.clear();
    Logger::instance()->info("ResourceManager", "Cache cleared");
    emit cacheCleared();
}
