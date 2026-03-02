#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE

namespace Ui
{
    class ReadIndexDialog;
}

QT_END_NAMESPACE


class ReadIndexDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ReadIndexDialog(const std::vector <uint8_t> &, QWidget *parent = nullptr);

    ~ReadIndexDialog() override;

private:
    Ui::ReadIndexDialog *ui;
};
