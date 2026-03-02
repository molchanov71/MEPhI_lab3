#include "model/service.hpp"
#include "model/repository.hpp"
#include "model/entity/program.hpp"
#include "model/entity/unit.hpp"
#include <future>
#include <thread>
#include <queue>

size_t MemoryService::searchForward(const size_t size) const
{
    if (size == 0)
        throw std::logic_error("Size can't be 0");
    size_t count = 0;
    auto &data = repo.readData();
    for (size_t i = 0; i < data.size(); ++i)
    {
        if (data[i].isBusy())
            count = 0;
        else
            ++count;
        if (count == size)
            return i - size + 1;
    }
    throw std::logic_error("No free space");
}

void MemoryService::defragment()
{
    auto cmp_units = [](const IMemoryUnit *u1, const IMemoryUnit *u2)
    {
        return u1->getAddress() > u2->getAddress();
    };
    std::priority_queue <IMemoryUnit *, std::vector <IMemoryUnit *>, decltype(cmp_units)> queue(cmp_units);
    for (auto &prog_ptr: repo.getPrograms() | std::views::values)
        for (auto &unit: prog_ptr->getOwnUnits() | std::views::values)
            queue.push(unit.get());
    for (auto &sh_unit: repo.getSharedElems() | std::views::values)
        queue.push(sh_unit.get());
    size_t new_idx = 0;
    while (!queue.empty())
    {
        IMemoryUnit *el = queue.top();
        const size_t old_idx = el->getAddress();
        if (new_idx > old_idx)
            throw std::runtime_error("New index is greater than old one");
        if (new_idx == old_idx)
        {
            new_idx += el->getTotalSize();
            queue.pop();
            continue;
        }
        for (size_t i = 0; i < el->getTotalSize(); ++i)
            std::swap(repo[new_idx + i], repo[old_idx + i]);
        el->setAddress(new_idx);
        new_idx += el->getTotalSize();
        queue.pop();
    }
}

size_t ProgramService::getTotalMemoryUsage(const std::string &prog_path) const
{
    if (!repo.getPrograms().contains(prog_path))
        throw std::runtime_error("Program not found: " + prog_path);

    const Program &program = repo.getProgram(prog_path);

    std::vector <const IMemoryUnit *> all_units;

    const auto &own_units = program.getOwnUnits();
    all_units.reserve(own_units.size() + program.getSharedUnits().size());

    for (const auto &unit: own_units | std::views::values)
        all_units.push_back(unit.get());

    for (const auto &wptr: program.getSharedUnits() | std::views::values)
        if (auto shared_ptr = wptr.lock())
            all_units.push_back(shared_ptr.get());

    if (all_units.empty())
        return 0;

    auto concur = std::thread::hardware_concurrency();
    if (!concur)
        concur = 2;
    const size_t num_threads = std::min <size_t>(
        concur,
        all_units.size()
    );

    const size_t group_size = (all_units.size() + num_threads - 1) / num_threads;
    std::vector <std::future <size_t> > futures;
    futures.reserve(num_threads);

    for (size_t i = 0; i < num_threads; ++i)
    {
        const size_t start_idx = i * group_size;
        const size_t end_idx = std::min((i + 1) * group_size, all_units.size());

        if (start_idx >= end_idx)
            continue;
        auto process_group = [units = std::vector(all_units.begin() + start_idx,
                                                  all_units.begin() + end_idx)]() -> size_t
        {
            size_t tmp_res = 0;
            for (const auto *unit: units)
                tmp_res += unit->getTotalSize();
            return tmp_res;
        };
        futures.push_back(std::async(std::launch::async, process_group));
    }
    size_t total_size = 0;
    for (auto &future: futures)
        total_size += future.get();
    return total_size;
}

void UnitService::createVariable(const std::string &prog_path, std::string unit_name, const size_t size)
{
    try
    {
        Program &prog = repo.getProgram(prog_path);
        const size_t begin = mem_service.searchForward(size);
        try
        {
            for (size_t i = begin; i < begin + size; ++i)
                repo[i].grant();
            prog.createVar(std::move(unit_name), begin, size);
        }
        catch (const std::exception &)
        {
            for (size_t i = begin; i < begin + size; ++i)
                repo[i].detach();
            throw;
        }
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

const AbstractUnit &UnitService::getUnit(const std::string &prog_path, const std::string &unit_name) const
{
    try
    {
        const Program &prog = repo.getProgram(prog_path);
        return prog.getUnit(unit_name);
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

std::vector <uint8_t> UnitService::readUnit(const std::string &prog_path, const std::string &unit_name) const
{
    const AbstractUnit &unit = getUnit(prog_path, unit_name);
    try
    {
        const size_t begin = unit.getAddress(), size = unit.getTotalSize();
        std::vector <uint8_t> res(size);
        for (size_t i = 0; i < size; ++i)
            res[i] = repo[begin + i].read();
        return res;
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void UnitService::writeUnit(const std::string &prog_path,
                            const std::string &unit_name,
                            const std::vector <uint8_t> &new_data)
{
    const AbstractUnit &unit = getUnit(prog_path, unit_name);
    try
    {
        const size_t begin = unit.getAddress(), size = unit.getTotalSize();
        if (new_data.size() != size)
            throw std::logic_error("Size mismatch");
        for (size_t i = 0; i < size; ++i)
            repo[begin + i].write(new_data[i]);
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void IndexedService::createArray(const std::string &prog_path,
                                 std::string unit_name,
                                 const size_t total_size,
                                 const size_t el_size)
{
    try
    {
        Program &prog = repo.getProgram(prog_path);
        const size_t begin = mem_service.searchForward(total_size);
        if (total_size % el_size != 0)
            throw std::logic_error("Total size should be multiple of element size");
        try
        {
            for (size_t i = begin; i < begin + total_size; ++i)
                repo[i].grant();
            prog.createArr(std::move(unit_name), begin, total_size, el_size);
        }
        catch (const std::exception &)
        {
            for (size_t i = begin; i < begin + total_size; ++i)
                repo[i].detach();
            throw;
        }
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void SharedService::createShared(std::string unit_name, const size_t total_size, const size_t el_size)
{
    try
    {
        const size_t begin = mem_service.searchForward(total_size);
        if (total_size % el_size != 0)
            throw std::logic_error("Total size should be multiple of element size");
        auto el = std::make_shared <Shared>(unit_name, begin, total_size, el_size);
        try
        {
            for (size_t i = begin; i < begin + total_size; ++i)
                repo[i].grant(); // переименовать это вот всё
            repo.addShared(std::move(unit_name), std::move(el));
        }
        catch (const std::exception &)
        {
            for (size_t i = begin; i < begin + total_size; ++i)
                repo[i].detach();
            throw;
        }
    }
    catch (const MemoryError &err)
    {
        repo.addError("GLOBAL", err.createUnique());
        throw;
    }
}

void SharedService::destroyShared(const std::string &unit_name)
{
    try
    {
        auto &shared_units = repo.getSharedElems();
        if (!shared_units.contains(unit_name))
            throw IdAccessError(std::format("Shared unit \"{}\" not found", unit_name));
        auto &found = shared_units.at(unit_name);
        const size_t begin = found->getAddress(), size = found->getTotalSize();
        for (size_t i = found->getAddress(); i != begin + size; ++i)
            repo[i].detach();
        for (auto &prog: repo.getPrograms() | std::views::values)
            if (prog->hasShared(unit_name))
                prog->detachAccess(unit_name);
        repo.removeShared(unit_name);
    }
    catch (const MemoryError &err)
    {
        repo.addError("GLOBAL", err.createUnique());
        throw;
    }
}

std::vector <uint8_t> IndexedService::readIndex(const std::string &prog_path,
                                                const std::string &unit_name,
                                                const size_t idx) const
{
    const auto ptr = &unit_service.getUnit(prog_path, unit_name);
    try
    {
        const auto idxd_ptr = dynamic_cast <const AbstractIndexed *>(ptr);
        if (!idxd_ptr)
            throw IdAccessError("This unit is not indexed");
        const size_t begin = ptr->getAddress(), el_size = idxd_ptr->getElSize();
        if (const size_t size = ptr->getTotalSize(); (idx + 1) * el_size > size)
            throw IndexError("Index out of range");
        std::vector <uint8_t> res(el_size);
        for (size_t i = 0; i < el_size; ++i)
            res[i] = repo[begin + idx * el_size + i].read();
        return res;
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void IndexedService::writeIndex(const std::string &prog_path,
                                const std::string &unit_name,
                                const size_t idx,
                                const std::vector <uint8_t> &new_data)
{
    const auto ptr = &unit_service.getUnit(prog_path, unit_name);
    try
    {
        const auto idxd_ptr = dynamic_cast <const AbstractIndexed *>(ptr);
        if (!idxd_ptr)
            throw IdAccessError("This unit is not indexed");
        const size_t begin = ptr->getAddress(), el_size = idxd_ptr->getElSize();
        if (const size_t size = ptr->getTotalSize(); (idx + 1) * el_size > size)
            throw IndexError("Index out of range");
        if (new_data.size() != el_size)
            throw std::logic_error("Size mismatch");
        for (size_t i = 0; i < el_size; ++i)
            repo[begin + idx * el_size + i].write(new_data[i]);
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

std::vector <uint8_t> IndexedService::getArrRange(const std::string &prog_path,
                                                  const std::string &unit_name,
                                                  const size_t begin,
                                                  const size_t end) const
{
    const auto ptr = &unit_service.getUnit(prog_path, unit_name);
    try
    {
        const auto idxd_ptr = dynamic_cast <const AbstractIndexed *>(ptr);
        if (!idxd_ptr)
            throw IdAccessError("This unit is not indexed");
        const size_t unit_begin = ptr->getAddress(), el_size = idxd_ptr->getElSize();
        if (const size_t size = ptr->getTotalSize(); end * el_size > size)
            throw IndexError("Index out of range");
        const unsigned diff = static_cast <long>(end) - begin;
        const unsigned vec_size = diff * el_size;
        std::vector <uint8_t> res;
        res.reserve(vec_size);
        for (size_t i = 0; i < vec_size; ++i)
            res.push_back(repo[unit_begin + begin * el_size + i].read());
        return res;
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

std::vector <uint8_t> IndexedService::getShRange(const std::string &unit_name,
                                                 const size_t begin,
                                                 const size_t end) const
{
    try
    {
        const auto unit = repo.getShared(unit_name);
        const size_t unit_begin = unit->getAddress(), el_size = unit->getElSize();
        if (const size_t size = unit->getTotalSize(); end * el_size > size)
            throw IndexError("Index out of range");
        std::vector <uint8_t> res((end - begin) * el_size);
        for (size_t i = 0; i < (end - begin) * el_size; ++i)
            res[i] = repo[unit_begin + begin * el_size + i].read();
        return res;
    }
    catch (const MemoryError &err)
    {
        repo.addError("GLOBAL", err.createUnique());
        throw;
    }
}

void ProgramService::createProgram(std::string prog_path, const size_t quota)
{
    try
    {
        if (repo.getPrograms().contains(prog_path))
            throw std::logic_error("Path is not unique");
        auto prog = std::make_unique <Program>(prog_path, quota);
        repo.addProgram(std::move(prog_path), std::move(prog));
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void ProgramService::destroyProgram(const std::string &prog_path)
{
    try
    {
        const Program &prog = repo.getProgram(prog_path);
        const auto &own_units = prog.getOwnUnits();
        for (const auto &v: own_units | std::views::values)
        {
            const size_t begin = v->getAddress(), size = v->getTotalSize();
            for (size_t i = begin; i < begin + size; ++i)
                repo[i].detach();
        }
        const bool is_empty = own_units.empty();
        repo.removeProgram(prog_path);
        if (!is_empty)
            throw LeakError(std::format("Memory leak in program \"{}\"", prog_path));
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void SharedService::grantShared(const std::string &prog_path, const std::string &unit_name)
{
    try
    {
        Program &prog = repo.getProgram(prog_path);
        const auto unit = repo.getShared(unit_name);
        prog.getAccess(unit);
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void SharedService::revokeShared(const std::string &prog_path, const std::string &unit_name)
{
    try
    {
        Program &prog = repo.getProgram(prog_path);
        const auto &shared_units = prog.getSharedUnits();
        if (!shared_units.contains(unit_name))
            throw IdAccessError(std::format("Unit \"{}\" not found", unit_name));
        const auto wptr = shared_units.at(unit_name);
        if (wptr.expired())
            return;
        prog.detachAccess(unit_name);
        if (const auto sptr = wptr.lock(); sptr.use_count() == 1)
        {
            const size_t begin = sptr->getAddress(), size = sptr->getTotalSize();
            for (size_t i = begin; i < begin + size; ++i)
                repo[i].detach();
        }
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void UnitService::destroyOwnUnit(const std::string &prog_path, const std::string &unit_name)
{
    try
    {
        Program &prog = repo.getProgram(prog_path);
        const std::unique_ptr <OwnUnit> own_unit = prog.extractOwnUnit(unit_name);
        const size_t begin = own_unit->getAddress(), size = own_unit->getTotalSize();
        for (size_t i = begin; i < begin + size; ++i)
            repo[i].detach();
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void RefService::createRef(const std::string &prog_path, const std::string &unit_name, const std::string &ref_name)
{
    try
    {
        Program &prog = repo.getProgram(prog_path);
        prog.createRef(unit_name, ref_name);
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}

void RefService::removeRef(const std::string &prog_path, const std::string &ref_name)
{
    try
    {
        Program &prog = repo.getProgram(prog_path);
        prog.removeRef(ref_name);
    }
    catch (const MemoryError &err)
    {
        repo.addError(prog_path, err.createUnique());
        throw;
    }
}
