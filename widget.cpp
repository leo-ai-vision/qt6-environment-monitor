#include "widget.h"

#include "historydialog.h"
#include "monitorlogic.h"
#include "setdialog.h"
#include "thresholdstore.h"
#include "ui_widget.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSqlError>
#include <QStandardPaths>

namespace {
const char *connectionName = "environment-monitor-main";
}

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(ui->tempSlider, &QSlider::valueChanged, this, &Widget::refreshDisplay);
    connect(ui->humSlider, &QSlider::valueChanged, this, &Widget::refreshDisplay);
    connect(ui->btnSet, &QPushButton::clicked, this, &Widget::openSetDialog);
    connect(ui->btnHistory, &QPushButton::clicked, this, &Widget::openHistory);

    const bool databaseReady = initializeDatabase();
    ui->btnSet->setEnabled(databaseReady);
    ui->btnHistory->setEnabled(databaseReady);
    if (databaseReady)
        loadLastThreshold();
    updateThresholdLabels();
    refreshDisplay();
}

Widget::~Widget()
{
    delete ui;
    store_.reset();
    if (database_.isValid()) {
        database_.close();
        database_ = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool Widget::initializeDatabase()
{
    const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDirectory.isEmpty() || !QDir().mkpath(dataDirectory)) {
        QMessageBox::critical(this, tr("数据库错误"), tr("无法创建应用数据目录。"));
        return false;
    }

    const QString databasePath = QDir(dataDirectory).filePath("env.db");
    if (!QFile::exists(databasePath)) {
        // Upgrade the old project when an env.db is next to the running program.
        const QStringList oldPaths = {
            QDir::current().filePath("env.db"),
            QDir(QCoreApplication::applicationDirPath()).filePath("env.db")
        };
        for (const QString &oldPath : oldPaths) {
            if (!QFile::exists(oldPath))
                continue;
            if (!QFile::copy(oldPath, databasePath)) {
                QMessageBox::warning(this, tr("迁移旧记录失败"),
                                     tr("无法复制旧数据库：%1\n程序会使用新的空数据库。")
                                         .arg(oldPath));
            }
            break;
        }
    }

    database_ = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    database_.setDatabaseName(databasePath);
    if (!database_.open()) {
        QMessageBox::critical(this, tr("数据库错误"),
                              tr("无法打开阈值数据库：%1").arg(database_.lastError().text()));
        return false;
    }

    store_ = std::make_unique<ThresholdStore>(database_);
    QString error;
    if (!store_->ensureSchema(&error)) {
        QMessageBox::critical(this, tr("数据库错误"), tr("无法初始化阈值记录：%1").arg(error));
        store_.reset();
        return false;
    }
    return true;
}

void Widget::loadLastThreshold()
{
    ThresholdRecord record;
    bool found = false;
    QString error;
    if (!store_->latest(record, found, &error)) {
        QMessageBox::warning(this, tr("读取失败"), tr("无法读取最新阈值：%1").arg(error));
        return;
    }

    temperatureThreshold_ = found ? record.temperature : 30.0;
    humidityThreshold_ = found ? record.humidity : 80.0;
}

void Widget::updateThresholdLabels()
{
    ui->labelTempTh->setText(tr("当前阈值：%1 °C").arg(temperatureThreshold_));
    ui->labelHumTh->setText(tr("当前阈值：%1 %").arg(humidityThreshold_));
}

void Widget::refreshDisplay()
{
    const int temperature = ui->tempSlider->value();
    const int humidity = ui->humSlider->value();
    const AlarmFlags alarm = evaluateAlarm(temperature, humidity,
                                           temperatureThreshold_, humidityThreshold_);

    ui->labelTempValue->setText(tr("%1 °C").arg(temperature));
    ui->labelHumValue->setText(tr("%1 %").arg(humidity));
    ui->labelTempValue->setStyleSheet(alarm.temperatureExceeded ? "color: #d32f2f; font-weight: bold;" : "");
    ui->labelHumValue->setStyleSheet(alarm.humidityExceeded ? "color: #d32f2f; font-weight: bold;" : "");

    if (alarm.temperatureExceeded && alarm.humidityExceeded)
        ui->statusLabel->setText(tr("警报：温度和湿度均超过阈值"));
    else if (alarm.temperatureExceeded)
        ui->statusLabel->setText(tr("警报：温度超过阈值"));
    else if (alarm.humidityExceeded)
        ui->statusLabel->setText(tr("警报：湿度超过阈值"));
    else
        ui->statusLabel->setText(tr("状态正常：当前读数未超过阈值"));

    ui->statusLabel->setStyleSheet((alarm.temperatureExceeded || alarm.humidityExceeded)
                                       ? "color: #d32f2f; font-weight: bold;"
                                       : "");
}

void Widget::openSetDialog()
{
    SetDialog dialog(temperatureThreshold_, humidityThreshold_, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString error;
    if (!store_->add(dialog.temperature(), dialog.humidity(), &error)) {
        QMessageBox::warning(this, tr("保存失败"), tr("无法保存阈值：%1").arg(error));
        return;
    }

    temperatureThreshold_ = dialog.temperature();
    humidityThreshold_ = dialog.humidity();
    updateThresholdLabels();
    refreshDisplay();
}

void Widget::openHistory()
{
    HistoryDialog dialog(store_.get(), this);
    dialog.exec();
    loadLastThreshold();
    updateThresholdLabels();
    refreshDisplay();
}
