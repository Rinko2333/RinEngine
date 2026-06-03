#include "VariableManager.h"
#include "Logger.h"
#include <QRegularExpression>
#include <QtMath>
#include <QJsonDocument>

VariableManager* VariableManager::instance()
{
    static VariableManager inst;
    return &inst;
}

VariableManager::VariableManager(QObject *parent)
    : QObject(parent)
{
}

void VariableManager::set(const QString &name, const QVariant &value)
{
    m_variables[name] = value;
    Logger::instance()->debug("Variable", name + " = " + value.toString());
    emit variableChanged(name, value);
}

QVariant VariableManager::get(const QString &name) const
{
    return m_variables.value(name, QVariant(0));
}

void VariableManager::add(const QString &name, double delta)
{
    QVariant cur = get(name);
    double val = cur.toDouble() + delta;
    m_variables[name] = val;
    Logger::instance()->debug("Variable", name + " += " + QString::number(delta) + " = " + QString::number(val));
    Logger::instance()->debug("Variable", "snapshot: " + QJsonDocument::fromVariant(m_variables).toJson(QJsonDocument::Compact));
    emit variableChanged(name, val);
}

bool VariableManager::evaluate(const QString &condition) const
{
    if (condition.trimmed().isEmpty())
        return true;

    QString expr = condition.trimmed();

    // Handle logical OR: split by "||"
    QStringList orParts = expr.split("||", Qt::SkipEmptyParts);
    if (orParts.size() > 1) {
        for (const auto &part : orParts) {
            if (evaluate(part.trimmed()))
                return true;
        }
        return false;
    }

    // Handle logical AND: split by "&&"
    QStringList andParts = expr.split("&&", Qt::SkipEmptyParts);
    if (andParts.size() > 1) {
        for (const auto &part : andParts) {
            if (!evaluate(part.trimmed()))
                return false;
        }
        return true;
    }

    // Handle NOT prefix
    if (expr.startsWith('!')) {
        return !evaluate(expr.mid(1));
    }

    return evaluateSimple(expr);
}

bool VariableManager::evaluateSimple(const QString &cond) const
{
    static const QVector<QPair<QString, QString>> ops = {
        {">=", ">="}, {"<=", "<="}, {"==", "=="}, {"!=", "!="},
        {">", ">"}, {"<", "<"}
    };

    QString trimmed = cond.trimmed();

    for (const auto &op : ops) {
        int idx = trimmed.indexOf(op.first);
        if (idx > 0) {
            QString varName = trimmed.left(idx).trimmed();
            QString rhs = trimmed.mid(idx + op.first.length()).trimmed();

            // If rhs is "true"/"false" it's a boolean check
            if (rhs.compare("true", Qt::CaseInsensitive) == 0) {
                return get(varName).toBool() == true;
            }
            if (rhs.compare("false", Qt::CaseInsensitive) == 0) {
                return get(varName).toBool() == false;
            }

            QVariant leftVal = get(varName);
            double leftNum = leftVal.toDouble();
            double rightNum = rhs.toDouble();

            // Compare as numbers if both numeric
            if (op.first == "==")
                return qFuzzyCompare(leftNum, rightNum);
            if (op.first == "!=")
                return !qFuzzyCompare(leftNum, rightNum);
            if (op.first == ">")
                return leftNum > rightNum;
            if (op.first == "<")
                return leftNum < rightNum;
            if (op.first == ">=")
                return leftNum >= rightNum;
            if (op.first == "<=")
                return leftNum <= rightNum;
        }
    }

    // No operator found — treat as existence check (truthy)
    QString varName = trimmed;
    QVariant val = get(varName);
    if (val.typeId() == QMetaType::Bool)
        return val.toBool();
    if (val.typeId() == QMetaType::Int || val.typeId() == QMetaType::Double)
        return val.toDouble() != 0;
    return !val.toString().isEmpty();
}

QVariantMap VariableManager::snapshot() const
{
    return m_variables;
}

void VariableManager::restore(const QVariantMap &data)
{
    m_variables = data;
}

void VariableManager::reset()
{
    m_variables.clear();
    Logger::instance()->info("Variable", "All variables reset");
}
