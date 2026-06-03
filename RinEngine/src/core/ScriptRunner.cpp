#include "ScriptRunner.h"
#include "VariableManager.h"
#include "GalleryManager.h"
#include "Logger.h"
#include <QCoreApplication>

ScriptRunner* ScriptRunner::instance()
{
    static ScriptRunner inst;
    return &inst;
}

ScriptRunner::ScriptRunner(QObject *parent)
    : QObject(parent)
{
}

QVariantList ScriptRunner::dialogueHistoryQVariant() const
{
    QVariantList list;
    for (const auto &s : m_dialogueHistory)
        list.append(s);
    return list;
}

void ScriptRunner::setDialogueHistory(const QStringList &h)
{
    m_dialogueHistory = h;
    emit dialogueHistoryChanged();
}

void ScriptRunner::appendDialogueHistory(const QString &entry)
{
    m_dialogueHistory.append(entry);
    if (m_dialogueHistory.size() > MAX_HISTORY)
        m_dialogueHistory.removeFirst();
    emit dialogueHistoryChanged();
}

void ScriptRunner::loadScript(const QString &filePath)
{
    stop();
    m_callStack.clear();
    m_dialogueHistory.clear();
    emit dialogueHistoryChanged();
    m_visualSnapshot = VisualSnapshot();
    m_executingChildren = false;
    m_childrenList.clear();
    m_childIndex = 0;

    m_currentScriptFile = filePath;

    if (!ScriptParser::instance()->loadFile(filePath)) {
        Logger::instance()->error("ScriptRunner", "Failed to load: " + filePath);
        return;
    }
    m_commands = ScriptParser::instance()->commands();
    m_currentIndex = 0;
    setWaitState(WAIT_NONE);
    m_awaitingChoice = false;
    m_running = false;

    Logger::instance()->info("ScriptRunner",
        "Loaded " + QString::number(m_commands.size()) + " commands from " + filePath);
    emit scriptLoaded(filePath);
}

void ScriptRunner::start()
{
    if (m_commands.isEmpty()) {
        Logger::instance()->warn("ScriptRunner", "No commands loaded");
        return;
    }
    m_running = true;
    setWaitState(WAIT_NONE);
    m_awaitingChoice = false;
    m_executingChildren = false;
    m_childrenList.clear();
    m_childIndex = 0;
    emit runningChanged();

    Logger::instance()->info("ScriptRunner", "Starting script execution");
    QCoreApplication::processEvents();
    executeCurrent();
}

void ScriptRunner::resume()
{
    if (m_commands.isEmpty()) return;
    m_running = true;
    setWaitState(WAIT_CLICK);
    m_awaitingChoice = false;
    m_executingChildren = false;
    m_childrenList.clear();
    m_childIndex = 0;
    emit runningChanged();
    // Restore last dialogue text from history (no typewriter)
    if (!m_dialogueHistory.isEmpty()) {
        QString last = m_dialogueHistory.last();
        int colonPos = last.indexOf(": ");
        if (colonPos > 0)
            emit restoreDialogue(last.left(colonPos), last.mid(colonPos + 2));
        else
            emit restoreDialogue("", last);
    }
    Logger::instance()->info("ScriptRunner", "Resumed from save at index " + QString::number(m_currentIndex));
}

void ScriptRunner::stop()
{
    m_running = false;
    setWaitState(WAIT_NONE);
    m_awaitingChoice = false;
    m_executingChildren = false;
    m_childrenList.clear();
    emit runningChanged();
}

void ScriptRunner::advance()
{
    if (!m_running) return;
    if (m_waitState == WAIT_CHOICE) return;

    setWaitState(WAIT_NONE);

    // If we're in the middle of executing children (if-block), continue with next child
    if (m_executingChildren) {
        executeNextChild();
        return;
    }

    advanceToNext();
}

void ScriptRunner::selectChoice(int index)
{
    if (!m_awaitingChoice || !m_running) return;

    m_awaitingChoice = false;
    setWaitState(WAIT_NONE);

    if (m_currentIndex >= 0 && m_currentIndex < m_commands.size()) {
        const auto &cmd = m_commands[m_currentIndex];
        if (cmd.type == "select" && index >= 0 && index < cmd.choices.size()) {
            QVariantList option = cmd.choices[index].toList();
            if (option.size() >= 2) {
                QString jumpLabel = option[1].toString();
                Logger::instance()->info("ScriptRunner", "Choice: " + option[0].toString() + " -> @" + jumpLabel);
                emit hideChoices();
                jumpToLabel(jumpLabel);
                return;
            }
        }
    }

    emit hideChoices();
    advanceToNext();
}

void ScriptRunner::executeCurrent()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_commands.size()) {
        finishScript();
        return;
    }

    const auto &cmd = m_commands[m_currentIndex];
    m_currentCommand = cmd.type;
    emit commandChanged();

    executeCommand(cmd);
}

void ScriptRunner::executeCommand(const ScriptCommand &cmd)
{
    setWaitState(WAIT_NONE);

    if (cmd.type == "bg") {
        QString bgId = cmd.args.size() > 0 ? cmd.args[0] : "";
        QString transition = cmd.options.value("tr", "fade").toString();
        double duration = cmd.options.value("d", 1.0).toDouble();
        updateVisualSnapshot(cmd);
        emit showBackground(bgId, transition, duration);
        setWaitState(WAIT_CLICK);
        return;

    } else if (cmd.type == "ch") {
        QString name = cmd.args.size() > 0 ? cmd.args[0] : "";
        QString face = cmd.args.size() > 1 ? cmd.args[1] : "normal";
        QString pos = cmd.args.size() > 2 ? cmd.args[2] : "center";
        QString effect = cmd.options.value("effect", "").toString();
        double ed = cmd.options.value("d", 0.3).toDouble();
        updateVisualSnapshot(cmd);
        emit showCharacter(name, face, pos, effect, ed);
        setWaitState(WAIT_CLICK);
        return;

    } else if (cmd.type == "ch_hide") {
        QString pos = cmd.args.size() > 0 ? cmd.args[0] : "center";
        // Remove from visual snapshot
        for (int i = 0; i < m_visualSnapshot.characters.size(); i++) {
            if (m_visualSnapshot.characters[i]["position"].toString() == pos) {
                m_visualSnapshot.characters.removeAt(i);
                break;
            }
        }
        emit hideCharacter(pos);
        advanceToNext();
        return;

    } else if (cmd.type == "ch_move") {
        QString from = cmd.args.size() > 0 ? cmd.args[0] : "";
        QString to = cmd.args.size() > 1 ? cmd.args[1] : "";
        double dur = cmd.options.value("d", 0.5).toDouble();
        // Update snapshot position
        for (auto &ch : m_visualSnapshot.characters) {
            if (ch["position"].toString() == from) {
                ch["position"] = to;
                break;
            }
        }
        emit moveCharacter(from, to, dur);
        setWaitState(WAIT_CLICK);
        return;

    } else if (cmd.type == "say") {
        QString speaker = cmd.args.size() > 0 ? cmd.args[0] : "";
        QString text = cmd.args.size() > 1 ? cmd.args[1] : "";
        if (speaker.trimmed().isEmpty() && text.trimmed().isEmpty()) {
            advanceToNext();
            return;
        }
        // Record dialogue history
        QString entry = speaker.isEmpty() ? text : speaker + ": " + text;
        appendDialogueHistory(entry);
        emit showDialogue(speaker, text);
        setWaitState(WAIT_CLICK);
        return;

    } else if (cmd.type == "select") {
        m_awaitingChoice = true;
        setWaitState(WAIT_CHOICE);
        emit showChoices(cmd.choices);
        return;

    } else if (cmd.type == "if") {
        QString condition = cmd.args.size() > 0 ? cmd.args[0] : "";
        if (VariableManager::instance()->evaluate(condition)) {
            if (!cmd.children.isEmpty()) {
                beginChildren(cmd.children);
                return;
            }
        }
        advanceToNext();
        return;

    } else if (cmd.type == "jump") {
        QString label = cmd.args.size() > 0 ? cmd.args[0] : "";
        jumpToLabel(label);
        return;

    } else if (cmd.type == "call") {
        QString label = cmd.args.size() > 0 ? cmd.args[0] : "";
        m_callStack.push_back(m_currentIndex + 1);
        jumpToLabel(label);
        return;

    } else if (cmd.type == "return") {
        if (!m_callStack.isEmpty()) {
            int retAddr = m_callStack.back();
            m_callStack.pop_back();
            m_currentIndex = retAddr;
            QCoreApplication::processEvents();
            executeCurrent();
        } else {
            Logger::instance()->warn("ScriptRunner", "return without call");
            advanceToNext();
        }
        return;

    } else if (cmd.type == "set") {
        QString varName = cmd.args.size() > 0 ? cmd.args[0] : "";
        QVariant value = cmd.args.size() > 1 ? cmd.args[1] : QVariant(0);
        bool ok = false;
        double numVal = value.toString().toDouble(&ok);
        if (ok)
            VariableManager::instance()->set(varName, numVal);
        else
            VariableManager::instance()->set(varName, value);
        advanceToNext();
        return;

    } else if (cmd.type == "add") {
        QString varName = cmd.args.size() > 0 ? cmd.args[0] : "";
        double delta = cmd.args.size() > 1 ? cmd.args[1].toDouble() : 0;
        VariableManager::instance()->add(varName, delta);
        advanceToNext();
        return;

    } else if (cmd.type == "bgm") {
        QString bgmId = cmd.args.size() > 0 ? cmd.args[0] : "";
        double vol = cmd.options.value("vol", 80.0).toDouble();
        emit playBgm(bgmId, vol);
        GalleryManager::instance()->unlockBgm(bgmId);
        advanceToNext();
        return;

    } else if (cmd.type == "unlock") {
        QString category = cmd.args.size() > 0 ? cmd.args[0] : "";
        QString id = cmd.args.size() > 1 ? cmd.args[1] : "";
        if (category == "cg") {
            GalleryManager::instance()->unlockCg(id);
        } else if (category == "bgm") {
            GalleryManager::instance()->unlockBgm(id);
        }
        advanceToNext();
        return;

    } else if (cmd.type == "se") {
        QString seId = cmd.args.size() > 0 ? cmd.args[0] : "";
        emit playSe(seId);
        advanceToNext();
        return;

    } else if (cmd.type == "voice") {
        QString voiceId = cmd.args.size() > 0 ? cmd.args[0] : "";
        emit playVoice(voiceId);
        advanceToNext();
        return;

    } else if (cmd.type == "video") {
        QString videoId = cmd.args.size() > 0 ? cmd.args[0] : "";
        emit playVideo(videoId);
        setWaitState(WAIT_CLICK);
        return;

    } else if (cmd.type == "wait") {
        setWaitState(WAIT_CLICK);
        return;

    } else if (cmd.type == "label") {
        // Labels are skipped during parse, but just in case
        advanceToNext();
        return;

    } else {
        Logger::instance()->warn("ScriptRunner", "Unknown command: " + cmd.type);
        advanceToNext();
    }
}

// ===== Children execution (if-block) =====

void ScriptRunner::beginChildren(const QVector<ScriptCommand> &children)
{
    m_executingChildren = true;
    m_childrenList = children;
    m_childIndex = 0;
    executeNextChild();
}

void ScriptRunner::executeNextChild()
{
    if (m_childIndex >= m_childrenList.size()) {
        finishChildren();
        return;
    }

    const ScriptCommand &childCmd = m_childrenList[m_childIndex];
    m_childIndex++;

    const QString &childType = childCmd.type;
    const QStringList &childArgs = childCmd.args;
    const QVariantMap &childOpts = childCmd.options;

    if (childType == "say") {
        QString speaker = childArgs.size() > 0 ? childArgs[0] : "";
        QString text = childArgs.size() > 1 ? childArgs[1] : "";
        if (speaker.trimmed().isEmpty() && text.trimmed().isEmpty()) {
            executeNextChild();
            return;
        }
        QString entry = speaker.isEmpty() ? text : speaker + ": " + text;
        appendDialogueHistory(entry);
        emit showDialogue(speaker, text);
        setWaitState(WAIT_CLICK);
        return;

    } else if (childType == "set" && childArgs.size() >= 2) {
        bool ok = false;
        double numVal = childArgs[1].toDouble(&ok);
        if (ok) VariableManager::instance()->set(childArgs[0], numVal);
        else VariableManager::instance()->set(childArgs[0], childArgs[1]);
        executeNextChild();
        return;

    } else if (childType == "add" && childArgs.size() >= 2) {
        VariableManager::instance()->add(childArgs[0], childArgs[1].toDouble());
        executeNextChild();
        return;

    } else if (childType == "bg") {
        updateVisualSnapshotFromArgs(childType, childArgs, childOpts);
        QString bgId = childArgs.size() > 0 ? childArgs[0] : "";
        QString transition = childOpts.value("tr", "fade").toString();
        double duration = childOpts.value("d", 1.0).toDouble();
        emit showBackground(bgId, transition, duration);
        setWaitState(WAIT_CLICK);
        return;

    } else if (childType == "ch") {
        updateVisualSnapshotFromArgs(childType, childArgs, childOpts);
        QString name = childArgs.size() > 0 ? childArgs[0] : "";
        QString face = childArgs.size() > 1 ? childArgs[1] : "normal";
        QString pos = childArgs.size() > 2 ? childArgs[2] : "center";
        QString effect = childOpts.value("effect", "").toString();
        double ed = childOpts.value("d", 0.3).toDouble();
        emit showCharacter(name, face, pos, effect, ed);
        setWaitState(WAIT_CLICK);
        return;

    } else if (childType == "jump" && !childArgs.isEmpty()) {
        m_executingChildren = false;
        m_childrenList.clear();
        jumpToLabel(childArgs[0]);
        return;

    } else {
        executeNextChild();
    }
}

void ScriptRunner::finishChildren()
{
    m_executingChildren = false;
    m_childrenList.clear();
    m_childIndex = 0;
    // After if-block, advance to next main script command
    advanceToNext();
}

// ===== Navigation =====

void ScriptRunner::jumpToLabel(const QString &label)
{
    if (ScriptParser::instance()->hasLabel(label)) {
        m_currentIndex = ScriptParser::instance()->labelIndex(label);
        QCoreApplication::processEvents();
        executeCurrent();
    } else {
        Logger::instance()->error("ScriptRunner", "Label not found: @" + label);
        advanceToNext();
    }
}

void ScriptRunner::advanceToNext()
{
    m_currentIndex++;
    setWaitState(WAIT_NONE);
    QCoreApplication::processEvents();
    executeCurrent();
}

void ScriptRunner::finishScript()
{
    m_running = false;
    setWaitState(WAIT_NONE);
    m_executingChildren = false;
    emit runningChanged();
    emit scriptEnded();
    Logger::instance()->info("ScriptRunner", "Script ended");
}

void ScriptRunner::setWaitState(WaitState state)
{
    if (m_waitState != state) {
        m_waitState = state;
        emit canSaveChanged();
    }
}

// ===== Visual snapshot =====

void ScriptRunner::updateVisualSnapshot(const ScriptCommand &cmd)
{
    if (cmd.type == "bg") {
        m_visualSnapshot.bgId = cmd.args.size() > 0 ? cmd.args[0] : "";
    } else if (cmd.type == "ch") {
        QString name = cmd.args.size() > 0 ? cmd.args[0] : "";
        QString face = cmd.args.size() > 1 ? cmd.args[1] : "normal";
        QString pos = cmd.args.size() > 2 ? cmd.args[2] : "center";
        // Update or add character
        bool found = false;
        for (auto &ch : m_visualSnapshot.characters) {
            if (ch["position"].toString() == pos) {
                ch["name"] = name;
                ch["face"] = face;
                found = true;
                break;
            }
        }
        if (!found) {
            QVariantMap entry;
            entry["name"] = name;
            entry["face"] = face;
            entry["position"] = pos;
            m_visualSnapshot.characters.append(entry);
        }
    }
}

void ScriptRunner::updateVisualSnapshotFromArgs(const QString &type, const QStringList &args, const QVariantMap &opts)
{
    if (type == "bg" && !args.isEmpty()) {
        m_visualSnapshot.bgId = args[0];
    } else if (type == "ch" && args.size() >= 1) {
        QString name = args[0];
        QString face = args.size() > 1 ? args[1] : "normal";
        QString pos = args.size() > 2 ? args[2] : "center";
        bool found = false;
        for (auto &ch : m_visualSnapshot.characters) {
            if (ch["position"].toString() == pos) {
                ch["name"] = name;
                ch["face"] = face;
                found = true;
                break;
            }
        }
        if (!found) {
            QVariantMap entry;
            entry["name"] = name;
            entry["face"] = face;
            entry["position"] = pos;
            m_visualSnapshot.characters.append(entry);
        }
    }
}

void ScriptRunner::applyVisualSnapshot(const VisualSnapshot &snap)
{
    m_visualSnapshot = snap;
    Logger::instance()->debug("ScriptRunner", "Restoring visual: bg='" + snap.bgId + "' chars=" + QString::number(snap.characters.size()));
    emit clearDialogue();
    // Use restore signals (no advance callback) for save/load restoration
    if (!snap.bgId.isEmpty()) {
        emit restoreBackground(snap.bgId);
    }
    for (const auto &ch : snap.characters) {
        emit restoreCharacter(
            ch["name"].toString(),
            ch["face"].toString(),
            ch["position"].toString()
        );
    }
    setWaitState(WAIT_CLICK);
}
