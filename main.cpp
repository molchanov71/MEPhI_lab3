#include "loader/loader.hpp"
#include "view.hpp"
#include "controller.hpp"
#include "model/repository.hpp"
#include <iostream>
#include <memory>

int main()
{
    try
    {
        const std::unique_ptr <Loader> loader = std::make_unique <StaticLoader> ();
        auto &view = loader->getView();
        view.run();
    }
    catch (const std::exception &err)
    {
        std::cerr << "Error: " << err.what() << std::endl;
        return 1;
    }
    return 0;
}
