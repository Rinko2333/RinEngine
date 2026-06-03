#include "GalleryManager.h"
#include "Logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

GalleryManager* GalleryManager::instance()
{
    static GalleryManager inst;
    return &inst;
}

GalleryManager::GalleryManager(QObject *parent)
    : QObject(parent)
{
    load();
}

static QString galleryFilePath()
{
    QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/saves/gallery.json",
        QCoreApplication::applicationDirPath() + "/../../saves/gallery.json",
        QDir::currentPath() + "/saves/gallery.json",
    };
    for (const auto &c : candidates) {
        QFileInfo fi(c);
        if (fi.dir().exists() || QDir().mkpath(fi.absolutePath()))
            return c;
    }
    return QDir::currentPath() + "/saves/gallery.json";
}

void GalleryManager::load()
{
    QString path = galleryFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->info("GalleryManager", "No existing gallery data (first run)");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    for (const auto &v : root["cgs"].toArray())
        m_unlockedCgs.insert(v.toString());
    for (const auto &v : root["bgms"].toArray())
        m_unlockedBgms.insert(v.toString());

    Logger::instance()->info("GalleryManager",
        QString("Loaded %1 cgs, %2 bgms").arg(m_unlockedCgs.size()).arg(m_unlockedBgms.size()));
}

void GalleryManager::save()
{
    QJsonObject root;
    QJsonArray cgs;
    for (const auto &id : m_unlockedCgs)
        cgs.append(id);
    root["cgs"] = cgs;

    QJsonArray bgms;
    for (const auto &id : m_unlockedBgms)
        bgms.append(id);
    root["bgms"] = bgms;

    QJsonDocument doc(root);

    QString path = galleryFilePath();
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        Logger::instance()->debug("GalleryManager", "Saved gallery data");
    } else {
        Logger::instance()->warn("GalleryManager", "Failed to save gallery: " + path);
    }
}

void GalleryManager::unlockCg(const QString &cgId)
{
    if (cgId.isEmpty() || m_unlockedCgs.contains(cgId))
        return;
    m_unlockedCgs.insert(cgId);
    Logger::instance()->info("GalleryManager", "CG unlocked: " + cgId);
    save();
    emit cgUnlocked(cgId);
}

void GalleryManager::unlockBgm(const QString &bgmId)
{
    if (bgmId.isEmpty() || m_unlockedBgms.contains(bgmId))
        return;
    m_unlockedBgms.insert(bgmId);
    Logger::instance()->info("GalleryManager", "BGM unlocked: " + bgmId);
    save();
    emit bgmUnlocked(bgmId);
}

bool GalleryManager::isCgUnlocked(const QString &cgId) const
{
    return m_unlockedCgs.contains(cgId);
}

bool GalleryManager::isBgmUnlocked(const QString &bgmId) const
{
    return m_unlockedBgms.contains(bgmId);
}

QStringList GalleryManager::unlockedCgs() const
{
    QStringList list;
    for (const auto &id : m_unlockedCgs)
        list.append(id);
    list.sort();
    return list;
}

QStringList GalleryManager::unlockedBgms() const
{
    QStringList list;
    for (const auto &id : m_unlockedBgms)
        list.append(id);
    list.sort();
    return list;
}
