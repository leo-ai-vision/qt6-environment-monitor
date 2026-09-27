#include "monitorlogic.h"
#include "thresholdstore.h"

#include <QSqlDatabase>
#include <QtTest>

class ThresholdTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void emptyHistoryUsesDefaults();
    void saveSearchAndDelete();
    void alarmTriggersOnlyAboveLimit();

private:
    QSqlDatabase database_;
    ThresholdStore *store_ = nullptr;
};

void ThresholdTests::initTestCase()
{
    database_ = QSqlDatabase::addDatabase("QSQLITE", "threshold-tests");
    database_.setDatabaseName(":memory:");
    QVERIFY(database_.open());
    store_ = new ThresholdStore(database_);
    QString error;
    QVERIFY2(store_->ensureSchema(&error), qPrintable(error));
}

void ThresholdTests::cleanupTestCase()
{
    delete store_;
    store_ = nullptr;
    database_.close();
    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase("threshold-tests");
}

void ThresholdTests::emptyHistoryUsesDefaults()
{
    ThresholdRecord record;
    bool found = true;
    QString error;
    QVERIFY2(store_->latest(record, found, &error), qPrintable(error));
    QVERIFY(!found);
}

void ThresholdTests::saveSearchAndDelete()
{
    QString error;
    QVERIFY2(store_->add(35.0, 80.0, &error), qPrintable(error));
    QVERIFY2(store_->add(36.0, 81.0, &error), qPrintable(error));

    ThresholdRecord latest;
    bool found = false;
    QVERIFY2(store_->latest(latest, found, &error), qPrintable(error));
    QVERIFY(found);
    QCOMPARE(latest.temperature, 36.0);
    QCOMPARE(latest.humidity, 81.0);

    QList<ThresholdRecord> records;
    QVERIFY2(store_->search(35.0, std::nullopt, records, &error), qPrintable(error));
    QCOMPARE(records.size(), 1);
    QCOMPARE(records.at(0).humidity, 80.0);

    const int firstId = records.at(0).id;
    QVERIFY2(store_->remove(firstId, &error), qPrintable(error));
    QVERIFY2(store_->search(35.0, std::nullopt, records, &error), qPrintable(error));
    QCOMPARE(records.size(), 0);
}

void ThresholdTests::alarmTriggersOnlyAboveLimit()
{
    const AlarmFlags atLimit = evaluateAlarm(30, 80, 30, 80);
    QVERIFY(!atLimit.temperatureExceeded);
    QVERIFY(!atLimit.humidityExceeded);

    const AlarmFlags temperatureHigh = evaluateAlarm(31, 80, 30, 80);
    QVERIFY(temperatureHigh.temperatureExceeded);
    QVERIFY(!temperatureHigh.humidityExceeded);

    const AlarmFlags bothHigh = evaluateAlarm(31, 81, 30, 80);
    QVERIFY(bothHigh.temperatureExceeded);
    QVERIFY(bothHigh.humidityExceeded);
}

QTEST_GUILESS_MAIN(ThresholdTests)
#include "test_thresholds.moc"
