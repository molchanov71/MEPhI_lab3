#pragma once

#include <QDialog>

class IController;

QT_BEGIN_NAMESPACE namespace Ui
{
    class CreateOwnUnitDialog;
}

QT_END_NAMESPACE class CreateOwnUnitDialog final : public QDialog
{
    Q_OBJECT

public:
    CreateOwnUnitDialog(IController &, std::string, QWidget *parent = nullptr);

    ~CreateOwnUnitDialog() override;

private:
    Ui::CreateOwnUnitDialog *ui;
    IController &controller;
    std::string prog_path;

    void onButtonClicked();
};
