#include "view.hpp"
#include "dialogs/main_window.hpp"
#include <QApplication>

GraphicView::GraphicView(IProgramRepository &repo_, IController &controller_) : repo(repo_), controller(controller_) {}

GraphicView::~GraphicView() = default;

void GraphicView::run()
{
    int argc = 0;
    char **argv = nullptr;
    app = std::make_unique <QApplication>(argc, argv);
    window = std::make_unique <MainWindow>(repo, controller);
    window->show();
    app->exec();
}
