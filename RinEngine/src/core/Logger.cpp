#include "Logger.h"

Logger* Logger::instance()
{
    static Logger inst;
    return &inst;
}

Logger::Logger()
    : QObject()
{
}

void Logger::debug(const QString &tag, const QString &message)
{
    log(LogLevel::Debug, tag, message);
}

void Logger::info(const QString &tag, const QString &message)
{
    log(LogLevel::Info, tag, message);
}

void Logger::warn(const QString &tag, const QString &message)
{
    log(LogLevel::Warn, tag, message);
}

void Logger::error(const QString &tag, const QString &message)
{
    log(LogLevel::Error, tag, message);
}

void Logger::fatal(const QString &tag, const QString &message)
{
    log(LogLevel::Fatal, tag, message);
}

QString Logger::levelString(LogLevel level)
{
    switch (level) {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info:  return "INFO";
    case LogLevel::Warn:  return "WARN";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Fatal: return "FATAL";
    }
    return "UNKNOWN";
}

void Logger::log(LogLevel level, const QString &tag, const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString formatted = QString("[%1] [%2] [%3] %4")
                            .arg(timestamp, levelString(level), tag, message);

    switch (level) {
    case LogLevel::Debug:
    case LogLevel::Info:
        qDebug().noquote() << formatted;
        break;
    case LogLevel::Warn:
        qWarning().noquote() << formatted;
        break;
    case LogLevel::Error:
    case LogLevel::Fatal:
        qCritical().noquote() << formatted;
        break;
    }

    emit logMessage(level, tag, message);
}
