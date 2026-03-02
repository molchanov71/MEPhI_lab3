#pragma once

#include <QDialog>

class IProgramRepository;
class IController;

QT_BEGIN_NAMESPACE namespace Ui
{
    class ProgramDialog;
}

QT_END_NAMESPACE class ProgramDialog final : public QDialog
{
    Q_OBJECT
public:
    ProgramDialog(IProgramRepository &, IController &, const char *, QWidget *parent = nullptr);

    ~ProgramDialog() override;

private:
    Ui::ProgramDialog *ui;
    IProgramRepository &repo;
    IController &controller;
    std::string prog_path;

    void updateOwnUnits();
    void updateSharedUnits();
    void updateRefs();
};
