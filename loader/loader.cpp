#include "loader.hpp"
#include "view.hpp"
#include "controller.hpp"
#include "model/service.hpp"
#include "model/repository.hpp"

View &StaticLoader::getView()
{
    if (!view)
        view = std::make_unique <GraphicView>(getRepo(), getController());
    return *view;
}

IController &StaticLoader::getController()
{
    if (!controller)
        controller = std::make_unique <Controller>(getRepo(),
                                                   getMemoryService(),
                                                   getProgramService(),
                                                   getSharedService(),
                                                   getIndexedService(),
                                                   getUnitService(),
                                                   getRefService());
    return *controller;
}

IMemoryService &StaticLoader::getMemoryService()
{
    if (!mem_service)
        mem_service = std::make_unique <MemoryService>(getRepo());
    return *mem_service;
}

IProgramService &StaticLoader::getProgramService()
{
    if (!prog_service)
        prog_service = std::make_unique <ProgramService>(getMemoryService());
    return *prog_service;
}

ISharedService &StaticLoader::getSharedService()
{
    if (!sh_service)
        sh_service = std::make_unique <SharedService>(getMemoryService());
    return *sh_service;
}

IIndexedService &StaticLoader::getIndexedService()
{
    if (!idx_service)
        idx_service = std::make_unique <IndexedService>(getMemoryService(), getUnitService());
    return *idx_service;
}

IUnitService &StaticLoader::getUnitService()
{
    if (!unit_service)
        unit_service = std::make_unique <UnitService>(getMemoryService());
    return *unit_service;
}

IRefService &StaticLoader::getRefService()
{
    if (!ref_service)
        ref_service = std::make_unique <RefService>(getMemoryService());
    return *ref_service;
}

IProgramRepository &StaticLoader::getRepo()
{
    if (!repo)
        repo = std::make_unique <ProgramRepository>(1000);
    return *repo;
}

StaticLoader::~StaticLoader() = default;
