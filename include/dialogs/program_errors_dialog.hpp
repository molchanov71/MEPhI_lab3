#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ProgramErrorsDialog;
}
QT_END_NAMESPACE

class ProgramErrorsDialog final : public QDialog
{
    Q_OBJECT
public:
    ProgramErrorsDialog(const class IProgramRepository &, const std::string &, QWidget *parent = nullptr);

    ~ProgramErrorsDialog() override;
private:
    Ui::ProgramErrorsDialog *ui;
};
