#include "model/service.hpp"
#include "model/repository.hpp"
#include <chrono>
#include <iostream>

int main()
{
    ProgramRepository repo(1000000);
    MemoryService mem_service(repo);
    UnitService unit_service(mem_service);
    ProgramService prog_service(mem_service);
    SharedService sh_service(mem_service);
    for (int i = 10000; i < 1000000; i += 10000)
    {
        prog_service.createProgram("prog", i);
        for (int j = 0; j < i / 2; ++j)
        {
            std::string own_name = "o" + std::to_string(j);
            unit_service.createVariable("prog", std::move(own_name), 1);
            std::string sh_name = "s" + std::to_string(j);
            sh_service.createShared(std::move(sh_name), 1, 1);
        }
        auto start = std::chrono::steady_clock::now();
        const size_t usage = prog_service.getTotalMemoryUsage("prog");
        auto end = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast <std::chrono::milliseconds> (end - start).count();
        std::cout << "Program quota: " << i << "\tUsage: " << usage << "\tDuration: " << duration << std::endl;
        for (int j = 0; j < i / 2; ++j)
        {
            std::string sh_name = "s" + std::to_string(j);
            sh_service.destroyShared(sh_name);
            std::string own_name = "o" + std::to_string(j);
            unit_service.destroyOwnUnit("prog", own_name);
        }
        prog_service.destroyProgram("prog");
    }
}
