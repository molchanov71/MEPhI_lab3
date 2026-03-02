#pragma once

#include <memory>

class QApplication;
class QMainWindow;

class View
{
public:
    virtual void run() = 0;

    virtual ~View() = default;
};

class GraphicView final : public View
{
public:
    GraphicView(class IProgramRepository &, class IController &);

    ~GraphicView() override;

    void run() override;

private:
    IProgramRepository &repo;
    IController &controller;
    std::unique_ptr <QApplication> app;
    std::unique_ptr <QMainWindow> window;
};
