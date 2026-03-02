#include "dialogs/grant_shared_dialog.hpp"
#include "ui_grant_shared_dialog.h"
#include "controller.hpp"
#include "model/repository.hpp"
#include <QMessageBox>

GrantSharedDialog::GrantSharedDialog(const IProgramRepository &repo,
                                     IController &controller,
                                     std::string prog_path,
                                     QWidget *parent) : QDialog(parent),
                                                        ui(new Ui::GrantSharedDialog)
{
    ui->setupUi(this);
    setWindowTitle(std::format("Получение \"{}\" доступа к разделяемому сегменту", prog_path).c_str());
    auto *table = ui->sharedUnitsTableWidget;
    for (const auto &[k, v]: repo.getSharedElems())
    {
        const int new_row = table->rowCount();
        table->insertRow(new_row);
        auto *name_item = new QTableWidgetItem(k.c_str());
        table->setItem(new_row, 0, name_item);
        auto *size_item = new QTableWidgetItem(std::to_string(v->getTotalSize()).c_str());
        table->setItem(new_row, 1, size_item);
        auto *el_size_item = new QTableWidgetItem(std::to_string(v->getElSize()).c_str());
        table->setItem(new_row, 2, el_size_item);
        auto *address_item = new QTableWidgetItem(std::to_string(v->getAddress()).c_str());
        table->setItem(new_row, 3, address_item);
    }
    connect(ui->selectButton,
            &QPushButton::clicked,
            this,
            [this, table, &controller, &prog_path]
            {
                const int row = table->currentRow();
                auto name = table->item(row, 0)->text().toStdString();
                try
                {
                    controller.grantShared(prog_path, name);
                    QMessageBox::information(this,
                                             "Успешно",
                                             std::format("Программе \"{}\" выдан доступ к переменной \"{}\"",
                                                         prog_path,
                                                         name).c_str());
                    accept();
                }
                catch (const std::exception &err)
                {
                    QMessageBox::critical(this, "Ошибка", err.what());
                    reject();
                }
            });
}

GrantSharedDialog::~GrantSharedDialog()
{
    delete ui;
}
