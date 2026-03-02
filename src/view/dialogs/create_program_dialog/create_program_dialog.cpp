#include "dialogs/create_program_dialog.hpp"
#include "ui_create_program_dialog.h"
#include "controller.hpp"

#include <QMessageBox>

CreateProgramDialog::CreateProgramDialog(IController &controller, QWidget *parent) : QDialog(parent),
    ui(new Ui::CreateProgramDialog), controller(controller)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &CreateProgramDialog::onButtonClicked);
}

void CreateProgramDialog::onButtonClicked()
{
    if (ui->pathLineEdit->text().trimmed().isEmpty())
    {
        QMessageBox::critical(this, "Ошибка", "Путь не может быть пустым!");
        return;
    }
    try
    {
        std::string path = ui->pathLineEdit->text().trimmed().toStdString();
        size_t quota = ui->quotaLineEdit->value();
        controller.createProgram(path, quota);
        QMessageBox::information(this, "Программа создана",
                                 QString(std::format("Вы успешно создали программу по пути {} с квотой {}!", path,
                                                     quota).c_str()));
        accept();
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
    }
}

CreateProgramDialog::~CreateProgramDialog()
{
    delete ui;
}
