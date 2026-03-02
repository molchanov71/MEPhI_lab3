#include "dialogs/program_dialog.hpp"
#include "ui_program_dialog.h"
#include "dialogs/create_own_unit_dialog.hpp"
#include "dialogs/create_ref_dialog.hpp"
#include "dialogs/grant_shared_dialog.hpp"
#include "dialogs/var_dialog.hpp"
#include "dialogs/arr_dialog.hpp"
#include "dialogs/program_errors_dialog.hpp"
#include "controller.hpp"
#include "model/repository.hpp"
#include <QMessageBox>
#include <QPushButton>


ProgramDialog::ProgramDialog(IProgramRepository &repo_,
                             IController &controller_,
                             const char *prog_path_,
                             QWidget *parent) : QDialog(parent),
                                                ui(new Ui::ProgramDialog),
                                                repo(repo_),
                                                controller(controller_),
                                                prog_path(prog_path_)
{
    if (!repo.getPrograms().contains(prog_path))
    {
        QMessageBox::critical(this,
                              "Ошибка",
                              std::format("Программа по пути \"{}\" не существует!", prog_path).c_str());
        QDialog::reject();
    }
    ui->setupUi(this);
    const Program &prog = repo.getProgram(prog_path);
    setWindowTitle(QString(prog_path_));
    const std::string label_text = std::format("Путь: \"{}\"\nКвота: {}", prog_path, prog.getQuota());
    ui->programInfoLabel->setText(label_text.c_str());
    updateOwnUnits();
    connect(ui->createOwnUnitButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                CreateOwnUnitDialog dialog(controller, prog_path, this);
                dialog.exec();
                updateOwnUnits();
            });
    connect(ui->deleteOwnUnitButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                const int selected_row = ui->ownUnitsTableWidget->currentRow();
                if (selected_row == -1)
                    return;
                const std::string unit_name = ui->ownUnitsTableWidget->item(selected_row, 0)->text().toStdString();
                try
                {
                    controller.destroyOwnUnit(prog_path, unit_name);
                    updateOwnUnits();
                    updateRefs();
                    QMessageBox::information(this,
                                             "Успешно",
                                             std::format("Элемент \"{}\" успешно удалён", unit_name).c_str());
                }
                catch (const std::exception &err)
                {
                    QMessageBox::critical(this, "Ошибка", err.what());
                }
            });
    connect(ui->ownUnitsTableWidget,
            &QTableWidget::itemDoubleClicked,
            this,
            [this](const QTableWidgetItem *item)
            {
                const int row = item->row();
                const auto unit_name = ui->ownUnitsTableWidget->item(row, 0)->text().toStdString();
                if (const auto type_name = ui->ownUnitsTableWidget->item(row, 1)->text().toStdString();
                    type_name == "Array")
                {
                    ArrDialog dialog(prog_path, unit_name, repo, controller, this);
                    dialog.exec();
                }
                else if (type_name == "Variable")
                {
                    VarDialog dialog(prog_path, unit_name, repo, controller, this);
                    dialog.exec();
                }
            });
    connect(ui->grantSharedButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                GrantSharedDialog dialog(repo, controller, prog_path, this);
                dialog.exec();
                updateSharedUnits();
            });
    connect(ui->createRefButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                const int curr_row = ui->ownUnitsTableWidget->currentRow();
                if (curr_row == -1)
                    return;
                const std::string unit_name = ui->ownUnitsTableWidget->item(curr_row, 0)->text().toStdString();
                CreateRefDialog dialog(controller, prog_path, unit_name, this);
                dialog.exec();
                updateRefs();
            });
    connect(ui->removeRefButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                const int curr_row = ui->refsTableWidget->currentRow();
                if (curr_row == -1)
                    return;
                const std::string ref_name = ui->refsTableWidget->item(curr_row, 0)->text().toStdString();
                try
                {
                    controller.removeRef(prog_path, ref_name);
                    QMessageBox::information(this,
                                             "Успешно",
                                             std::format("Ссылка \"{}\" была успешно удалена", ref_name).c_str());
                }
                catch (const std::exception &err)
                {
                    QMessageBox::critical(this, "Ошибка", err.what());
                }
            });
    connect(ui->listErrorsButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                ProgramErrorsDialog dialog(repo, prog_path, this);
                dialog.exec();
            });
    connect(ui->refsTableWidget,
            &QTableWidget::itemDoubleClicked,
            this,
            [this](const QTableWidgetItem *item)
            {
                const int curr_row = item->row();
                const std::string ref_name = ui->refsTableWidget->item(curr_row, 0)->text().toStdString();
                try
                {
                    if (const AbstractUnit &unit = controller.getUnit(prog_path, ref_name);
                        unit.getType() == "Variable")
                    {
                        VarDialog dialog(prog_path, unit.getName(), repo, controller, this);
                        dialog.exec();
                    }
                    else if (unit.getType() == "Array")
                    {
                        ArrDialog dialog(prog_path, unit.getName(), repo, controller, this);
                        dialog.exec();
                    }
                }
                catch (const std::exception &err)
                {
                    QMessageBox::critical(this, "Ошибка", err.what());
                }
            });
    connect(ui->getMemoryUsageButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                try
                {
                    size_t res = controller.getTotalMemoryUsage(prog_path);
                    QMessageBox::information(this,
                                             "Использование памяти",
                                             QString::fromStdString(std::format("Используется: {} байт", res)));
                }
                catch (const std::exception &err)
                {
                    QMessageBox::critical(this, "Ошибка", err.what());
                }
            });
}

void ProgramDialog::updateOwnUnits()
{
    ui->ownUnitsTableWidget->setRowCount(0);
    if (!repo.getPrograms().contains(prog_path))
    {
        QMessageBox::critical(this,
                              "Ошибка",
                              std::format("Программа по пути \"{}\" не существует!", prog_path).c_str());
        reject();
    }
    const Program &prog = repo.getProgram(prog_path);
    auto *table = ui->ownUnitsTableWidget;
    for (const auto &[k, v]: prog.getOwnUnits())
    {
        const int new_row = table->rowCount();
        table->insertRow(new_row);
        auto *name_item = new QTableWidgetItem(k.c_str());
        auto *type_item = new QTableWidgetItem(v->getType().c_str());
        auto *size_item = new QTableWidgetItem(std::to_string(v->getTotalSize()).c_str());
        QString el_size_str;
        if (const auto arr_ptr = dynamic_cast <AbstractIndexed *>(v.get()))
            el_size_str = QString::number(arr_ptr->getElSize());
        auto *el_size_item = new QTableWidgetItem(el_size_str);
        auto *address_item = new QTableWidgetItem(std::to_string(v->getAddress()).c_str());
        table->setItem(new_row, 0, name_item);
        table->setItem(new_row, 1, type_item);
        table->setItem(new_row, 2, size_item);
        table->setItem(new_row, 3, el_size_item);
        table->setItem(new_row, 4, address_item);
    }
}

void ProgramDialog::updateSharedUnits()
{
    auto *table = ui->sharedUnitsTableWidget;
    table->setRowCount(0);
    if (!repo.getPrograms().contains(prog_path))
    {
        QMessageBox::critical(this,
                              "Ошибка",
                              std::format("Программа по пути \"{}\" не существует!", prog_path).c_str());
        reject();
    }
    for (Program &prog = repo.getProgram(prog_path); const auto &[k, v]: prog.getSharedUnits())
    {
        const int new_row = table->rowCount();
        table->insertRow(new_row);
        if (v.expired())
        {
            prog.detachAccess(k);
            table->removeRow(new_row);
            continue;
        }
        const auto sptr = v.lock();
        auto *name_item = new QTableWidgetItem(k.c_str());
        auto *size_item = new QTableWidgetItem(std::to_string(sptr->getTotalSize()).c_str());
        std::string el_size_str;
        if (const auto arr_ptr = dynamic_cast <AbstractIndexed *>(sptr.get()))
            el_size_str = std::to_string(arr_ptr->getElSize());
        auto *el_size_item = new QTableWidgetItem(el_size_str.c_str());
        auto *address_item = new QTableWidgetItem(std::to_string(sptr->getAddress()).c_str());
        table->setItem(new_row, 0, name_item);
        table->setItem(new_row, 1, size_item);
        table->setItem(new_row, 2, el_size_item);
        table->setItem(new_row, 3, address_item);
    }
}

void ProgramDialog::updateRefs()
{
    auto *table = ui->refsTableWidget;
    table->setRowCount(0);
    if (!repo.getPrograms().contains(prog_path))
    {
        QMessageBox::critical(this,
                              "Ошибка",
                              std::format("Программа по пути \"{}\" не существует!", prog_path).c_str());
        reject();
    }
    for (const Program &prog = repo.getProgram(prog_path); const auto &[k, v]: prog.getRefs())
    {
        const int new_row = table->rowCount();
        table->insertRow(new_row);
        auto *name_item = new QTableWidgetItem(k.c_str());
        table->setItem(new_row, 0, name_item);
        auto *active_item = new QTableWidgetItem(v.detached() ? "Нет" : "Да");
        table->setItem(new_row, 1, active_item);
        if (!v.detached())
        {
            const OwnUnit *unit = v.getPtr();
            auto *unitname_item = new QTableWidgetItem(unit->getName().c_str());
            table->setItem(new_row, 2, unitname_item);
        }
    }
}

ProgramDialog::~ProgramDialog()
{
    delete ui;
}
