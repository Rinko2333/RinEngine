#include "GameStateManager.h"
#include "ScriptRunner.h"
#include "VariableManager.h"
#include "Logger.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDateTime>
#include <QStandardPaths>

GameStateManager* GameStateManager::instance()
{
    static GameStateManager inst;
    return &inst;
}

GameStateManager::GameStateManager(QObject *parent)
    : QObject(parent)
{
}

QString GameStateManager::savesDir() const
{
    // Use a "saves" directory relative to the executable
    QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/saves",
        QCoreApplication::applicationDirPath() + "/../../saves",
        QDir::currentPath() + "/saves",
    };
    for (const auto &c : candidates) {
        QDir dir(c);
        if (dir.exists())
            return dir.absolutePath();
    }
    // Create on first use
    QDir dir(QCoreApplication::applicationDirPath() + "/saves");
    if (!dir.exists()) dir.mkpath(".");
    return dir.absolutePath();
}

QString GameStateManager::slotFilePath(int slot) const
{
    return savesDir() + QString("/save_%1.json").arg(slot, 2, 10, QChar('0'));
}

QString GameStateManager::thumbFilePath(int slot) const
{
    return savesDir() + QString("/save_%1_thumb.png").arg(slot, 2, 10, QChar('0'));
}

bool GameStateManager::canSave() const
{
    return ScriptRunner::instance()->canSave();
}

bool GameStateManager::slotExists(int slot) const
{
    return QFileInfo::exists(slotFilePath(slot));
}

bool GameStateManager::hasAnySave() const
{
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (QFileInfo::exists(slotFilePath(i)))
            return true;
    }
    return false;
}

QVariantMap GameStateManager::slotInfo(int slot)
{
    QVariantMap info;
    info["slot"] = slot;
    info["exists"] = false;

    QString path = slotFilePath(slot);
    if (!QFileInfo::exists(path))
        return info;

    QVariantMap data = readSlot(slot);
    if (data.isEmpty())
        return info;

    info["exists"] = true;
    info["timestamp"] = data.value("timestamp", "");
    info["sceneName"] = data.value("sceneName", "");
    info["scriptFile"] = data.value("scriptFile", "");
    info["commandIndex"] = data.value("commandIndex", 0);
    info["thumbPath"] = thumbFilePath(slot);
    return info;
}

QVariantList GameStateManager::listSaves()
{
    QVariantList saves;
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        QVariantMap info = slotInfo(i);
        if (info["exists"].toBool())
            saves.append(info);
    }
    return saves;
}

bool GameStateManager::save(int slot)
{
    if (!canSave()) {
        emit saveFailed(slot, "Cannot save now — not in a wait state");
        return false;
    }

    QVariantMap data;
    data["version"] = 1;
    data["timestamp"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    data["scriptFile"] = ScriptRunner::instance()->currentScriptFile();
    data["commandIndex"] = ScriptRunner::instance()->currentIndex();
    data["variables"] = VariableManager::instance()->snapshot();
    data["dialogueHistory"] = QVariant::fromValue(ScriptRunner::instance()->dialogueHistoryStringList());

    // Visual snapshot
    VisualSnapshot vis = ScriptRunner::instance()->captureVisualSnapshot();
    data["bgId"] = vis.bgId;
    QVariantList chars;
    for (const auto &ch : vis.characters) {
        chars.append(ch);
    }
    data["characters"] = chars;

    if (!writeSlot(slot, data))
        return false;

    Logger::instance()->info("GameState", "Saved to slot " + QString::number(slot));
    emit saveCompleted(slot);
    return true;
}

bool GameStateManager::load(int slot)
{
    QVariantMap data = readSlot(slot);
    if (data.isEmpty()) {
        emit saveFailed(slot, "Save data not found or corrupted");
        return false;
    }

    int commandIndex = data.value("commandIndex", 0).toInt();

    // Stop current execution
    ScriptRunner::instance()->stop();

    // Re-load the script first (clears state)
    QString scriptFile = data.value("scriptFile", "scripts/demo/script.json").toString();
    ScriptRunner::instance()->loadScript(scriptFile);

    // Restore variables
    VariableManager::instance()->restore(data.value("variables").toMap());

    // Restore dialogue history (after loadScript cleared it)
    QStringList history;
    QVariantList histList = data.value("dialogueHistory").toList();
    for (const auto &h : histList)
        history.append(h.toString());
    ScriptRunner::instance()->setDialogueHistory(history);

    // Restore visual state
    VisualSnapshot vis;
    vis.bgId = data.value("bgId").toString();
    QVariantList chars = data.value("characters").toList();
    for (const auto &ch : chars) {
        vis.characters.append(ch.toMap());
    }
    ScriptRunner::instance()->applyVisualSnapshot(vis);

    // Resume from saved position without re-executing commands
    ScriptRunner::instance()->setCurrentIndex(commandIndex);
    ScriptRunner::instance()->resume();

    Logger::instance()->info("GameState", "Loaded slot " + QString::number(slot));
    emit loadCompleted(slot);
    return true;
}

void GameStateManager::quickSave()
{
    save(QUICK_SAVE_SLOT);
}

void GameStateManager::quickLoad()
{
    if (slotExists(QUICK_LOAD_SLOT))
        load(QUICK_LOAD_SLOT);
}

void GameStateManager::scheduleLoad(int slot)
{
    m_pendingLoadSlot = slot;
}

bool GameStateManager::hasPendingLoad() const
{
    return m_pendingLoadSlot >= 0 && slotExists(m_pendingLoadSlot);
}

void GameStateManager::executePendingLoad()
{
    int slot = m_pendingLoadSlot;
    m_pendingLoadSlot = -1;
    load(slot);
}

int GameStateManager::autoSave()
{
    int slot = findNextAutoSlot();
    save(slot);
    return slot;
}

bool GameStateManager::deleteSlot(int slot)
{
    QFile::remove(slotFilePath(slot));
    QFile::remove(thumbFilePath(slot));
    Logger::instance()->info("GameState", "Deleted slot " + QString::number(slot));
    return true;
}

int GameStateManager::findNextAutoSlot()
{
    // Cycle through auto-save slots
    static int autoCounter = AUTO_SAVE_SLOT_START;
    int slot = autoCounter;
    autoCounter++;
    if (autoCounter > AUTO_SAVE_SLOT_END)
        autoCounter = AUTO_SAVE_SLOT_START;
    return slot;
}

bool GameStateManager::writeSlot(int slot, const QVariantMap &data)
{
    QString path = slotFilePath(slot);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit saveFailed(slot, "Cannot write: " + path);
        return false;
    }

    QJsonDocument doc(QJsonObject::fromVariantMap(data));
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QVariantMap GameStateManager::readSlot(int slot) const
{
    QString path = slotFilePath(slot);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QVariantMap();

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject())
        return QVariantMap();

    return doc.object().toVariantMap();
}
