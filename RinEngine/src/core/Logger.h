#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QDebug>

enum class LogLevel {
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger* instance();

    Q_INVOKABLE void debug(const QString &tag, const QString &message);
    Q_INVOKABLE void info(const QString &tag, const QString &message);
    Q_INVOKABLE void warn(const QString &tag, const QString &message);
    Q_INVOKABLE void error(const QString &tag, const QString &message);
    Q_INVOKABLE void fatal(const QString &tag, const QString &message);

signals:
    void logMessage(LogLevel level, const QString &tag, const QString &message);

private:
    Logger();
    void log(LogLevel level, const QString &tag, const QString &message);
    static QString levelString(LogLevel level);
};

#endif // LOGGER_H
