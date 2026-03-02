#include "dialogs/arr_dialog.hpp"
#include "ui_arr_dialog.h"
#include "dialogs/write_data_dialog.hpp"
#include "dialogs/input_index_dialog.hpp"
#include "dialogs/read_index_dialog.hpp"
#include "controller.hpp"
#include "model/repository.hpp"
#include <QMessageBox>
#include <utility>

ArrDialog::ArrDialog(std::string prog_path_,
                     std::string unit_name_,
                     IProgramRepository &repo_,
                     IController &controller_,
                     QWidget *parent) : QDialog(parent),
                                        ui(new Ui::ArrDialog),
                                        prog_path(std::move(prog_path_)),
                                        unit_name(std::move(unit_name_)),
                                        repo(repo_),
                                        controller(controller_)
{
    ui->setupUi(this);
    setWindowTitle(std::format("Массив \"{}\" в \"{}\"", unit_name, prog_path).c_str());
    try
    {
        const Program &prog = repo.getProgram(prog_path);
        const AbstractUnit *unit = &prog.getUnit(unit_name);
        auto *arr_ptr = getArrPtr();
        ui->infoLabel->setText(std::format("Размер: {}\nРазмер одного элемента: {}",
                                           unit->getTotalSize(),
                                           arr_ptr->getElSize()).c_str());
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
        QDialog::reject();
        delete ui;
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
    connect(ui->readIndexButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                const auto *arr_ptr = getArrPtr();
                const size_t len = arr_ptr->getLen();
                if (InputIndexDialog idx_dialog(len - 1, this); idx_dialog.exec() == QDialog::Accepted)
                {
                    const size_t idx = idx_dialog.value();
                    const std::vector data = controller.readIndex(prog_path, unit_name, idx);
                    ReadIndexDialog read_idx_dialog(data, this);
                    read_idx_dialog.exec();
                }
            });
    connect(ui->writeIndexButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                const auto *arr_ptr = getArrPtr();
                const size_t len = arr_ptr->getLen();
                if (InputIndexDialog idx_dialog(len - 1, this); idx_dialog.exec() == QDialog::Accepted)
                {
                    const size_t idx = idx_dialog.value();
                    const std::vector data = controller.readIndex(prog_path, unit_name, idx);
                    WriteDataDialog write_dialog(prog_path,
                                                 unit_name,
                                                 repo,
                                                 controller,
                                                 static_cast <int>(idx),
                                                 data,
                                                 this);
                    write_dialog.exec();
                    updateData();
                }
            });
}

const AbstractIndexed *ArrDialog::getArrPtr()
{
    const Program &prog = repo.getProgram(prog_path);
    const AbstractUnit *unit = &prog.getUnit(unit_name);
    if (const auto *arr_ptr = dynamic_cast <const AbstractIndexed *>(unit); arr_ptr)
        return arr_ptr;
    throw std::logic_error(std::format("Элемент \"{}\" не индексируемый", unit_name));
}

void ArrDialog::writeData()
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

void ArrDialog::writeIndex(const size_t idx)
{
    try
    {
        const std::vector data = controller.readIndex(prog_path, unit_name, idx);
        WriteDataDialog dialog(prog_path, unit_name, repo, controller, static_cast <int>(idx), data, this);
        dialog.exec();
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
    }
}

void ArrDialog::updateData()
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

ArrDialog::~ArrDialog()
{
    delete ui;
}
