#ifndef THRESHOLDSTORE_H
#define THRESHOLDSTORE_H

#include <QList>
#include <QSqlDatabase>
#include <QString>

#include <optional>

struct ThresholdRecord
{
    int id = 0;
    double temperature = 0;
    double humidity = 0;
    QString time;
};

class ThresholdStore
{
public:
    explicit ThresholdStore(const QSqlDatabase &database);

    bool ensureSchema(QString *error = nullptr) const;
    bool latest(ThresholdRecord &record, bool &found, QString *error = nullptr) const;
    bool add(double temperature, double humidity, QString *error = nullptr) const;
    bool search(std::optional<double> temperature, std::optional<double> humidity,
                QList<ThresholdRecord> &records, QString *error = nullptr) const;
    bool remove(int id, QString *error = nullptr) const;

private:
    QSqlDatabase database_;
};

#endif // THRESHOLDSTORE_H
