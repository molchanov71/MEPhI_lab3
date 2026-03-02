#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class GrantSharedDialog;
}
QT_END_NAMESPACE


class GrantSharedDialog final : public QDialog
{
    Q_OBJECT
public:
    GrantSharedDialog(const class IProgramRepository &, class IController &, std::string, QWidget *parent = nullptr);

    ~GrantSharedDialog() override;
private:
    Ui::GrantSharedDialog *ui;
};
