#pragma once

#include <QDialog>

class IController;

QT_BEGIN_NAMESPACE namespace Ui
{
    class CreateProgramDialog;
}

QT_END_NAMESPACE class CreateProgramDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit CreateProgramDialog(IController &, QWidget *parent = nullptr);

    ~CreateProgramDialog() override;

private:
    Ui::CreateProgramDialog *ui;
    IController &controller;

    void onButtonClicked();
};
