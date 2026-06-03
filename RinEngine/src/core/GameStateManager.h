#ifndef GAME_STATE_MANAGER_H
#define GAME_STATE_MANAGER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QVector>
#include <QImage>
#include <QDir>

class GameStateManager : public QObject
{
    Q_OBJECT
public:
    static GameStateManager* instance();

    // Save/Load operations
    Q_INVOKABLE bool save(int slot);
    Q_INVOKABLE bool load(int slot);
    Q_INVOKABLE void quickSave();
    Q_INVOKABLE void quickLoad();
    Q_INVOKABLE int autoSave();

    // Deferred load (for continue from title — waits until game screen is ready)
    Q_INVOKABLE void scheduleLoad(int slot);
    Q_INVOKABLE bool hasPendingLoad() const;
    Q_INVOKABLE void executePendingLoad();

    // Query save slots
    Q_INVOKABLE QVariantList listSaves();
    Q_INVOKABLE QVariantMap slotInfo(int slot);
    Q_INVOKABLE bool slotExists(int slot) const;
    Q_INVOKABLE bool canSave() const;
    Q_INVOKABLE bool hasAnySave() const;

    // Delete
    Q_INVOKABLE bool deleteSlot(int slot);

    // Paths
    QString savesDir() const;

signals:
    void saveCompleted(int slot);
    void loadCompleted(int slot);
    void saveFailed(int slot, const QString &error);

private:
    explicit GameStateManager(QObject *parent = nullptr);

    int findNextAutoSlot();
    QString slotFilePath(int slot) const;
    QString thumbFilePath(int slot) const;
    bool writeSlot(int slot, const QVariantMap &data);
    QVariantMap readSlot(int slot) const;

    static const int MAX_SAVE_SLOTS = 100;
    static const int QUICK_SAVE_SLOT = 98;
    static const int QUICK_LOAD_SLOT = 98;
    static const int AUTO_SAVE_SLOT_START = 80;
    static const int AUTO_SAVE_SLOT_END = 97;

    int m_pendingLoadSlot = -1;
};

#endif // GAME_STATE_MANAGER_H
