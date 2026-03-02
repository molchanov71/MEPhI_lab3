#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class InputIndexDialog;
}
QT_END_NAMESPACE

class InputIndexDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit InputIndexDialog(size_t, QWidget *parent = nullptr);

    [[nodiscard]] size_t value() const;

    ~InputIndexDialog() override;
private:
    Ui::InputIndexDialog *ui;
    size_t val;
};
