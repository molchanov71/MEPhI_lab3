#include "dialogs/create_shared_dialog.hpp"
#include "ui_create_shared_dialog.h"
#include "controller.hpp"
#include <QMessageBox>

CreateSharedDialog::CreateSharedDialog(IController &controller_,
                                       QWidget *parent) : QDialog(parent), ui(new Ui::CreateSharedDialog),
                                                          controller(controller_)
{
    ui->setupUi(this);
    connect(ui->submitPushButton, &QPushButton::clicked, this, &CreateSharedDialog::onButtonClicked);
}

void CreateSharedDialog::onButtonClicked()
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
    if (ui->elSizeSpinBox->value() == 0)
    {
        QMessageBox::critical(this, "Ошибка", "Размер одного элемента массива не может быть равен нулю!");
        return;
    }
    std::string line_text = ui->nameLineEdit->text().toStdString();
    try
    {
        controller.createShared(line_text, ui->totalSizeSpinBox->value(), ui->elSizeSpinBox->value());
        QMessageBox::information(this, "Успешно!",
                                 std::format("Вы успешно создали разделяемый сегмент \"{}\"!", line_text).c_str());
        accept();
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
    }
}

CreateSharedDialog::~CreateSharedDialog()
{
    delete ui;
}
