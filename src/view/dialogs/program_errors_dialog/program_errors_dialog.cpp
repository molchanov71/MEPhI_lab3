#include "dialogs/program_errors_dialog.hpp"
#include "ui_program_errors_dialog.h"
#include "model/repository.hpp"
#include <QMessageBox>
#include <iostream>

ProgramErrorsDialog::ProgramErrorsDialog(const IProgramRepository &repo,
                                         const std::string &prog_path,
                                         QWidget *parent) : QDialog(parent),
                                                            ui(new Ui::ProgramErrorsDialog)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromStdString(std::format("Все ошибки в программе \"{}\"", prog_path)));
    auto *table = ui->errorsTableWidget;
    try
    {
        if (const auto &errors = repo.getErrors(); errors.contains(prog_path))
            for (auto &err: errors.at(prog_path))
            {
                const int new_row = table->rowCount();
                table->insertRow(new_row);
                auto *type_item = new QTableWidgetItem(QString::fromStdString(err->getType()));
                table->setItem(new_row, 0, type_item);
                time_point tp = err->getTimePoint();
                auto *date_item = new QTableWidgetItem(std::format("{:%d.%m.%Y}", tp).c_str());
                table->setItem(new_row, 1, date_item);
                auto *time_item = new QTableWidgetItem(std::format("{:%H:%M:%S}", tp).c_str());
                table->setItem(new_row, 2, time_item);
                auto *message_item = new QTableWidgetItem(err->what());
                table->setItem(new_row, 3, message_item);
            }
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
        QDialog::reject();
    }
}

ProgramErrorsDialog::~ProgramErrorsDialog()
{
    delete ui;
}
