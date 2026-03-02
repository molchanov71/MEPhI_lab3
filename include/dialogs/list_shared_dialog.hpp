#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE

namespace Ui
{
    class ListSharedDialog;
}

QT_END_NAMESPACE


class ListSharedDialog final : public QDialog
{
    Q_OBJECT

public:
    ListSharedDialog(class IProgramRepository &, class IController &, QWidget *parent = nullptr);

    ~ListSharedDialog() override;

private:
    Ui::ListSharedDialog *ui;
    IProgramRepository &repo;
    IController &controller;

    void onItemDoubleClicked();

    void updateTable() const;
};
