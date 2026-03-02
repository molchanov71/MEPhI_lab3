#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE namespace Ui
{
    class WriteDataDialog;
}

QT_END_NAMESPACE class WriteDataDialog final : public QDialog
{
    Q_OBJECT

public:
    WriteDataDialog(const std::string &prog_path,
                    const std::string &name,
                    class IProgramRepository &,
                    class IController &,
                    int,
                    const std::vector <uint8_t> &,
                    QWidget *parent = nullptr);

    ~WriteDataDialog() override;

private:
    Ui::WriteDataDialog *ui;
};
