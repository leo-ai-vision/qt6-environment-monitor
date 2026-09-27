#include "historydialog.h"

#include "thresholdstore.h"
#include "ui_historydialog.h"

#include <QDoubleValidator>
#include <QHeaderView>
#include <QLocale>
#include <QMessageBox>
#include <QTableWidgetItem>

#include <optional>

namespace {
std::optional<double> readFilter(const QString &text, double maximum, bool &valid)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return std::nullopt;

    bool parsed = false;
    const double value = QLocale::c().toDouble(trimmed, &parsed);
    valid = parsed && value >= 0 && value <= maximum;
    if (!valid)
        return std::nullopt;
    return value;
}

QTableWidgetItem *readOnlyItem(const QString &text)
{
    auto *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}
}

HistoryDialog::HistoryDialog(ThresholdStore *store, QWidget *parent)
    : QDialog(parent), ui(new Ui::HistoryDialog), store_(store)
{
    ui->setupUi(this);

    auto *temperatureValidator = new QDoubleValidator(0, 50, 1, this);
    temperatureValidator->setLocale(QLocale::c());
    ui->tempEdit->setValidator(temperatureValidator);
    auto *humidityValidator = new QDoubleValidator(0, 100, 1, this);
    humidityValidator->setLocale(QLocale::c());
    ui->humEdit->setValidator(humidityValidator);

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->searchBtn, &QPushButton::clicked, this, &HistoryDialog::searchData);
    connect(ui->resetBtn, &QPushButton::clicked, this, &HistoryDialog::resetFilters);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &HistoryDialog::deleteSelected);
    searchData();
}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::searchData()
{
    bool valid = true;
    const std::optional<double> temperature = readFilter(ui->tempEdit->text(), 50, valid);
    if (!valid) {
        QMessageBox::warning(this, tr("输入无效"), tr("温度请填写 0 到 50 之间的数字，或留空。"));
        return;
    }
    const std::optional<double> humidity = readFilter(ui->humEdit->text(), 100, valid);
    if (!valid) {
        QMessageBox::warning(this, tr("输入无效"), tr("湿度请填写 0 到 100 之间的数字，或留空。"));
        return;
    }

    QList<ThresholdRecord> records;
    QString error;
    if (!store_->search(temperature, humidity, records, &error)) {
        QMessageBox::warning(this, tr("查询失败"), tr("无法读取历史记录：%1").arg(error));
        return;
    }

    ui->tableWidget->setRowCount(records.size());
    for (int row = 0; row < records.size(); ++row) {
        const ThresholdRecord &record = records.at(row);
        ui->tableWidget->setItem(row, 0, readOnlyItem(QString::number(record.id)));
        ui->tableWidget->setItem(row, 1, readOnlyItem(QString::number(record.temperature)));
        ui->tableWidget->setItem(row, 2, readOnlyItem(QString::number(record.humidity)));
        ui->tableWidget->setItem(row, 3, readOnlyItem(record.time));
    }
    ui->resultLabel->setText(tr("共 %1 条记录").arg(records.size()));
}

void HistoryDialog::resetFilters()
{
    ui->tempEdit->clear();
    ui->humEdit->clear();
    searchData();
}

void HistoryDialog::deleteSelected()
{
    const int row = ui->tableWidget->currentRow();
    if (row < 0 || !ui->tableWidget->item(row, 0)) {
        QMessageBox::information(this, tr("请选择记录"), tr("先在表格中选择一行。"));
        return;
    }

    const int id = ui->tableWidget->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, tr("确认删除"), tr("确定删除编号 %1 的阈值记录吗？").arg(id))
        != QMessageBox::Yes)
        return;

    QString error;
    if (!store_->remove(id, &error)) {
        QMessageBox::warning(this, tr("删除失败"), tr("无法删除记录：%1").arg(error));
        return;
    }
    searchData();
}
