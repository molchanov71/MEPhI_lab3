#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class CreateSharedDialog;
}
QT_END_NAMESPACE


class CreateSharedDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit CreateSharedDialog(class IController &, QWidget *parent = nullptr);

    ~CreateSharedDialog() override;
private:
    Ui::CreateSharedDialog *ui;
    IController &controller;

    void onButtonClicked();
};
