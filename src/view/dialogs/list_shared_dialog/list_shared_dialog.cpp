#include "dialogs/list_shared_dialog.hpp"
#include "ui_list_shared_dialog.h"
#include "dialogs/create_shared_dialog.hpp"
#include "controller.hpp"
#include "model/repository.hpp"
#include "table.hpp"

ListSharedDialog::ListSharedDialog(IProgramRepository &repo_, IController &controller_,
                                   QWidget *parent) : QDialog(parent), ui(new Ui::ListSharedDialog), repo(repo_),
                                                      controller(controller_)
{
    ui->setupUi(this);
    updateTable();
    connect(ui->createSharedButton, &QPushButton::clicked, this, [this]
    {
        CreateSharedDialog dialog(controller, this);
        dialog.exec();
        updateTable();
    });
}

void ListSharedDialog::updateTable() const
{
    const Table<std::string, std::shared_ptr<AbstractShared> > &units = repo.getSharedElems();
    auto *table = ui->sharedUnitsTableWidget;
    table->setRowCount(0);
    for (const auto &[k, v]: units)
    {
        const int new_row = ui->sharedUnitsTableWidget->rowCount();
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
}

ListSharedDialog::~ListSharedDialog()
{
    delete ui;
}
