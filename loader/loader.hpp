#pragma once

#include <memory>

class View;
class IController;
class IMemoryService;
class IProgramService;
class ISharedService;
class IIndexedService;
class IUnitService;
class IRefService;
class IProgramRepository;

class Loader
{
public:
    virtual View &getView() = 0;

    virtual IController &getController() = 0;

    virtual IMemoryService &getMemoryService() = 0;

    virtual IProgramService &getProgramService() = 0;

    virtual ISharedService &getSharedService() = 0;

    virtual IIndexedService &getIndexedService() = 0;

    virtual IUnitService &getUnitService() = 0;

    virtual IRefService &getRefService() = 0;

    virtual IProgramRepository &getRepo() = 0;

    virtual ~Loader() = default;
};

class StaticLoader final : public Loader
{
    std::unique_ptr <View> view;
    std::unique_ptr <IController> controller;
    std::unique_ptr <IMemoryService> mem_service;
    std::unique_ptr <IProgramService> prog_service;
    std::unique_ptr <ISharedService> sh_service;
    std::unique_ptr <IIndexedService> idx_service;
    std::unique_ptr <IUnitService> unit_service;
    std::unique_ptr <IRefService> ref_service;
    std::unique_ptr <IProgramRepository> repo;
public:
    View &getView() override;

    IController &getController() override;

    IMemoryService &getMemoryService() override;

    IProgramService &getProgramService() override;
    
    ISharedService &getSharedService() override;

    IIndexedService &getIndexedService() override;

    IUnitService &getUnitService() override;

    IRefService &getRefService() override;

    IProgramRepository &getRepo() override;

    ~StaticLoader() override;
};
