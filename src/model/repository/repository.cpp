#include "model/repository.hpp"
#include "model/entity/program.hpp"
#include "model/entity/unit.hpp"
#include <memory>
#include <stdexcept>

void ProgramRepository::addProgram(std::string path, std::unique_ptr <Program> prog)
{
    if (programs.contains(path))
        throw std::logic_error("Path duplicate");
    if (prog->getQuota() + allocated > data.size())
        throw std::logic_error("Too big quota");
    programs.insert(std::make_pair(std::move(path), std::move(prog)));
}

void ProgramRepository::addShared(std::string name, std::shared_ptr <AbstractShared> el)
{
    if (shared_units.contains(name))
        throw std::logic_error("Shared elems duplicate");
    shared_units.insert(std::make_pair(std::move(name), std::move(el)));
}

void ProgramRepository::addError(const std::string &path, std::unique_ptr <MemoryError> err)
{
    errors[path].push_back(std::move(err));
}

Program &ProgramRepository::getProgram(const std::string &path)
{
    if (!programs.contains(path))
        throw std::logic_error("Path not found");
    return *programs.at(path);
}

const Program &ProgramRepository::getProgram(const std::string &path) const
{
    if (!programs.contains(path))
        throw std::logic_error("Path not found");
    return *programs.at(path);
}

std::shared_ptr <AbstractShared> ProgramRepository::getShared(const std::string &name)
{
    if (!shared_units.contains(name))
        throw std::logic_error("Shared element not found");
    return shared_units.at(name);
}

std::shared_ptr <const AbstractShared> ProgramRepository::getShared(const std::string &name) const
{
    if (!shared_units.contains(name))
        throw std::logic_error("Shared element not found");
    return shared_units.at(name);
}

void ProgramRepository::removeProgram(const std::string &path)
{
    if (!programs.contains(path))
        throw std::logic_error(std::format("Path \"{}\" not found", path));
    programs.erase(path);
}

void ProgramRepository::removeShared(const std::string &name)
{
    if (!shared_units.contains(name))
        throw std::logic_error(std::format("Unit \"{}\" not found", name));
    shared_units.erase(name);
}

void Byte::grant()
{
    if (busy)
        throw std::runtime_error("Already granted");
    busy = true;
}

uint8_t Byte::read() const
{
    if (!busy)
        throw std::logic_error("No access");
    return value;
}

void Byte::write(const uint8_t new_value)
{
    if (!busy)
        throw std::logic_error("No access");
    value = new_value;
}

void Byte::detach()
{
    if (!busy)
        throw DoubleFreeError("Byte already freed");
    busy = false;
}
