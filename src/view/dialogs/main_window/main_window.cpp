#include "dialogs/main_window.hpp"
#include "ui_main_window.h"
#include "dialogs/program_dialog.hpp"
#include "dialogs/create_program_dialog.hpp"
#include "dialogs/list_shared_dialog.hpp"
#include "controller.hpp"
#include "model/repository.hpp"
#include <QMessageBox>

MainWindow::MainWindow(IProgramRepository &repo_, IController &controller_, QWidget *parent_) : QMainWindow(parent_),
    ui(new Ui::MainWindow), repo(repo_), controller(controller_)
{
    ui->setupUi(this);
    for (const auto &prog: repo.getPrograms() | std::views::keys)
    {
        auto *new_item = new QListWidgetItem;
        new_item->setText(prog.c_str());
        ui->listProgramsWidget->addItem(new_item);
    }
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onTextEdited);
    connect(ui->createProgramButton, &QPushButton::clicked, this, [this]
    {
        CreateProgramDialog dialog(controller, this);
        dialog.exec();
        ui->searchLineEdit->setText("");
        onTextEdited("");
    });
    connect(ui->removeProgramButton, &QPushButton::clicked, this, &MainWindow::removeProgram);
    connect(ui->listSharedButton, &QPushButton::clicked, this, [this]
    {
        ListSharedDialog dialog(repo, controller, this);
        dialog.exec();
    });
    connect(ui->showAllProgsButton, &QPushButton::clicked, this, [this] { onTextEdited(""); });
    connect(ui->listProgramsWidget, &QListWidget::itemDoubleClicked, this, [this](const QListWidgetItem *item)
    {
        try
        {
            const std::string text = item->text().toStdString();
            const auto c_text = text.c_str();
            ProgramDialog dialog(repo, controller, c_text, this);
            dialog.exec();
        }
        catch (const std::runtime_error &err)
        {
            QMessageBox::critical(this, "Ошибка", err.what());
        }
    });
    connect(ui->defragmentButton, &QPushButton::clicked, this, [this]
    {
        try
        {
            controller.defragment();
            QMessageBox::information(this, "Успешно", "Дефрагментация успешно произведена");
        }
        catch (const std::exception &err)
        {
            QMessageBox::critical(this, "Ошибка", err.what());
        }
    });
}

void MainWindow::onTextEdited(const QString &text) const
{
    ui->listProgramsWidget->clear();
    for (const auto &prog: repo.getPrograms() | std::views::keys | std::views::filter([&text](const std::string &path)
    {
        return path.contains(text.toStdString());
    }))
    {
        auto *new_item = new QListWidgetItem;
        new_item->setText(prog.c_str());
        ui->listProgramsWidget->addItem(new_item);
    }
}

void MainWindow::removeProgram()
{
    const auto *current = ui->listProgramsWidget->currentItem();
    if (current == nullptr)
        return;
    try
    {
        std::string path = current->text().toStdString();
        controller.destroyProgram(path);
        QMessageBox::information(this, "Успешно", std::format("Программа по пути \"{}\" успешно удалена", path).c_str());
        delete current;
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, "Ошибка", err.what());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
