#ifndef GALLERY_MANAGER_H
#define GALLERY_MANAGER_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

class GalleryManager : public QObject
{
    Q_OBJECT

public:
    static GalleryManager* instance();

    Q_INVOKABLE void unlockCg(const QString &cgId);
    Q_INVOKABLE void unlockBgm(const QString &bgmId);
    Q_INVOKABLE bool isCgUnlocked(const QString &cgId) const;
    Q_INVOKABLE bool isBgmUnlocked(const QString &bgmId) const;
    Q_INVOKABLE QStringList unlockedCgs() const;
    Q_INVOKABLE QStringList unlockedBgms() const;

signals:
    void cgUnlocked(const QString &cgId);
    void bgmUnlocked(const QString &bgmId);

private:
    explicit GalleryManager(QObject *parent = nullptr);

    void load();
    void save();

    QSet<QString> m_unlockedCgs;
    QSet<QString> m_unlockedBgms;
};

#endif // GALLERY_MANAGER_H
