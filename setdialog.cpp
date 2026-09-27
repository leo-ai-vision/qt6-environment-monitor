#include "setdialog.h"
#include "ui_setdialog.h"

SetDialog::SetDialog(double temperature, double humidity, QWidget *parent)
    : QDialog(parent), ui(new Ui::SetDialog)
{
    ui->setupUi(this);
    ui->tempSpin->setValue(temperature);
    ui->humSpin->setValue(humidity);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

SetDialog::~SetDialog()
{
    delete ui;
}

double SetDialog::temperature() const
{
    return ui->tempSpin->value();
}

double SetDialog::humidity() const
{
    return ui->humSpin->value();
}
