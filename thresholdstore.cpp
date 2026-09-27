#include "thresholdstore.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>

namespace {
bool reportQueryError(const QSqlQuery &query, QString *error)
{
    if (error)
        *error = query.lastError().text();
    return false;
}

ThresholdRecord readRecord(const QSqlQuery &query)
{
    ThresholdRecord record;
    record.id = query.value(0).toInt();
    record.temperature = query.value(1).toDouble();
    record.humidity = query.value(2).toDouble();
    record.time = query.value(3).toString();
    return record;
}
}

ThresholdStore::ThresholdStore(const QSqlDatabase &database)
    : database_(database)
{
}

bool ThresholdStore::ensureSchema(QString *error) const
{
    QSqlQuery query(database_);
    if (!query.exec("CREATE TABLE IF NOT EXISTS threshold ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "temp REAL NOT NULL, hum REAL NOT NULL, time TEXT NOT NULL)"))
        return reportQueryError(query, error);
    return true;
}

bool ThresholdStore::latest(ThresholdRecord &record, bool &found, QString *error) const
{
    found = false;
    QSqlQuery query(database_);
    if (!query.exec("SELECT id, temp, hum, time FROM threshold ORDER BY id DESC LIMIT 1"))
        return reportQueryError(query, error);
    if (query.next()) {
        record = readRecord(query);
        found = true;
    }
    return true;
}

bool ThresholdStore::add(double temperature, double humidity, QString *error) const
{
    QSqlQuery query(database_);
    if (!query.prepare("INSERT INTO threshold (temp, hum, time) VALUES (?, ?, ?)"))
        return reportQueryError(query, error);
    query.addBindValue(temperature);
    query.addBindValue(humidity);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!query.exec())
        return reportQueryError(query, error);
    return true;
}

bool ThresholdStore::search(std::optional<double> temperature,
                            std::optional<double> humidity,
                            QList<ThresholdRecord> &records, QString *error) const
{
    records.clear();
    QString statement = "SELECT id, temp, hum, time FROM threshold WHERE 1=1";
    if (temperature)
        statement += " AND temp = :temp";
    if (humidity)
        statement += " AND hum = :hum";
    statement += " ORDER BY id DESC";

    QSqlQuery query(database_);
    if (!query.prepare(statement))
        return reportQueryError(query, error);
    if (temperature)
        query.bindValue(":temp", *temperature);
    if (humidity)
        query.bindValue(":hum", *humidity);
    if (!query.exec())
        return reportQueryError(query, error);

    while (query.next())
        records.append(readRecord(query));
    return true;
}

bool ThresholdStore::remove(int id, QString *error) const
{
    QSqlQuery query(database_);
    if (!query.prepare("DELETE FROM threshold WHERE id = :id"))
        return reportQueryError(query, error);
    query.bindValue(":id", id);
    if (!query.exec())
        return reportQueryError(query, error);
    return true;
}
