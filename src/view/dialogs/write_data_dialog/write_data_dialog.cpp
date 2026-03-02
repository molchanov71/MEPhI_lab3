#include "dialogs/write_data_dialog.hpp"
#include "ui_write_data_dialog.h"
#include "controller.hpp"
#include "model/repository.hpp"
#include "model/entity/program.hpp"
#include "model/entity/unit.hpp"
#include <QMessageBox>
#include <QSpinBox>

WriteDataDialog::WriteDataDialog(const std::string &prog_path,
                                 const std::string &name,
                                 IProgramRepository &repo_,
                                 IController &controller_,
                                 int idx,
                                 const std::vector <uint8_t> &data,
                                 QWidget *parent) : QDialog(parent),
                                                    ui(new Ui::WriteDataDialog)
{
    ui->setupUi(this);
    setWindowTitle(std::format("Запись данных в переменную \"{}\" в \"{}\"", name, prog_path).c_str());
    const Program &prog = repo_.getProgram(prog_path);
    const AbstractUnit &unit = prog.getUnit(name);
    size_t size;
    if (idx < 0)
        size = unit.getTotalSize();
    else
    {
        if (const auto idx_ptr = dynamic_cast <const AbstractIndexed *>(&unit); !idx_ptr)
        {
            size = 0;
            QMessageBox::critical(this, "Ошибка", "Элемент не является массивом, но индекс был передан");
            QDialog::reject();
        }
        else
            size = idx_ptr->getElSize();
    }
    if (data.size() != size)
    {
        QMessageBox::critical(this, "Ошибка", "Размер переданных данных не подходит");
        QDialog::reject();
    }
    auto *table = ui->bytesTableWidget;
    for (size_t i = 0; i < size; ++i)
    {
        const int new_row = table->rowCount();
        table->insertRow(new_row);
        auto *idx_item = new QTableWidgetItem(std::to_string(i).c_str());
        table->setItem(new_row, 0, idx_item);
        auto *spinbox = new QSpinBox(table);
        spinbox->setRange(0, 255);
        spinbox->setValue(data[i]);
        table->setCellWidget(new_row, 1, spinbox);
    }
    connect(ui->sendButton,
            &QPushButton::clicked,
            this,
            [this, size, &controller_, &prog_path, &name, idx]
            {
                const auto *table_ = ui->bytesTableWidget;
                std::vector <uint8_t> new_data(size);
                for (size_t i = 0; i < size; ++i)
                {
                    const auto *spin = qobject_cast <QSpinBox *>(table_->cellWidget(static_cast <int>(i), 1));
                    if (!spin)
                    {
                        QMessageBox::critical(this, "Ошибка", "Не удалось преобразовать ячейку");
                        reject();
                        return;
                    }
                    new_data[i] = spin->value();
                }
                try
                {
                    if (idx < 0)
                        controller_.writeUnit(prog_path, name, new_data);
                    else
                        controller_.writeIndex(prog_path, name, idx, new_data);
                    QMessageBox::information(this, "Успешно!", "Данные перезаписаны");
                    accept();
                }
                catch (const std::exception &err)
                {
                    QMessageBox::critical(this, "Ошибка", err.what());
                }
            });
}

WriteDataDialog::~WriteDataDialog()
{
    delete ui;
}
