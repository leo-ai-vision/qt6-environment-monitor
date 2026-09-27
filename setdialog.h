#ifndef SETDIALOG_H
#define SETDIALOG_H

#include <QDialog>

namespace Ui { class SetDialog; }

class SetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetDialog(double temperature, double humidity, QWidget *parent = nullptr);
    ~SetDialog() override;

    double temperature() const;
    double humidity() const;

private:
    Ui::SetDialog *ui;
};

#endif // SETDIALOG_H
