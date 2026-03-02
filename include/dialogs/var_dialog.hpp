#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE namespace Ui
{
    class VarDialog;
}

QT_END_NAMESPACE class VarDialog final : public QDialog
{
    Q_OBJECT

public:
    VarDialog(std::string prog_path_,
              std::string unit_name_,
              class IProgramRepository &,
              class IController &,
              QWidget *parent = nullptr);

    ~VarDialog() override;

private:
    Ui::VarDialog *ui;
    std::string prog_path, unit_name;
    IProgramRepository &repo;
    IController &controller;

    void writeData();

    void updateData();
};
