#ifndef SCRIPT_PARSER_H
#define SCRIPT_PARSER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QHash>

struct ScriptCommand {
    QString type;           // "bg", "ch", "say", "select", "if", etc.
    QStringList args;       // Positional arguments
    QVariantMap options;    // Named options (from the last object arg)
    QVariantList choices;   // For "select": option arrays [text, jump, condition]
    QVector<ScriptCommand> children;  // For "if": sub-commands
    int sourceLine;         // Line number in source file (0-based)
};

class ScriptParser : public QObject
{
    Q_OBJECT
public:
    static ScriptParser* instance();

    // Parse a JSON array file into command list
    Q_INVOKABLE bool loadFile(const QString &filePath);
    Q_INVOKABLE bool parse(const QVariantList &data);

    const QVector<ScriptCommand>& commands() const { return m_commands; }

    // Label map: label name -> command index
    Q_INVOKABLE int labelIndex(const QString &label) const;
    bool hasLabel(const QString &label) const;

    Q_INVOKABLE QString lastError() const { return m_lastError; }
    Q_INVOKABLE int commandCount() const { return m_commands.size(); }

signals:
    void parseError(const QString &message);

private:
    explicit ScriptParser(QObject *parent = nullptr);

    ScriptCommand parseCommand(const QVariant &item, int line);
    void buildLabelMap();

    QVector<ScriptCommand> m_commands;
    QHash<QString, int> m_labelMap;
    QString m_lastError;
};

#endif // SCRIPT_PARSER_H
