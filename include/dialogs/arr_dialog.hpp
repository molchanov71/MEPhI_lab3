#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE namespace Ui
{
    class ArrDialog;
}

QT_END_NAMESPACE class ArrDialog final : public QDialog
{
    Q_OBJECT

public:
    ArrDialog(std::string prog_path_,
              std::string unit_name_,
              class IProgramRepository &,
              class IController &,
              QWidget *parent = nullptr);

    ~ArrDialog() override;

private:
    Ui::ArrDialog *ui;
    std::string prog_path, unit_name;
    IProgramRepository &repo;
    IController &controller;

    void writeData();

    void writeIndex(size_t);

    void updateData();

    const class AbstractIndexed *getArrPtr();
};
