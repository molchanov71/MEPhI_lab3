#include "dialogs/input_index_dialog.hpp"
#include "ui_input_index_dialog.h"

InputIndexDialog::InputIndexDialog(const size_t max, QWidget *parent) : QDialog(parent), ui(new Ui::InputIndexDialog), val(0)
{
    ui->setupUi(this);
    ui->indexSpinBox->setMaximum(max);
    connect(ui->submitButton, &QPushButton::clicked, this, [this]
    {
        accept();
    });
}

size_t InputIndexDialog::value() const
{
    return ui->indexSpinBox->value();
}

InputIndexDialog::~InputIndexDialog()
{
    delete ui;
}