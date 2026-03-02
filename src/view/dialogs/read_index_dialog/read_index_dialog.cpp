#include "dialogs/read_index_dialog.hpp"
#include "ui_read_index_dialog.h"

#include <QMessageBox>

ReadIndexDialog::ReadIndexDialog(const std::vector <uint8_t> &data, QWidget *parent) : QDialog(parent),
    ui(new Ui::ReadIndexDialog)
{
    ui->setupUi(this);
    setWindowTitle("Данные по индексу");
    for (const uint8_t byte: data)
    {
        const int new_row = ui->dataWidget->rowCount();
        ui->dataWidget->insertRow(new_row);
        auto *idx_item = new QTableWidgetItem(std::to_string(new_row).c_str());
        ui->dataWidget->setItem(new_row, 0, idx_item);
        auto *byte_item = new QTableWidgetItem(std::to_string(byte).c_str());
        ui->dataWidget->setItem(new_row, 1, byte_item);
    }
}

ReadIndexDialog::~ReadIndexDialog()
{
    delete ui;
}
