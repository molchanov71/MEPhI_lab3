#pragma once

#include "errors.hpp"
#include "table.hpp"
#include <QMainWindow>

class IProgramRepository;

QT_BEGIN_NAMESPACE namespace Ui
{
    class MainWindow;
}

QT_END_NAMESPACE class IController;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(IProgramRepository &, IController &, QWidget *parent_ = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    IProgramRepository &repo;
    IController &controller;

    void onTextEdited(const QString &) const;

    void removeProgram();
};
