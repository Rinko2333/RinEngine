#ifndef VARIABLE_MANAGER_H
#define VARIABLE_MANAGER_H

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QString>

class VariableManager : public QObject
{
    Q_OBJECT
public:
    static VariableManager* instance();

    Q_INVOKABLE void set(const QString &name, const QVariant &value);
    Q_INVOKABLE QVariant get(const QString &name) const;
    Q_INVOKABLE void add(const QString &name, double delta);

    // Evaluate condition like "affection > 10", "flag_met==true", "money>=100 && has_key"
    Q_INVOKABLE bool evaluate(const QString &condition) const;

    // Serialization for save/load
    QVariantMap snapshot() const;
    void restore(const QVariantMap &data);

    Q_INVOKABLE void reset();

signals:
    void variableChanged(const QString &name, const QVariant &value);

private:
    explicit VariableManager(QObject *parent = nullptr);
    bool evaluateSimple(const QString &cond) const;
    QString simplifyLogical(const QString &expr) const;

    QVariantMap m_variables;
};

#endif // VARIABLE_MANAGER_H
