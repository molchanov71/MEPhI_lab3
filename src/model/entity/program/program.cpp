#include "model/entity/program.hpp"
#include "errors.hpp"

void Program::createVar(std::string name,
                        const size_t address,
                        const size_t size)
{
    if (own_units.contains(name) || refs.contains(name) || shared_units.contains(name))
        throw std::logic_error("Duplicate name");
    if (own_space + size > quota)
        throw std::logic_error("Quota limit exceeded");
    auto uptr = std::make_unique <Variable>(name, address, size);
    own_space += size;
    own_units.emplace(std::move(name), std::move(uptr));
}

void Program::createArr(std::string name,
                        const size_t address,
                        const size_t size,
                        const size_t el_size)
{
    if (own_units.contains(name) || refs.contains(name) || shared_units.contains(name))
        throw std::logic_error("Duplicate name");
    if (own_space + size > quota)
        throw std::logic_error("Quota limit exceeded");
    auto uptr = std::make_unique <Array>(name, address, size, el_size);
    own_space += size;
    own_units.emplace(std::move(name), std::move(uptr));
}

void Program::createRef(const std::string &unit_name, std::string ref_name)
{
    if (unit_name == ref_name)
        throw std::logic_error("Names can't be the same");
    if (own_units.contains(ref_name) || shared_units.contains(ref_name) || refs.contains(ref_name))
        throw std::logic_error("Duplicate name");
    OwnUnit *found = nullptr;
    if (own_units.contains(unit_name))
        found = own_units.at(unit_name).get();
    if (!found)
        throw std::logic_error(std::format("Unit \"{}\" not found", ref_name));
    std::string ref_name_cpy(ref_name);
    refs.insert({std::move(ref_name_cpy), Reference(std::move(ref_name), found)});
}

void Program::removeRef(const std::string &ref_name)
{
    if (!refs.contains(ref_name))
        throw IdAccessError("No such reference");
    refs.erase(ref_name);
}

Reference &Program::getRef(const std::string &ref_name) { return refs.at(ref_name); }

const Reference &Program::getRef(const std::string &ref_name) const { return refs.at(ref_name); }

bool Program::hasRef(const std::string &ref_name) const { return refs.contains(ref_name); }

void Program::getAccess(std::shared_ptr <AbstractShared> sptr)
{
    if (shared_units.contains(sptr->getName()))
        return;
    shared_units.emplace(sptr->getName(), sptr);
    sptr->grantAccess(path);
}

void Program::detachAccess(const std::string &name)
{
    if (!shared_units.contains(name))
        throw IdAccessError("Shared element not found");
    if (const auto wptr = shared_units.at(name); !wptr.expired())
    {
        const auto sptr = wptr.lock();
        sptr->revokeAccess(path);
    }
    shared_units.erase(name);
}

std::unique_ptr <OwnUnit> Program::extractOwnUnit(const std::string &name)
{
    if (!own_units.contains(name))
        throw IdAccessError("Unit not found");
    auto own_unit = std::move(own_units.at(name));
    for (auto &ref: refs | std::views::values)
        if (ref.getPtr() == own_unit.get())
            ref.detach();
    own_space -= own_unit->getTotalSize();
    own_units.erase(name);
    return own_unit;
}

AbstractUnit &Program::getUnit(const std::string &name)
{
    AbstractUnit *ptr = nullptr;
    if (own_units.contains(name))
        ptr = own_units.at(name).get();
    if (shared_units.contains(name))
    {
        const auto found_wptr = shared_units.at(name);
        if (found_wptr.expired())
            throw IdAccessError(std::format("Unit \"{}\" was already deleted", name));
        ptr = found_wptr.lock().get();
    }
    if (refs.contains(name))
        ptr = refs.at(name).getPtr();
    if (!ptr)
        throw IdAccessError(std::format("Unit \"{}\" not found", name));
    return *ptr;
}

const AbstractUnit &Program::getUnit(const std::string &name) const
{
    const AbstractUnit *ptr = nullptr;
    if (own_units.contains(name))
        ptr = own_units.at(name).get();
    if (!ptr && shared_units.contains(name))
    {
        const auto found_wptr = shared_units.at(name);
        if (found_wptr.expired())
            throw IdAccessError(std::format("Unit \"{}\" was already deleted", name));
        ptr = found_wptr.lock().get();
    }
    if (!ptr && refs.contains(name))
    {
        ptr = refs.at(name).getPtr();
        if (!ptr)
            throw ReferenceError("Unit was already deleted");
    }
    if (!ptr)
        throw IdAccessError(std::format("Unit \"{}\" not found", name));
    return *ptr;
}

bool Program::hasAnyUnit(const std::string &name) const
{
    return own_units.contains(name) || refs.contains(name) || shared_units.contains(name);
}
