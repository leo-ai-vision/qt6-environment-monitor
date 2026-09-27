#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>

class ThresholdStore;

namespace Ui { class HistoryDialog; }

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(ThresholdStore *store, QWidget *parent = nullptr);
    ~HistoryDialog() override;

private slots:
    void searchData();
    void resetFilters();
    void deleteSelected();

private:
    Ui::HistoryDialog *ui;
    ThresholdStore *store_; // Owned by the main window.
};

#endif // HISTORYDIALOG_H
