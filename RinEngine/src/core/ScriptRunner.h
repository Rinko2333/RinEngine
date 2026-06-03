#ifndef SCRIPT_RUNNER_H
#define SCRIPT_RUNNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include "ScriptParser.h"

struct VisualSnapshot {
    QString bgId;
    QVector<QVariantMap> characters;  // [{name, face, position}]
};

class ScriptRunner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString currentCommand READ currentCommand NOTIFY commandChanged)
    Q_PROPERTY(bool canSave READ canSave NOTIFY canSaveChanged)

public:
    enum WaitState {
        WAIT_NONE,
        WAIT_CLICK,
        WAIT_CHOICE,
    };
    Q_ENUM(WaitState)

    static ScriptRunner* instance();

    Q_INVOKABLE void loadScript(const QString &filePath);
    Q_INVOKABLE QString currentScriptFile() const { return m_currentScriptFile; }
    Q_INVOKABLE void start();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void advance();
    Q_INVOKABLE void selectChoice(int index);

    bool isRunning() const { return m_running; }
    QString currentCommand() const { return m_currentCommand; }
    WaitState waitState() const { return m_waitState; }

    // Execution position
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int idx) { m_currentIndex = idx; }

    // Visual snapshot for save/load
    VisualSnapshot captureVisualSnapshot() const { return m_visualSnapshot; }
    void applyVisualSnapshot(const VisualSnapshot &snap);

    // Dialogue history — Q_PROPERTY so QML can bind to it via list model
    Q_PROPERTY(QVariantList dialogueHistory READ dialogueHistoryQVariant NOTIFY dialogueHistoryChanged)
    QStringList m_dialogueHistory;
    static constexpr int MAX_HISTORY = 1000;
    QVariantList dialogueHistoryQVariant() const;
    Q_INVOKABLE QStringList dialogueHistoryStringList() const { return m_dialogueHistory; }
    void setDialogueHistory(const QStringList &h);
    void appendDialogueHistory(const QString &entry);

    // Can we save right now?
    // Q_PROPERTY with NOTIFY so QML bindings stay live
    bool canSave() const { return m_running && (m_waitState == WAIT_CLICK); }

signals:
    void showBackground(const QString &bgId, const QString &transition, double duration);
    void showCharacter(const QString &charName, const QString &face,
                       const QString &position, const QString &effect, double effectDuration);
    void hideCharacter(const QString &position);
    void moveCharacter(const QString &fromPos, const QString &toPos, double duration);
    void showDialogue(const QString &speaker, const QString &text);
    void playBgm(const QString &bgmId, double volume);
    void playSe(const QString &seId);
    void playVoice(const QString &voiceId);
    void showChoices(const QVariantList &options);
    void hideChoices();
    void playVideo(const QString &videoId);
    void scriptEnded();
    void scriptLoaded(const QString &filePath);

    // Restoration signals (no advance callback — for save/load)
    void restoreBackground(const QString &bgId);
    void restoreCharacter(const QString &charName, const QString &face, const QString &position);
    void restoreDialogue(const QString &speaker, const QString &text);
    void clearDialogue();

    void runningChanged();
    void commandChanged();
    void dialogueHistoryChanged();
    void canSaveChanged();

private:
    explicit ScriptRunner(QObject *parent = nullptr);

    void executeCurrent();
    void executeCommand(const ScriptCommand &cmd);
    void jumpToLabel(const QString &label);
    void advanceToNext();
    void finishScript();
    void setWaitState(WaitState state);

    // Children execution (for if-blocks)
    bool m_executingChildren = false;
    QVector<ScriptCommand> m_childrenList;
    int m_childIndex = 0;
    void beginChildren(const QVector<ScriptCommand> &children);
    void executeNextChild();
    void finishChildren();

    // Visual state tracking
    VisualSnapshot m_visualSnapshot;
    void updateVisualSnapshot(const ScriptCommand &cmd);
    void updateVisualSnapshotFromArgs(const QString &type, const QStringList &args, const QVariantMap &opts);

    QVector<ScriptCommand> m_commands;
    int m_currentIndex = 0;
    bool m_running = false;
    WaitState m_waitState = WAIT_NONE;
    QString m_currentCommand;

    QVector<int> m_callStack;
    bool m_awaitingChoice = false;
    QString m_currentScriptFile;
};

#endif // SCRIPT_RUNNER_H
