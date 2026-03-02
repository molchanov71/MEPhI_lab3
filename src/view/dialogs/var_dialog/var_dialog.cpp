#include "dialogs/var_dialog.hpp"
#include "ui_var_dialog.h"
#include "dialogs/write_data_dialog.hpp"

#include "controller.hpp"
#include "model/repository.hpp"
#include "model/entity/unit.hpp"
#include <QMessageBox>
#include <utility>

VarDialog::VarDialog(std::string prog_path_,
                     std::string unit_name_,
                     IProgramRepository &repo_,
                     IController &controller_,
                     QWidget *parent) : QDialog(parent),
                                        ui(new Ui::VarDialog),
                                        prog_path(std::move(prog_path_)),
                                        unit_name(std::move(unit_name_)),
                                        repo(repo_),
                                        controller(controller_)
{
    ui->setupUi(this);
    setWindowTitle(std::format("Переменная \"{}\" в \"{}\"", unit_name, prog_path).c_str());
    try
    {
        const Program &prog = repo.getProgram(prog_path);
        const AbstractUnit &unit = prog.getUnit(unit_name);
        ui->infoLabel->setText(std::format("Размер: {}", unit.getTotalSize()).c_str());
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
        QDialog::reject();
    }
    updateData();
    connect(ui->writeDataButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                writeData();
                updateData();
            });
}

void VarDialog::writeData()
{
    try
    {
        const std::vector data = controller.readUnit(prog_path, unit_name);
        WriteDataDialog dialog(prog_path, unit_name, repo, controller, -1, data, this);
        dialog.exec();
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
    }
}

void VarDialog::updateData()
{
    try
    {
        const std::vector data = controller.readUnit(prog_path, unit_name);
        ui->dataWidget->setRowCount(0);
        for (const auto byte: data)
        {
            const int new_row = ui->dataWidget->rowCount();
            ui->dataWidget->insertRow(new_row);
            auto *idx_item = new QTableWidgetItem(std::to_string(new_row).c_str());
            ui->dataWidget->setItem(new_row, 0, idx_item);
            auto *byte_item = new QTableWidgetItem(std::to_string(byte).c_str());
            ui->dataWidget->setItem(new_row, 1, byte_item);
        }
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
    }
}

VarDialog::~VarDialog()
{
    delete ui;
}
