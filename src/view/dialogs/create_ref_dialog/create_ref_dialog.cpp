#include "dialogs/create_ref_dialog.hpp"
#include "ui_create_ref_dialog.h"
#include "controller.hpp"
#include <QMessageBox>

CreateRefDialog::CreateRefDialog(IController &controller, const std::string &prog_path, const std::string &unit_name, QWidget *parent) : QDialog(parent), ui(new Ui::CreateRefDialog)
{
    ui->setupUi(this);
    setWindowTitle(std::format("Создание ссылки в \"{}\"", prog_path).c_str());
    connect(ui->submitButton, &QPushButton::clicked, this, [this, &controller, &prog_path, &unit_name]
    {
        std::string ref_name = ui->refNameLineEdit->text().toStdString();
        try
        {
            controller.createRef(prog_path, unit_name, ref_name);
            QMessageBox::information(this, "Успешно", std::format("Вы успешно создали ссылку \"{}\" на переменную \"{}\"", ref_name, unit_name).c_str());
            accept();
        }
        catch (const std::exception &err)
        {
            QMessageBox::critical(this, "Ошибка", err.what());
            reject();
        }
    });
}

CreateRefDialog::~CreateRefDialog()
{
    delete ui;
}
