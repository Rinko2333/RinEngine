#include "ScriptParser.h"
#include "Logger.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

ScriptParser* ScriptParser::instance()
{
    static ScriptParser inst;
    return &inst;
}

ScriptParser::ScriptParser(QObject *parent)
    : QObject(parent)
{
}

bool ScriptParser::loadFile(const QString &filePath)
{
    m_commands.clear();
    m_labelMap.clear();
    m_lastError.clear();

    // Resolve path relative to scripts/ directory
    QString fullPath = filePath;
    if (!QFileInfo::exists(fullPath)) {
        // Try under executable dir or current dir
        QStringList candidates = {
            filePath,
            QCoreApplication::applicationDirPath() + "/" + filePath,
            QCoreApplication::applicationDirPath() + "/../../" + filePath,
            QDir::currentPath() + "/" + filePath,
            QDir::currentPath() + "/../../" + filePath,
        };
        for (const auto &c : candidates) {
            if (QFileInfo::exists(c)) {
                fullPath = c;
                break;
            }
        }
    }

    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = "Cannot open file: " + fullPath;
        Logger::instance()->error("ScriptParser", m_lastError);
        emit parseError(m_lastError);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError) {
        m_lastError = "JSON parse error: " + err.errorString();
        Logger::instance()->error("ScriptParser", m_lastError);
        emit parseError(m_lastError);
        return false;
    }

    if (!doc.isArray()) {
        m_lastError = "Script must be a JSON array";
        Logger::instance()->error("ScriptParser", m_lastError);
        emit parseError(m_lastError);
        return false;
    }

    QVariantList list = doc.array().toVariantList();
    return parse(list);
}

bool ScriptParser::parse(const QVariantList &data)
{
    m_commands.clear();
    m_labelMap.clear();
    m_lastError.clear();

    // First pass: create a regular array where we also inline "if" children
    // and record label positions
    QVector<ScriptCommand> rawCommands;

    for (int i = 0; i < data.size(); i++) {
        const QVariant &item = data[i];
        if (!item.isValid() || item.isNull())
            continue;

        // Handle label specially: ["label", "scene_01"]
        if (item.canConvert<QVariantList>()) {
            QVariantList arr = item.toList();
            if (arr.size() >= 2 && arr[0].toString() == "label") {
                QString labelName = arr[1].toString();
                // Label points to the NEXT command
                m_labelMap[labelName] = rawCommands.size();
                Logger::instance()->debug("ScriptParser", "Label @" + labelName + " -> command " + QString::number(rawCommands.size()));
                continue;  // label itself is not a command
            }
        }

        ScriptCommand cmd = parseCommand(item, i);
        rawCommands.append(cmd);
    }

    // Second pass: resolve "label" entries that were stored as commands
    // (if first pass stored them, but we skipped labels above)
    m_commands = rawCommands;

    Logger::instance()->info("ScriptParser",
        QString("Parsed %1 commands, %2 labels")
            .arg(m_commands.size())
            .arg(m_labelMap.size()));

    return true;
}

ScriptCommand ScriptParser::parseCommand(const QVariant &item, int line)
{
    ScriptCommand cmd;
    cmd.sourceLine = line;

    if (!item.canConvert<QVariantList>())
        return cmd;

    QVariantList arr = item.toList();
    if (arr.isEmpty()) return cmd;

    cmd.type = arr[0].toString();

    // Process remaining elements
    for (int i = 1; i < arr.size(); i++) {
        const QVariant &arg = arr[i];

        if (arg.canConvert<QVariantMap>()) {
            // Named options object
            cmd.options = arg.toMap();
        } else if (cmd.type == "select" && arg.canConvert<QVariantList>()) {
            // Select options: keep as QVariantList for QML consumption
            cmd.choices = arg.toList();
        } else if (cmd.type == "if" && arg.canConvert<QVariantList>()) {
            // If children: recursively parse each one as ScriptCommand
            QVariantList rawChildren = arg.toList();
            for (const auto &childItem : rawChildren) {
                ScriptCommand childCmd = parseCommand(childItem, line);
                if (!childCmd.type.isEmpty())
                    cmd.children.append(childCmd);
            }
        } else {
            // Positional argument
            cmd.args.append(arg.toString());
        }
    }

    return cmd;
}

void ScriptParser::buildLabelMap()
{
    // Already done during parse()
}

int ScriptParser::labelIndex(const QString &label) const
{
    return m_labelMap.value(label, -1);
}

bool ScriptParser::hasLabel(const QString &label) const
{
    return m_labelMap.contains(label);
}
