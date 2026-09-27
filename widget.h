#ifndef WIDGET_H
#define WIDGET_H

#include <QSqlDatabase>
#include <QWidget>

#include <memory>

class ThresholdStore;

namespace Ui { class Widget; }

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

private slots:
    void refreshDisplay();
    void openSetDialog();
    void openHistory();

private:
    bool initializeDatabase();
    void loadLastThreshold();
    void updateThresholdLabels();

    Ui::Widget *ui;
    QSqlDatabase database_;
    std::unique_ptr<ThresholdStore> store_;
    double temperatureThreshold_ = 30.0;
    double humidityThreshold_ = 80.0;
};

#endif // WIDGET_H
