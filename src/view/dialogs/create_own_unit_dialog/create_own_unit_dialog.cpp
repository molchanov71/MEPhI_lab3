#include "dialogs/create_own_unit_dialog.hpp"
#include "ui_create_own_unit_dialog.h"
#include "controller.hpp"
#include <QMessageBox>

CreateOwnUnitDialog::CreateOwnUnitDialog(IController &controller_, std::string prog_path_,
                                         QWidget *parent) : QDialog(parent), ui(new Ui::CreateOwnUnitDialog),
                                                            controller(controller_), prog_path(std::move(prog_path_))
{
    ui->setupUi(this);
    setWindowTitle(std::format("Выделение сегмента памяти в \"{}\"", prog_path).c_str());
    connect(ui->arrRadioButton, &QRadioButton::toggled, this, [this](const bool checked)
    {
        ui->elSizeLabel->setEnabled(checked);
        ui->elSizeSpinBox->setEnabled(checked);
    });
    connect(ui->submitPushButton, &QPushButton::clicked, this, &CreateOwnUnitDialog::onButtonClicked);
}

void CreateOwnUnitDialog::onButtonClicked()
{
    if (ui->nameLineEdit->text().trimmed().isEmpty())
    {
        QMessageBox::critical(this, "Ошибка", "Не введено имя элемента!");
        return;
    }
    if (ui->totalSizeSpinBox->value() == 0)
    {
        QMessageBox::critical(this, "Ошибка", "Размер элемента не может быть равен нулю!");
        return;
    }
    if (ui->arrRadioButton->isChecked() && ui->elSizeSpinBox->value() == 0)
    {
        QMessageBox::critical(this, "Ошибка", "Размер одного элемента массива не может быть равен нулю!");
        return;
    }
    std::string line_text;
    if (ui->varRadioButton->isChecked())
        try
        {
            line_text = ui->nameLineEdit->text().toStdString();
            controller.createVariable(prog_path, line_text, ui->totalSizeSpinBox->value());
            QMessageBox::information(this, "Успешно!", std::format("Вы успешно создали переменную \"{}\"!", line_text).c_str());
            accept();
        }
        catch (const std::exception &err)
        {
            QMessageBox::critical(this, "Ошибка", err.what());
        }
    else if (ui->arrRadioButton->isChecked())
        try
        {
            line_text = ui->nameLineEdit->text().toStdString();
            controller.createArray(prog_path, line_text, ui->totalSizeSpinBox->value(), ui->elSizeSpinBox->value());
            QMessageBox::information(this, "Успешно!", std::format("Вы успешно создали массив \"{}\"!", line_text).c_str());
            accept();
        }
        catch (const std::exception &err)
        {
            QMessageBox::critical(this, "Ошибка", err.what());
        }
}

CreateOwnUnitDialog::~CreateOwnUnitDialog() { delete ui; }
