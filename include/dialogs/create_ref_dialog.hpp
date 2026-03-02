#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class CreateRefDialog;
}
QT_END_NAMESPACE


class CreateRefDialog final : public QDialog
{
    Q_OBJECT
public:
    CreateRefDialog(class IController &, const std::string &prog_path, const std::string &unit_name, QWidget *parent = nullptr);

    ~CreateRefDialog() override;
private:
    Ui::CreateRefDialog *ui;
};
