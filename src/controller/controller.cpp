#include "controller.hpp"
#include "model/service.hpp"

Controller::Controller(IProgramRepository &repo_,
                       IMemoryService &mem_service_,
                       IProgramService &prog_service_,
                       ISharedService &sh_service_,
                       IIndexedService &idx_service_,
                       IUnitService &unit_service_,
                       IRefService &ref_service_) : repo(repo_),
                                                    mem_service(mem_service_),
                                                    prog_service(prog_service_),
                                                    sh_service(sh_service_),
                                                    idx_service(idx_service_),
                                                    unit_service(unit_service_),
                                                    ref_service(ref_service_)
{
}

void Controller::createProgram(std::string path, const size_t quota)
{
    prog_service.createProgram(std::move(path), quota);
}

void Controller::destroyProgram(const std::string &path)
{
    prog_service.destroyProgram(path);
}

size_t Controller::getTotalMemoryUsage(const std::string &prog_path) const
{
    return prog_service.getTotalMemoryUsage(prog_path);
}

void Controller::createShared(std::string name, const size_t total_size, const size_t el_size)
{
    sh_service.createShared(std::move(name), total_size, el_size);
}

void Controller::grantShared(const std::string &prog_path, const std::string &unit_name)
{
    sh_service.grantShared(prog_path, unit_name);
}

void Controller::revokeShared(const std::string &prog_path, const std::string &unit_name)
{
    sh_service.revokeShared(prog_path, unit_name);
}

void Controller::createArray(const std::string &prog_path,
                             std::string unit_name,
                             const size_t total_size,
                             const size_t el_size)
{
    idx_service.createArray(prog_path, std::move(unit_name), total_size, el_size);
}

void Controller::createRef(const std::string &prog_path, const std::string &unit_name, const std::string &ref_name)
{
    ref_service.createRef(prog_path, unit_name, ref_name);
}

void Controller::removeRef(const std::string &prog_path, const std::string &ref_name)
{
    ref_service.removeRef(prog_path, ref_name);
}

std::vector <uint8_t> Controller::readIndex(const std::string &prog_path,
                                            const std::string &unit_name,
                                            const size_t idx) const
{
    return idx_service.readIndex(prog_path, unit_name, idx);
}

void Controller::writeIndex(const std::string &prog_path,
                            const std::string &unit_name,
                            const size_t idx,
                            const std::vector <uint8_t> &new_data)
{
    idx_service.writeIndex(prog_path, unit_name, idx, new_data);
}

std::vector <uint8_t> Controller::getArrRange(const std::string &prog_path,
                                              const std::string &unit_name,
                                              const size_t begin,
                                              const size_t end) const
{
    return idx_service.getArrRange(prog_path, unit_name, begin, end);
}

std::vector <uint8_t> Controller::getShRange(const std::string &unit_name, const size_t begin, const size_t end) const
{
    return idx_service.getShRange(unit_name, begin, end);
}

void Controller::createVariable(const std::string &prog_path, std::string unit_name, const size_t size)
{
    unit_service.createVariable(prog_path, std::move(unit_name), size);
}

const AbstractUnit &Controller::getUnit(const std::string &prog_path, const std::string &unit_name) const
{
    return unit_service.getUnit(prog_path, unit_name);
}

std::vector <uint8_t> Controller::readUnit(const std::string &prog_path, const std::string &unit_name) const
{
    return unit_service.readUnit(prog_path, unit_name);
}

void Controller::writeUnit(const std::string &prog_path,
                           const std::string &unit_name,
                           const std::vector <uint8_t> &new_data)
{
    unit_service.writeUnit(prog_path, unit_name, new_data);
}

void Controller::destroyOwnUnit(const std::string &prog_path, const std::string &unit_name)
{
    unit_service.destroyOwnUnit(prog_path, unit_name);
}

void Controller::defragment() const
{
    mem_service.defragment();
}
